import tvm
from tvm import te
from tvm.ir.module import IRModule
from tvm.script import tir as T
import numpy as np
from tvm.relay import backend, testing
from tvm.testing.aot import generate_ref_data
from pathlib import Path
import textwrap
import rich


test_so_path = Path(__file__).resolve().parent / "test.so"
loaded_mod = tvm.runtime.load_module(test_so_path)
runner = tvm.runtime.executor.AotModule(loaded_mod["default"](tvm.cpu(0)))

data = np.ones((224, 1)).astype("float32")
weight = np.ones((224,)).astype("float32")
# print(data)
# print(weight)
runner.set_input(data=data, weight=weight)
runner.run()
# print(runner.get_output(0).numpy())
