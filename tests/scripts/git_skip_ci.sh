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

set -eux

# This implements the skip commands found here:
# https://github.blog/changelog/2021-02-08-github-actions-skip-pull-request-and-push-workflows-with-skip-ci/

# The script will return 1 if any of these are found in the HEAD commit message
# headline

if git log --format='%s' HEAD~1..HEAD | grep --quiet \
    --regexp='\[skip ci\]' \
    --regexp='\[ci skip\]' \
    --regexp='\[no ci\]' \
    --regexp='\[skip actions\]' \
    --regexp='\[actions skip\]'; then
    # last commit message subject matched one of the tags, exit with 1 to
    # tell Jenkins to skip subsequent jobs
    exit 1
else
    # no match in message, don't skip anything
    exit 0
fi
