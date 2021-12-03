#!/usr/bin/env bash

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

# Usage: github_label_pr.sh <pr number> <label>
set -euxo pipefail

PR_NUMBER=$1
LABEL=$2
ORIGIN=$(git config --get remote.origin.url)

if [[ $ORIGIN == https* ]]; then
    # Grab the user/repo combo from an https remote
    GITHUB_USER=$(basename "$(dirname "$ORIGIN")")
    GITHUB_REPO=$(basename "$ORIGIN" | sed 's/\.git//g')
else
    # Grab the user/repo combo from an SSH remote
    GITHUB_USER=$(echo "$ORIGIN" | sed 's/.*:\(.*\)\/\(.*\)\.git.*/\1/g')
    GITHUB_REPO=$(echo "$ORIGIN" | sed 's/.*:\(.*\)\/\(.*\)\.git.*/\2/g')
fi

LABELS_URL="https://api.github.com/repos/$GITHUB_USER/$GITHUB_REPO/issues/$PR_NUMBER/labels"

echo "Labeling https://github.com/$GITHUB_USER/$GITHUB_REPO/pull/$PR_NUMBER with $LABEL"
curl -H "Authorization: Bearer $TOKEN" -d "{\"labels\": [\"$LABEL\"]}" "$LABELS_URL" | tee result.json

if grep --quiet "\"message\"" result.json; then
    echo "Error message found in response"
    exit 1
else
    echo "Successfully labelled PR"
    rm result.json
fi