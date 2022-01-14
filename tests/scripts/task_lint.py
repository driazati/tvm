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

import asyncio
import subprocess
import sys
import multiprocessing
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent.parent


async def ngather(tasks, n):
    print(f"Gathering {len(tasks)} tasks across {n} cores")
    semaphore = asyncio.Semaphore(n)

    async def sem_task(task):
        async with semaphore:
            return await task

    return await asyncio.gather(*(sem_task(task) for task in tasks))


async def cmd(description, command):
    print(description)
    print("+", command)
    sys.stdout.flush()
    proc = await asyncio.create_subprocess_shell(
        command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, encoding="utf-8"
    )
    stdout, stderr = await proc.communicate()
    print(stdout.decode())
    print(stderr.decode())
    return proc


async def main():
    coros = []

    coros.append(
        cmd(description="Checking file types...", command="python3 tests/lint/check_file_type.py")
    )

    coros.append(
        cmd(
            description="Checking ASF license headers...",
            command="tests/lint/check_asf_header.sh --local",
        )
    )

    # coros.append(
    #     cmd(
    #         description="Linting C++ code in vta/include",
    #         command="python3 3rdparty/dmlc-core/scripts/lint.py vta cpp vta/include",
    #     )
    # )
    # coros.append(
    #     cmd(
    #         description="Linting C++ code in vta/src",
    #         command="python3 3rdparty/dmlc-core/scripts/lint.py vta cpp vta/src",
    #     )
    # )
    # coros.append(
    #     cmd(
    #         description="Linting C++ code in include",
    #         command="python3 3rdparty/dmlc-core/scripts/lint.py tvm cpp include",
    #     )
    # )

    # src = REPO_ROOT / "src"
    # dirs = [f for f in src.glob("*") if f.is_dir()]
    # for dir in dirs:
    #     coros.append(
    #         cmd(
    #             description="Linting C++ code in src",
    #             command=f"python3 3rdparty/dmlc-core/scripts/lint.py tvm cpp {dir.relative_to(REPO_ROOT)}",
    #         )
    #     )

    # coros.append(
    #     cmd(
    #         description="Linting C++ code in examples/extension/src",
    #         command="python3 3rdparty/dmlc-core/scripts/lint.py tvm cpp examples/extension/src",
    #     )
    # )
    # coros.append(
    #     cmd(
    #         description="Linting C++ code in examples/graph_executor/src",
    #         command="python3 3rdparty/dmlc-core/scripts/lint.py tvm cpp examples/graph_executor/src",
    #     )
    # )
    # coros.append(
    #     cmd(
    #         description="Linting C++ code in tests/cpp",
    #         command="python3 3rdparty/dmlc-core/scripts/lint.py tvm cpp tests/cpp",
    #     )
    # )
    # coros.append(
    #     cmd(
    #         description="Linting C++ code in tests/crt",
    #         command="python3 3rdparty/dmlc-core/scripts/lint.py tvm cpp tests/crt",
    #     )
    # )

    # coros.append(cmd(description="clang-format check...", command="tests/lint/clang_format.sh"))

    coros.append(cmd(description="Rust check...", command="tests/lint/rust_format.sh"))

    coros.append(cmd(description="black check...", command="tests/lint/python_format.sh"))

    # coros.append(
    #     cmd(description="Linting the Python code with pylint...", command="tests/lint/pylint.sh")
    # )

    coros.append(
        cmd(description="Linting the Python code with flake8...", command="tests/lint/flake8.sh")
    )

    coros.append(cmd(description="Linting the JNI code...", command="tests/lint/jnilint.sh"))

    coros.append(cmd(description="Checking C++ documentation...", command="tests/lint/cppdocs.sh"))

    coros.append(
        cmd(description="Type checking with MyPy ...", command="tests/scripts/task_mypy.sh")
    )

    await ngather(coros, multiprocessing.cpu_count())


if __name__ == "__main__":
    loop = asyncio.get_event_loop()
    loop.run_until_complete(main())
