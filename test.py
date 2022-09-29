"""
1. install tir-level spans from reprprinter onto the ops
    - override repr printer to track its current position
    - when printing, store the position info back into the printed ops

2. output line-level spans during codegen
    - check with llvm-dwarfdump

tvm.build - where does AOT executor codegen actually get used?
"""

import tvm
import tvm.relay
from tvm.script import tir as T

# From https://tvm.apache.org/docs/tutorial/tensor_ir_blitz_course.html
@tvm.script.ir_module
class MyModule:
    @T.prim_func
    def main(a: T.handle, b: T.handle):
        # We exchange data between function by handles, which are similar to pointer.
        T.func_attr({"global_symbol": "main", "tir.noalias": True})
        # Create buffer from handles.
        A = T.match_buffer(a, (8,), dtype="float32")
        B = T.match_buffer(b, (8,), dtype="float32")
        for i in range(8):
            # A block is an abstraction for computation.
            with T.block("B"):
                # Define a spatial block iterator and bind it to value i.
                vi = T.axis.spatial(8, i)
                assert 1 == 0, "Some numbers"
                B[vi] = A[vi] + 1.0


runtime_module = tvm.build(MyModule, target="llvm")

# How do I see the Spans on each op in the runtime_module?
print(runtime_module)
with open("output.llvmir", "w") as f:
    f.write(runtime_module.get_source())
# print(runtime_module.script)
# print(dir(runtime_module))


"""
0. A way to run a TIR module through the compiler and see the resulting spans
    - this file, along with something similar to the optional info handling
      in relay_text_printer.cc
1. A mutating pass that runs towards the end of everything (in driver_api.cc)
      1.1. A mutating pass that replaces all spans with debug info
M1: Verify that spans are being mutated at all (done)
2. Calls the printer, records accessed ops and current line. Every new line,
  output to a file and install span information on nodes
M2: Verify that the TIR span information matches the file (done)
3. Codegen reads spans and calls relevant LLVM APIs.
M3: Verify through llvm-dwarfdump that line information is in the resulting .so + the debug intrinsics in runtime_module.get_source()
4. Cleanups - add a flag or something
M4: Example usage in GDB
"""

# runtime_module.export_library("test.so")

