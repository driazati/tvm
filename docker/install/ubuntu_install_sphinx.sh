#!/bin/bash
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

set -e
set -u
set -o pipefail

# NOTE: install docutils < 0.17 to work around https://github.com/readthedocs/sphinx_rtd_theme/issues/1115
pip3 install \
    autodocsumm \
    "commonmark>=0.7.3" \
    "docutils>=0.11,<0.17" \
    Image \
    matplotlib \
    sphinx==4.2.0 \
    sphinx_autodoc_annotation \
    sphinx_rtd_theme

cleanup() {
  rm -rf /sphinx-gallery /sphinx_gallery_colab_fixes.patch
}

trap cleanup 0

# We use a custom build of sphinx-gallery to enable open-in-Colab links
# This integrates https://github.com/sphinx-gallery/sphinx-gallery/pull/941 and
# https://github.com/sphinx-gallery/sphinx-gallery/pull/940 (if those are merged
# into sphinx-gallery we can go back to using the official version)
git clone https://github.com/sphinx-gallery/sphinx-gallery.git
pushd sphinx-gallery
git checkout b3bf338ceb2993b83a137905f5a50351be28a1f9
cat /sphinx_gallery_colab_fixes.patch | patch -p1 -d .
pip3 install .
popd
