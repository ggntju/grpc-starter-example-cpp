# Copyright 2018 gRPC authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# cmake build file for C++ helloworld example.
# Assumes protobuf and gRPC have been installed using cmake.
# See cmake_externalproject/CMakeLists.txt for all-in-one cmake build
# that automatically builds all the dependencies before building helloworld.

cmake_minimum_required(VERSION 3.16)

project(HelloWorld C CXX)

include(./common.cmake)

# Proto file
get_filename_component(starter_proto "../protos/starter.proto" ABSOLUTE)
get_filename_component(starter_proto_path "${starter_proto}" PATH)

# Generated sources
set(starter_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/starter.pb.cc")
set(starter_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/starter.pb.h")
set(starter_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/starter.grpc.pb.cc")
set(starter_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/starter.grpc.pb.h")
add_custom_command(
      OUTPUT "${starter_proto_srcs}" "${starter_proto_hdrs}" "${starter_grpc_srcs}" "${starter_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${starter_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${starter_proto}"
      DEPENDS "${starter_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# hw_grpc_proto
add_library(starter_grpc_proto
  ${starter_grpc_srcs}
  ${starter_grpc_hdrs}
  ${starter_proto_srcs}
  ${starter_proto_hdrs})
target_link_libraries(starter_grpc_proto
  absl::check
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})

# Targets greeter_[async_](client|server)
foreach(_target
  syncClient syncServer asyncClient asyncServer asyncCallbackClient asyncCallbackServer)
  add_executable(${_target} "${_target}.cc")
  target_link_libraries(${_target}
    starter_grpc_proto
    absl::check
    absl::flags
    absl::flags_parse
    absl::log
    absl::log_initialize
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})
endforeach()

