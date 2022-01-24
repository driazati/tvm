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

import sys
import argparse
import fnmatch
import typing
from enum import Enum
from typing import Dict, Any, List


from git_utils import git


class Tags(Enum):
    lint = 0
    docs = 1
    gpu = 2
    cpu = 3
    build = 4
    unittest = 5
    integration = 6


# This an ordered list of rules that produce tags for each file. If a file
# matches a rule, then that tag is added to the output.
FILE_RULES = [
    ("Jenkinsfile", [Tags.lint]),
    ("gallery/**", [Tags.lint, Tags.docs]),
    ("docker/**", [Tags.lint]),
    ("**", [tag for tag in Tags]),
]


if __name__ == "__main__":
    help = "Output a list of tags to run based on the last N commits"
    parser = argparse.ArgumentParser(description=help)
    parser.add_argument("--lookback", default="1", help="number of commits to inspect")
    args = parser.parse_args()

    lookback = int(args.lookback)
    edited_files = git(["diff", f"HEAD~{lookback}", "HEAD~1", "--name-only"]).strip().split("\n")
    edited_files = [f.strip() for f in edited_files]
    edited_files = [f for f in edited_files if f != ""]

    file_tags = {}
    print(f"Found files: {edited_files}", file=sys.stderr)
    for filename in edited_files:
        file_tags[filename] = []
        for filter, tags in FILE_RULES:
            if fnmatch.fnmatch(filename, filter):
                to_add = [t.name for t in tags]
                print(f"Adding tags '{to_add}' since '{filename}' matched rule '{filter}'", file=sys.stderr)
                file_tags[filename] += to_add
                break
    
    tags = list(set(tag for tags in file_tags.values() for tag in tags))

    if len(edited_files) == 0:
        # No files, maybe something went wrong? Run everything just to be safe
        print(",".join([tag.name for tag in Tags]))
    else:
        if len(tags) > 0:
            # Output the list of tags for Jenkins to consume
            print(",".join(tags))
 