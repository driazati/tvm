import torch
import tvm
from tvm import relay
from tvm.contrib import graph_executor


def fn(x):
    return x.dot(x)


def dump_mod(mod):
    s = str(mod).split("\n")
    print("\n".join(s[113:]))


input = torch.randn(2)
model = torch.jit.trace(fn, (input,))
info = [(i.debugName(), tuple(i.type().sizes())) for i in model.graph.inputs()]
mod, params = relay.frontend.from_pytorch(model, input_infos=info)


dump_mod(mod)

target = tvm.target.Target("llvm", host="llvm")
dev = tvm.cpu(0)
with tvm.transform.PassContext(opt_level=3):
    lib = relay.build(mod, target=target, params=params)

dtype = "float32"
m = graph_executor.GraphModule(lib["default"](dev))
# Set inputs
i = torch.ones(2)
# m.set_input("x", tvm.nd.array(i))
# # Execute
# m.run()
# # Get outputs
# tvm_output = m.get_output(0)
y = tvm.nd.array(i)
# print(type(y))
# exit(0)
tvm_output = m(x=i)
print(tvm_output)
print(i.dot(i))
