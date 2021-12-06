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
import subprocess
import re
from urllib import request
from typing import Dict, Tuple, Any


class GitHubRepo:
    def __init__(self, user, repo, token):
        self.token = token
        self.user = user
        self.repo = repo
        self.base = f"https://api.github.com/repos/{user}/{repo}/"

    def headers(self):
        return {
            "Authorization": f"Bearer {self.token}",
        }

    def post(self, url: str, data: Dict[str, Any]) -> Dict[str, Any]:
        url = self.base + url
        print("Posting", url)
        data = json.dumps(data).encode()
        req = request.Request(url, headers=self.headers(), data=data)
        with request.urlopen(req) as response:
            response = json.loads(response.read())
        return response

    def get(self, url: str) -> Dict[str, Any]:
        url = self.base + url
        print("Requesting", url)
        req = request.Request(url, headers=self.headers())
        with request.urlopen(req) as response:
            response = json.loads(response.read())
        return response

    def mark_ci_skipped(self, pr_number: str) -> None:
        pr = self.get(f"pulls/{pr_number}")
        current_title = pr["title"]
        if current_title.startswith("[skip ci]"):
            print("PR title already starts with '[skip ci]', not doing anything")
        else:
            print("Adding '[skip ci]' to PR title")
            title = f"[skip ci] {current_title}"
            self.post(f"pulls/{pr_number}", data={"title": title})

        self.post(f"issues/{pr_number}/labels", data={"labels": ["ci-skipped"]})


def parse_remote(remote: str) -> Tuple[str, str]:
    """
    Get a GitHub (user, repo) pair out of a git remote
    """
    if remote.startswith("https://"):
        # Parse HTTP remote
        parts = remote.split("/")
        if len(parts) < 2:
            raise RuntimeError(f"Unable to parse remote '{remote}'")
        return parts[-2], parts[-1].replace(".git", "")
    else:
        # Parse SSH remote
        m = re.search(r":(.*)/(.*)\.git", remote)
        if m is None or len(m.groups()) != 2:
            raise RuntimeError(f"Unable to parse remote '{remote}'")
        return m.groups()


if __name__ == "__main__":
    help = "Marks a PR as skipped by adding '[skip ci]' to the PR title and adding a label"
    parser = argparse.ArgumentParser(description=help)
    parser.add_argument("--pr", required=True)
    parser.add_argument("--remote", default="origin", help="ssh remote to parse")
    args = parser.parse_args()

    proc = subprocess.run(
        ["git", "config", "--get", f"remote.{args.remote}.url"], stdout=subprocess.PIPE, check=True
    )
    remote = proc.stdout.decode().strip()
    user, repo = parse_remote(remote)

    github = GitHubRepo(token=os.environ["TOKEN"], user=user, repo=repo)
    github.mark_ci_skipped(args.pr)
