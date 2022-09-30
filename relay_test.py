import re
import textwrap

import numpy as np
import pytest

import tvm
from tvm import IRModule
from tvm import relay
from tvm.relay import backend, testing
from tvm.testing.aot import generate_ref_data

relay_model = textwrap.dedent(
    """\
    #[version = "0.0.5"]
    def @main(%data : Tensor[(1, 3, 64, 64), uint8], %weight : Tensor[(3, 3, 5, 5), int8]) {
        %1 = nn.conv2d(
                %data,
                %weight,
                padding=[2, 2],
                channels=3,
                kernel_size=[5, 5],
                data_layout="NCHW",
                kernel_layout="OIHW",
                out_dtype="int32");
        %2 = cast(nn.max_pool2d(%1, pool_size=[3, 3]), dtype="int8");
        %3 = nn.conv2d(
                %2,
                %weight,
                padding=[2, 2],
                channels=3,
                kernel_size=[5, 5],
                data_layout="NCHW",
                kernel_layout="OIHW",
                out_dtype="int32");
        %4 = nn.max_pool2d(%3, pool_size=[3, 3]);
        %4
    }
"""
)
ir_mod = tvm.parser.fromtext(relay_model)

main_func = ir_mod["main"]
shape_dict = {p.name_hint: p.checked_type.concrete_shape for p in main_func.params}
type_dict = {p.name_hint: p.checked_type.dtype for p in main_func.params}

weight_data = np.random.randint(1, 255, shape_dict["weight"]).astype(type_dict["weight"])
input_data = np.ones(shape_dict["data"]).astype(type_dict["data"])
params = {"weight": weight_data}
inputs = {"data": input_data}
ref_outputs = generate_ref_data(ir_mod, inputs, params)
test_so_path = "relay_test.so"

BUILD = True

if BUILD:
    with tvm.transform.PassContext(
        opt_level=3,
        config={
            "tir.disable_vectorize": True,
            "tir.usmp.enable": False,
        },
    ):
        mod = tvm.relay.build(
            ir_mod,
            params=params,
            target="llvm",
            executor=backend.Executor("aot", {"interface-api": "packed", "unpacked-api": False}),
        )
    mod.export_library(test_so_path, cc="gcc", options=["-std=c11", "-g3", "-O0"])

loaded_mod = tvm.runtime.load_module(test_so_path)
runner = tvm.runtime.executor.AotModule(loaded_mod["default"](tvm.cpu(0)))
runner.set_input(**inputs)
runner.run()
assert (runner.get_output(0).numpy() == list(ref_outputs.values())[0]).all()
