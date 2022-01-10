import torch
import tvm
from tvm import relay
from tvm.contrib import graph_executor


def fn(x):
    return x.dot(x)


def dump_mod(mod):
    s = str(mod).split("\n")
    print("\n".join(s[113:]))


print("Tracing fn")
model = torch.jit.trace(fn, (torch.ones(2),))
info = [(i.debugName(), tuple(i.type().sizes())) for i in model.graph.inputs()]

# Generate module
print("Importing to relay")
mod, params = relay.frontend.from_pytorch(model, input_infos=info)
dump_mod(mod)

# Execute
print("Lowering to llvm")
target = tvm.target.Target("llvm", host="llvm")
dev = tvm.cpu(0)
with tvm.transform.PassContext(opt_level=0):
    lib = relay.build(mod, target=target, params=params)
m = graph_executor.GraphModule(lib["default"](dev))
i = torch.ones(2)
m.set_input("x", tvm.nd.array(i))
m.run()

# Get outputs
tvm_output = m.get_output(0)
print(tvm_output)