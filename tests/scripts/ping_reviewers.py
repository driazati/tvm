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
import argparse
import re
import datetime
import json
from typing import Dict, Any, List

from git_utils import git, GitHubRepo, parse_remote


def prs_query(user: str, repo: str, cursor: str = None):
    after = ""
    if cursor is not None:
        after = f', before:"{cursor}"'
    return f"""
        {{
    repository(name: "{repo}", owner: "{user}") {{
        pullRequests(states: [OPEN], last: 10{after}) {{
        edges {{
            cursor
        }}
        nodes {{
            number
            url
            body
            isDraft
            author {{
                login
            }}
            reviews(last:100) {{
                nodes {{
                    author {{ login }}
                    comments(last:100) {{
                        nodes {{
                            updatedAt
                            bodyText
                        }}
                    }}
                }}
            }}
            publishedAt
            comments(last:100) {{
                nodes {{
                    authorAssociation
                    bodyText
                    updatedAt
                    author {{
                        login
                    }}
                }}
            }}
        }}
        }}
    }}
    }}
    """


def find_reviewers(body: str) -> List[str]:
    matches = re.findall(r"(cc( @[-A-Za-z0-9]+)+)", body, flags=re.MULTILINE)
    matches = [full for full, last in matches]

    reviewers = []
    for match in matches:
        if match.startswith("cc "):
            match = match.replace("cc ", "")
        users = [x.strip() for x in match.split("@")]
        reviewers += users

    reviewers = set(x for x in reviewers if x != "")
    return list(reviewers)


def check_pr(pr, wait_time):
    published_at = datetime.datetime.strptime(pr["publishedAt"], "%Y-%m-%dT%H:%M:%SZ")
    last_action = published_at

    # GitHub counts comments left as part of a review separately than standalone
    # comments
    reviews = pr["reviews"]["nodes"]
    review_comments = []
    for review in reviews:
        review_comments += review["comments"]["nodes"]

    # Collate all comments
    comments = pr["comments"]["nodes"] + review_comments

    # Find the last date of any comment
    for comment in comments:
        commented_at = datetime.datetime.strptime(comment["updatedAt"], "%Y-%m-%dT%H:%M:%SZ")
        if commented_at > last_action:
            last_action = commented_at

    time_since_last_action = datetime.datetime.utcnow() - last_action

    # Find reviewers in the PR's body
    pr_body_reviewers = find_reviewers(pr["body"])

    # Pull out reviewers from any cc @... text in a comment
    cc_reviewers = [find_reviewers(c["bodyText"]) for c in comments]
    cc_reviewers = [r for revs in cc_reviewers for r in revs]

    # Anyone that has left a review as a reviewer (this may include the PR
    # author since their responses count as reviews)
    review_reviewers = list(set(r["author"]["login"] for r in reviews))

    reviewers = cc_reviewers + review_reviewers + pr_body_reviewers

    if time_since_last_action > wait_time:
        print(
            "    Pinging reviewers",
            reviewers,
            "on",
            pr["url"],
            "since it has been",
            time_since_last_action,
            "since anything happened on that PR",
        )
        return reviewers

    return None


def ping_reviewers(pr, reviewers):
    reviewers = [f"@{r}" for r in reviewers]
    text = (
        "It has been a while since this PR was updated, "
        + " ".join(reviewers)
        + " please leave a review or address the outstanding comments"
    )
    r = github.post(f"issues/{pr['number']}/comments", {"body": text})
    print(r)


if __name__ == "__main__":
    help = "Comment on languishing issues and PRs"
    parser = argparse.ArgumentParser(description=help)
    parser.add_argument("--remote", default="origin", help="ssh remote to parse")
    parser.add_argument("--wait-time-minutes", required=True, type=int, help="ssh remote to parse")
    parser.add_argument("--cutoff-pr-number", default=0, type=int, help="ssh remote to parse")
    parser.add_argument("--dry-run", action="store_true", help="don't update GitHub")
    parser.add_argument("--allowlist", help="filter by these PR authors")
    parser.add_argument("--pr-json", help="(testing) data for testing to use instead of GitHub")
    args = parser.parse_args()

    remote = git(["config", "--get", f"remote.{args.remote}.url"])
    user, repo = parse_remote(remote)

    wait_time = datetime.timedelta(minutes=int(args.wait_time_minutes))
    cutoff_pr_number = int(args.cutoff_pr_number)
    print(
        "Running with:\n"
        f"  time cutoff: {wait_time}\n"
        f"  number cutoff: {cutoff_pr_number}\n"
        f"  dry run: {args.dry_run}\n"
        f"  user/repo: {user}/{repo}\n",
        end="",
    )

    # [slow rollout]
    # This code is here to gate this feature to a limited set of people before
    # deploying it for everyone to avoid spamming in the case of bugs or
    # ongoing development.
    if args.allowlist:
        author_allowlist = args.allowlist.split(",")
    else:
        github = GitHubRepo(token=os.environ["GITHUB_TOKEN"], user=user, repo=repo)
        allowlist_issue = github.get("issues/9983")
        author_allowlist = set(find_reviewers(allowlist_issue["body"]))

    if args.pr_json:
        r = json.loads(args.pr_json)
    else:
        q = prs_query(user, repo)
        r = github.graphql(q)

    # Loop until all PRs have been checked
    while True:
        prs = r["data"]["repository"]["pullRequests"]["nodes"]

        # Don't look at draft PRs at all
        prs = [pr for pr in prs if not pr["isDraft"]]

        # Don't look at super old PRs
        prs = [pr for pr in prs if pr["number"] > cutoff_pr_number]

        # [slow rollout]
        prs = [pr for pr in prs if pr["author"]["login"] in author_allowlist]

        print(f"Checking {len(prs)} PRs: {[pr['number'] for pr in prs]}")

        # Ping reviewers on each PR in the response if necessary
        for pr in prs:
            print("Checking", pr["url"])
            reviewers = check_pr(pr, wait_time)
            if reviewers is not None and not args.dry_run:
                ping_reviewers(pr, reviewers)

        edges = r["data"]["repository"]["pullRequests"]["edges"]
        if len(edges) == 0:
            # No more results to check
            break

        cursor = edges[0]["cursor"]
        r = github.graphql(prs_query(user, repo, cursor))
