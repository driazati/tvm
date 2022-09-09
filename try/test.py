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
# tvmscript -> IRModule (tir) -> runtime::Module
# tvmscript bypasses relay by writing TIR straight in Python

# @tvm.script.ir_module
# class MyModule:
#     @T.prim_func
#     def main(a: T.handle, b: T.handle):
#         T.func_attr({
#             "global_symbol": "main",
#             "tir.noalias": True,
#         })
#         A = T.match_buffer(a, (5,))
#         B = T.match_buffer(b, (5,))
#         for i in range(5):
#             with T.block("B"):
#                 vi = T.axis.spatial(5, i)
#                 B[vi] = A[vi] + 1.0

# irm = MyModule
# print(irm.script())
# main_func = irm["main"]
# print("main:", main_func)
# print("ok")
# print(main_func.params)
# p = main_func.params
# rich.inspect(p)
# rich.inspect(p[0])
# exit(0)

# relay_model = textwrap.dedent(
#     """\
#     #[version = "0.0.5"]
#     def @main(%data : Tensor[(1, 3, 64, 64), uint8], %weight : Tensor[(3, 3, 5, 5), int8]) {
#         %1 = nn.conv2d(
#                 %data,
#                 %weight,
#                 padding=[2, 2],
#                 channels=3,
#                 kernel_size=[5, 5],
#                 data_layout="NCHW",
#                 kernel_layout="OIHW",
#                 out_dtype="int32");
#         %2 = cast(nn.max_pool2d(%1, pool_size=[3, 3]), dtype="int8");
#         %3 = nn.conv2d(
#                 %2,
#                 %weight,
#                 padding=[2, 2],
#                 channels=3,
#                 kernel_size=[5, 5],
#                 data_layout="NCHW",
#                 kernel_layout="OIHW",
#                 out_dtype="int32");
#         %4 = nn.max_pool2d(%3, pool_size=[3, 3]);
#         %4
#     }
# """
# )
# irm = tvm.parser.fromtext(relay_model)

# A = te.placeholder((5,), dtype="float32", name="A")
# B = te.compute((5,), lambda *i: A(*i) + 1.0, name="B")
# func = te.create_prim_func([A, B])
# te_irm = IRModule({"main": func})
# print(te_irm.script())

# # create the runtime module
# mod = tvm.build(irm, target="llvm")
# print(mod)

# a = tvm.nd.array(np.arange(5).astype("float32"))
# b = tvm.nd.array(np.zeros((5,)).astype("float32"))
# mod(a, b)
# print(a)
# print(b)

# schedule = tvm.tir.Schedule(irm)
# print(schedule)
# block_b = schedule.get_block("B")
# print(block_b)
# loops = schedule.get_loops(block_b)[0]
# print(loops)
# schedule.split(loops, factors=[2, 2, 2])
# print(schedule.mod.script())


enable_usmp = False
target_kind = "llvm"

# main_func = irm["main"]
# shape_dict = {p.name_hint: p.checked_type.concrete_shape for p in main_func.params}
# type_dict = {p.name_hint: p.checked_type.dtype for p in main_func.params}

# weight_data = np.random.randint(1, 255, shape_dict["weight"]).astype(type_dict["weight"])
# input_data = np.ones(shape_dict["data"]).astype(type_dict["data"])
# params = {"weight": weight_data}
# inputs = {"data": input_data}
# ref_outputs = generate_ref_data(irm, inputs, params)

# with tvm.transform.PassContext(
#     opt_level=3,
#     config={
#         "tir.disable_vectorize": True,
#         "tir.usmp.enable": enable_usmp,
#     },
# ):
#     mod = tvm.relay.build(
#         irm,
#         params=params,
#         target=target_kind,
#         executor=backend.Executor("aot", {"interface-api": "packed", "unpacked-api": False}),
#     )
# temp_dir = tvm.contrib.utils.TempDirectory()
# test_so_path = temp_dir / "test.so"
# mod.export_library(test_so_path, cc="gcc", options=["-std=c11", "-g3", "-O0"])
# loaded_mod = tvm.runtime.load_module(test_so_path)
# runner = tvm.runtime.executor.AotModule(loaded_mod["default"](tvm.cpu(0)))
# runner.set_input(**inputs)
# runner.run()
# assert (runner.get_output(0).numpy() == list(ref_outputs.values())[0]).all()
# import pprint
# odir = dir
# def ddir(x):
#     return
#     print(type(x))
#     pprint.pprint([a for a in odir(x) if not a.startswith("__")])
# from tvm.target import Target
# """Tests compilation of convolutions"""
# relay_model = textwrap.dedent(
#     """\
#     #[version = "0.0.5"]
#     def @main(%data : Tensor[(224, 1), float32], %bias : Tensor[(224), float32]) {
#         %1 = nn.bias_add(
#                 %data,
#                 %bias,
#                 axis=0);
#         %1
#     }
# """
# )
# ir_mod = tvm.parser.fromtext(relay_model)
# # print(tvm.relay.build.__code__)
# # exit(0)
# mod = tvm.relay.build(ir_mod, target="llvm", executor=backend.Executor("aot", {
#     "interface-api": "packed", "unpacked-api": False,
# }))
# print("got mod")

