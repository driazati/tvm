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
from pathlib import Path

summary = "\n".join(sys.argv[2:])


REPO_ROOT = Path(__file__).resolve().parent.parent

SOURCE = REPO_ROOT / "python" / "tvm" / "_version.py"
OUTPUT = REPO_ROOT / "python" / "tvm" / "version.py"


with open(SOURCE) as f:
    content = f.read()

header = []
content_lines = []


preamble = f"""
# This file is generated from {Path(__file__).relative_to(REPO_ROOT)} as part of the CMake
# build
""".strip()

license_line = "# under the License."
header, content = content.split(license_line)
header = header + license_line


new_content = header + "\n\n" + preamble + content.format(CMAKE_SUMMARY=summary)

with open(OUTPUT, "w") as f:
    f.write(new_content)
