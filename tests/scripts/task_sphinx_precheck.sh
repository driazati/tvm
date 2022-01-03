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

# Precheck if sphinx docs build can fail.
set -e
set -u
set -x
set -o pipefail

cleanup()
{
    rm -rf /tmp/$$.*
}
trap cleanup 0

export PATH=/opt/miniconda3/bin:$PATH

# TODO: Move these to the docker image
pip install autodocsumm==0.2.7
pip install docutils==0.17.1
pip install Image==1.5.33
pip install matplotlib==3.5.0
pip install Pillow==8.4.0
pip install recommonmark==0.7.1
pip install sphinx_rtd_theme==1.0.0
pip install sphinx-gallery==0.10.1
pip install sphinx==4.3.0
pip install tlcpack-sphinx-addon==0.2.1
pip install pytest==6.2.5
pip install attrs
pip install cloudpickle
pip install decorator
pip install numpy
pip install psutil
pip install scipy
pip install synr
pip install tornado

# cleanup cache
rm -rf docs/_staging
rm -rf docs/_build
find . -type f -path "*.pyc" | xargs rm -f
make cython3

echo "PreCheck sphinx doc generation WARNINGS.."
cd docs
make clean
TVM_TUTORIAL_EXEC_PATTERN=none make html 2>&1 | tee /tmp/$$.log.txt

grep -v -E "__mro__|UserWarning|FutureWarning|tensorflow|Keras|pytorch|TensorFlow|403|git describe|scikit-learn version" < /tmp/$$.log.txt > /tmp/$$.logclean.txt || true
echo "---------Sphinx Log----------"
cat /tmp/$$.logclean.txt
echo "-----------------------------"
if grep --quiet -E "WARN" < /tmp/$$.logclean.txt; then
    echo "WARNING found in the log, please fix them."
    grep -E "WARN" < /tmp/$$.logclean.txt
    echo "You can reproduce locally by running ./tests/scripts/task_sphinx_precheck.sh"
    exit 1
fi
echo "No WARNINGS to be fixed."
