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

# This file is generated from cmake/generate_version.py as part of the CMake
# build
import textwrap

cmake_summary = """
  CMake version         : 3.18.4
  CMake executable      : /usr/bin/cmake
  Generator             : Ninja
  System                : Linux
  C++ compiler          : /home/driazati/tools/bin/c++
  C++ compiler ID       : GNU
  C++ compiler version  : 10.3.0
  CXX flags             : -std=c++14 -faligned-new -O2 -Wall -fPIC 
  Linker flags          :   -B "/usr/local/libexec/mold"
  Build type            : 
  Compile definitions   : TVM_INDEX_DEFAULT_I64=1
USE_PROFILER=1
TVM_THREADPOOL_USE_OPENMP=0
DMLC_USE_FOPEN64=0
NDEBUG=1
_GNU_SOURCE
_DEBUG
__STDC_CONSTANT_MACROS
__STDC_FORMAT_MACROS
__STDC_LIMIT_MACROS
TVM_LLVM_VERSION=140
  Options:
   BUILD_STATIC_RUNTIME               : OFF
   COMPILER_RT_PATH                   : 3rdparty/compiler-rt
   DLPACK_PATH                        : 3rdparty/dlpack/include
   DMLC_PATH                          : 3rdparty/dmlc-core/include
   HIDE_PRIVATE_SYMBOLS               : OFF
   INDEX_DEFAULT_I64                  : ON
   INSTALL_DEV                        : OFF
   PICOJSON_PATH                      : 3rdparty/picojson
   RANG_PATH                          : 3rdparty/rang/include
   ROCM_PATH                          : /opt/rocm
   SUMMARIZE                          : 1
   USE_ALTERNATIVE_LINKER             : AUTO
   USE_AOT_EXECUTOR                   : ON
   USE_ARM_COMPUTE_LIB                : OFF
   USE_ARM_COMPUTE_LIB_GRAPH_EXECUTOR : OFF
   USE_BLAS                           : none
   USE_BNNS                           : OFF
   USE_BYODT_POSIT                    : OFF
   USE_CMSISNN                        : OFF
   USE_COREML                         : OFF
   USE_CPP_RPC                        : OFF
   USE_CUBLAS                         : OFF
   USE_CUDA                           : 1
   USE_CUDNN                          : OFF
   USE_CUSTOM_LOGGING                 : OFF
   USE_CUTLASS                        : OFF
   USE_DNNL_CODEGEN                   : OFF
   USE_ETHOSN                         : OFF
   USE_FALLBACK_STL_MAP               : OFF
   USE_GRAPH_EXECUTOR                 : ON
   USE_GRAPH_EXECUTOR_CUDA_GRAPH      : OFF
   USE_GTEST                          : AUTO
   USE_HEXAGON_DEVICE                 : OFF
   USE_HEXAGON_RPC                    : OFF
   USE_HEXAGON_SDK                    : /path/to/sdk
   USE_IOS_RPC                        : OFF
   USE_KHRONOS_SPIRV                  : OFF
   USE_LIBBACKTRACE                   : ON
   USE_LIBTORCH                       : OFF
   USE_LLVM                           : 1
   USE_METAL                          : OFF
   USE_MICRO                          : OFF
   USE_MICRO_STANDALONE_RUNTIME       : OFF
   USE_MIOPEN                         : OFF
   USE_MKL                            : OFF
   USE_MKLDNN                         : OFF
   USE_MSVC_MT                        : OFF
   USE_NNPACK                         : OFF
   USE_OPENCL                         : OFF
   USE_OPENMP                         : OFF
   USE_PAPI                           : OFF
   USE_PROFILER                       : ON
   USE_PT_TVMDSOOP                    : OFF
   USE_RANDOM                         : ON
   USE_RELAY_DEBUG                    : OFF
   USE_ROCBLAS                        : OFF
   USE_ROCM                           : OFF
   USE_RPC                            : ON
   USE_RTTI                           : ON
   USE_RUST_EXT                       : OFF
   USE_SORT                           : ON
   USE_SPIRV_KHR_INTEGER_DOT_PRODUCT  : OFF
   USE_STACKVM_RUNTIME                : OFF
   USE_TARGET_ONNX                    : OFF
   USE_TENSORFLOW_PATH                : none
   USE_TENSORRT_CODEGEN               : OFF
   USE_TENSORRT_RUNTIME               : OFF
   USE_TFLITE                         : OFF
   USE_TF_TVMDSOOP                    : OFF
   USE_THREADS                        : ON
   USE_THRUST                         : OFF
   USE_VITIS_AI                       : OFF
   USE_VULKAN                         : OFF
""".strip()


def describe():
    print(f"CMake Summary:\n{textwrap.indent(cmake_summary, prefix='  ')}")
