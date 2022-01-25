#!/usr/bin/env python3
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

import os
import json
import argparse
import tempfile
import requests
from pathlib import Path
from urllib import request
from typing import Any, Dict

from git_utils import git, GitHubRepo, parse_remote

REPO_ROOT = Path(__file__).resolve().parent.parent.parent


_commit_query_fields = """
    messageHeadline
    oid
    statusCheckRollup {
        contexts(last:100) {
            nodes {
                ... on CheckRun {
                    conclusion
                    status
                    name
                    detailsUrl
                    checkSuite {
                        workflowRun {
                            workflow {
                                name
                            }
                        }
                    }
                }
                ... on StatusContext {
                    context
                    state
                    targetUrl
                }
            }
        }
    }
"""


def commits_query(user: str, repo: str, cursor: str = None):
    """
    Create a GraphQL query to find the last N commits along with their statuses
    and some metadata (paginated after 'cursor')
    """
    after = ""
    if cursor is not None:
        after = f', after:"{cursor}"'

    return f"""
    {{
    repository(name: "{repo}", owner: "{user}") {{
        defaultBranchRef {{
        target {{
            ... on Commit {{
            history(first: 15{after}) {{
                edges {{ cursor }}
                nodes {{
                    {_commit_query_fields}
                }}
            }}
            }}
        }}
        }}
    }}
    }}
    """


def check_commit(commit: Dict[str, Any]) -> bool:
    """
    Check the commit and message discord if necessary
    """
    statuses = commit["statusCheckRollup"]["contexts"]["nodes"]

    # GitHub Actions statuses are different from external GitHub statuses, so
    # unify them into 1 representation
    unified_statuses = []
    for status in statuses:
        if "context" in status:
            # Parse non-GHA status
            # unified_statuses.append((status["context"], status["state"] == "SUCCESS"))
            unified_statuses.append(
                {
                    "name": status["context"],
                    "status": status["state"],
                    "url": status["targetUrl"],
                }
            )
        else:
            # Parse GitHub Actions item
            workflow = status["checkSuite"]["workflowRun"]["workflow"]["name"]
            name = f"{workflow} / {status['name']}"
            # unified_statuses.append((name, status["conclusion"] == "SUCCESS"))
            unified_statuses.append(
                {
                    "name": name,
                    "status": status["conclusion"],
                    "url": status["detailsUrl"],
                }
            )

    # print(f"Statuses on {commit['oid']}:", json.dumps(unified_statuses, indent=2))

    # # Assert that specific jobs are present in the commit statuses (i.e. don't
    # # approve if CI was broken and didn't schedule a job)
    # expected_jobs = {"tvm-ci/branch", "CI / MacOS", "CI / Windows"}
    # job_names = {name for name, status in unified_statuses}
    # for job in expected_jobs:
    #     if job not in job_names:
    #         # Did not find expected job name
    #         return False

    # passed_ci = all(status for name, status in unified_statuses)
    return {"oid": commit["oid"], "statuses": unified_statuses, "messageHeadline": commit["messageHeadline"]}


def message_diff(old, new):
    print("wow")

    def find_old(oid):
        for c in old:
            if c == old["oid"]:
                return c
        return None

    for commit in new:
        old_commit = find_old(commit["oid"])
        if old_commit is not None:
            # find which jobs to message
            old_names = {x["name"] for x in old_commit["statuses"]}
            to_message = [x for x in commit["statuses"] if x["name"] not in old_names]
        else:
            # message about all jobs
            to_message = commit["statuses"]

        to_message = [
            x for x in to_message if x["status"] != "SUCCESS" and x["status"] != "PENDING"
        ]

        for m in to_message:
            msg = f"Job `{m['name']}` failed on commit `{commit['oid']}`: {commit['messageHeadline']}"
            discord(
                {
                    "content": msg,
                    "embeds": [
                        # {
                        #     "title": commit['oid'],
                        #     "url": m["url"],
                        # },
                        {
                            "title": m["url"],
                            "url": m["url"],
                        }
                    ]
                }
            )
    # exit(0)


def discord(body: Dict[str, Any]) -> Dict[str, Any]:
    url = os.environ["DISCORD_WEBHOOK"]
    r = requests.post(url, json=body)
    print(r.content)


if __name__ == "__main__":
    help = "Ping discord on CI failures"
    parser = argparse.ArgumentParser(description=help)
    parser.add_argument("--user", default="apache", help="ssh remote to parse")
    parser.add_argument("--repo", default="tvm", help="ssh remote to parse")
    args = parser.parse_args()

    user = args.user
    repo = args.repo

    github = GitHubRepo(token=os.environ["GITHUB_TOKEN"], user=user, repo=repo)
    q = commits_query(user, repo)
    r = github.graphql(q)

    commits = r["data"]["repository"]["defaultBranchRef"]["target"]["history"]["nodes"]

    # Limit GraphQL pagination
    MAX_COMMITS_TO_CHECK = 5
    i = 0

    all_data = []

    with open(REPO_ROOT / "statuses.json") as f:
        old_all_data = json.load(f)

    while i < MAX_COMMITS_TO_CHECK:
        # Check each commit
        for commit in commits:
            all_data.append(check_commit(commit))

        break
        # No good commit found, proceed to next page of results
        edges = r["data"]["repository"]["defaultBranchRef"]["target"]["history"]["edges"]
        if len(edges) == 0:
            break
        else:
            q = commits_query(user, repo, cursor=edges[-1]["cursor"])
            r = github.graphql(q)
            commits = r["data"]["repository"]["defaultBranchRef"]["target"]["history"]["nodes"]

        # Backstop to prevent looking through all the past commits
        i += len(commits)

    print(old_all_data)
    print(all_data)
    print(old_all_data == all_data)
    if old_all_data != all_data:
        message_diff(old_all_data, all_data)

    if old_all_data != all_data:
        with open(REPO_ROOT / "statuses.json", "w") as f:
            json.dump(all_data, f)

        git(["add", "statuses.json"])
        git(["config", "user.email", "email@example.com"])
        git(["config", "user.name", "Your Name"])
        git(["commit", "-m'update status'"])
        git(["push"])