@tvm.script.ir_module
class MyModule:
    @T.prim_func
    def main(a: T.handle, b: T.handle):
        T.func_attr({'global_symbol': 'main', 'tir.noalias': True})
        A = T.match_buffer(a, (8,), dtype="float32")
        B = T.match_buffer(b, (8,), dtype="float32")
        for i in range(8):
            with T.block("B"):
                vi = T.axis.spatial(8, i)
                B[vi] = A[vi] + 1.5

ir_mod = MyModule
a = tvm.build(ir_mod, target="llvm")
print(a)

# ddir(mod)
# print(mod.lowered_ir_mods)
# llvm_mod = mod.lowered_ir_mods[mod.target]
# xs = [x for x in mod.lowered_ir_mods.values()]
# lmod = xs[0]
# ddir(lmod)
# bias_add, main = [x for x in lmod.functions.keys()]
# print(bias_add)
# ddir(bias_add)
# print(main)
# ddir(main)
# func = lmod.functions.
# print(func)
# print(lmod.source_map)
# llvm_mod = mod.lowered_ir_mods["llvm -keys=cpu -link-params=0"]
# ddir(mod.get_lib())
# # ddir(mod.ir_mod)
# print(mod.get_lib().impoted_modules)
# print(mod.get_lib().get_source())
# A = te.placeholder((224,1), "a")
# B = te.placeholder((224,), "b")
# tmod = tvm.build(ir_mod, [A, B], "llvm", "mymod")

test_so_path = Path(__file__).resolve().parent / "test.so"
print("Saving to", test_so_path)

# function that does the work is called:
    # tvmgen_default_fused_nn_bias_add_compute_
# Exporting without the AOT executor makes __tvm_main__ undefined
# Using the AOT executor makes it defined and global
# the 'cc' arg here is just used to link various .o files:
#   devc.o
#   lib0.o
#   lib1.o
a.export_library(test_so_path, cc="gcc", options=["-std=c11", "-g3", "-O0"])
exit(0)
m = mod
i = ir_mod
# breakpoint()
exit(0)
loaded_mod = tvm.runtime.load_module(test_so_path)
runner = tvm.runtime.executor.AotModule(loaded_mod["default"](tvm.cpu(0)))
# runner.set_input(**inputs)
# runner.run()
# assert (runner.get_output(0).numpy() == list(ref_outputs.values())[0]).all()

for k, v in ir_mod.functions.items():
    rich.inspect(k)
    rich.inspect(v)
    print("\n\n")
    rich.inspect(v.checked_type)
# rich.inspect(ir_mod)


loaded_mod = tvm.runtime.load_module(test_so_path)
runner = tvm.runtime.executor.AotModule(loaded_mod["default"](tvm.cpu(0)))

data = np.ones((224, 1)).astype("float32")
weight = np.ones((224,)).astype("float32")
# data += 2
print(data)
print(weight)
runner.set_input(data=data, weight=weight)
runner.run()
print(runner.get_output(0).numpy())

exit(0)

main_func = ir_mod["main"]
shape_dict = {p.name_hint: p.checked_type.concrete_shape for p in main_func.params}
type_dict = {p.name_hint: p.checked_type.dtype for p in main_func.params}

weight_data = np.random.randint(1, 255, shape_dict["weight"]).astype(type_dict["weight"])
input_data = np.ones(shape_dict["data"]).astype(type_dict["data"])
params = {"weight": weight_data}
inputs = {"data": input_data}
ref_outputs = generate_ref_data(ir_mod, inputs, params)

with tvm.transform.PassContext(
    opt_level=3,
    config={
        "tir.disable_vectorize": True,
        "tir.usmp.enable": enable_usmp,
    },
):
    mod = tvm.relay.build(
        ir_mod,
        params=params,
        target=target_kind,
        executor=backend.Executor("aot", {"interface-api": "packed", "unpacked-api": False}),
    )
# temp_dir = tvm.contrib.utils.TempDirectory()
# test_so_path = temp_dir / "test.so"
test_so_path = Path(__file__).resolve().parent / "test.so"
print(test_so_path)


mod.export_library(test_so_path, cc="gcc", options=["-std=c11", "-g3", "-O0"])
loaded_mod = tvm.runtime.load_module(test_so_path)
runner = tvm.runtime.executor.AotModule(loaded_mod["default"](tvm.cpu(0)))
runner.set_input(**inputs)
runner.run()
assert (runner.get_output(0).numpy() == list(ref_outputs.values())[0]).all()
