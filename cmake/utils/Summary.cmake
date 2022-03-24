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

function(pad_string output str padchar length)
    string(LENGTH "${str}" _strlen)
    math(EXPR _strlen "${length} - ${_strlen}")

    if(_strlen GREATER 0)
        unset(_pad)
        foreach(_i RANGE 1 ${_strlen}) # inclusive
            string(APPEND _pad ${padchar})
        endforeach()
        string(APPEND str ${_pad})
    endif()

    set(${output} "${str}" PARENT_SCOPE)
endfunction()

macro(print_summary)
    set(SUMMARY_OUTPUT " ")
    message(STATUS "  ---------------- Summary ----------------")

    macro(message_save TEXT)
        message(STATUS "${TEXT}")
        list(APPEND SUMMARY_OUTPUT "${TEXT}")
    endmacro()

    # Show some basic system information
    message_save("  CMake version         : ${CMAKE_VERSION}")
    message_save("  CMake executable      : ${CMAKE_COMMAND}")
    message_save("  Generator             : ${CMAKE_GENERATOR}")
    message_save("  System                : ${CMAKE_SYSTEM_NAME}")
    message_save("  C++ compiler          : ${CMAKE_CXX_COMPILER}")
    message_save("  C++ compiler ID       : ${CMAKE_CXX_COMPILER_ID}")
    message_save("  C++ compiler version  : ${CMAKE_CXX_COMPILER_VERSION}")
    message_save("  CXX flags             : ${CMAKE_CXX_FLAGS}")
    message_save("  Linker flags          : ${CMAKE_SHARED_LINKER_FLAGS}")
    message_save("  Build type            : ${CMAKE_BUILD_TYPE}")
    get_directory_property(READABLE_COMPILE_DEFS DIRECTORY ${PROJECT_SOURCE_DIR} COMPILE_DEFINITIONS)
    message_save("  Compile definitions   : ${READABLE_COMPILE_DEFS}")

    list(SORT TVM_ALL_OPTIONS)
    message_save("  Options:")

    # Compute padding necessary for options
    set(MAX_LENGTH 0)
    foreach(OPTION ${TVM_ALL_OPTIONS})
        string(LENGTH ${OPTION} OPTIONLENGTH)
        if(${OPTIONLENGTH} GREATER ${MAX_LENGTH})
            set(MAX_LENGTH ${OPTIONLENGTH})
        endif()
    endforeach()
    math(EXPR PADDING_LENGTH "${MAX_LENGTH} + 3")

    # Print each of the options (padded out so they're all aligned)
    foreach(OPTION ${TVM_ALL_OPTIONS})
        set(OPTION_VALUE "${${OPTION}}")
        pad_string(OUT "   ${OPTION}" " " ${PADDING_LENGTH})
        message_save("${OUT} : ${OPTION_VALUE}")
    endforeach()

    execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/cmake/generate_version.py ${SUMMARY_OUTPUT} RESULT_VARIABLE ret)

    if(ret EQUAL "1")
        message(FATAL_ERROR "Failed to generate tvm/version.py from cmake/generate_version.py")
    endif()
endmacro()
