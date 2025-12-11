# grpc-starter-example-cpp

# C++ examples for starters to learn how to use gRPC

This repository contains simple C++ examples demonstrating how to use gRPC for building client-server applications. Each example is designed to help you understand different aspects of gRPC, from basic setup to advanced features.

## Prerequisites

- Windows Platform
- CMake 3.30
- Visual Studio 2022
- gRPC v1.72 
- [gRPC C++ - Building from source](https://github.com/grpc/grpc/blob/master/BUILDING.md#build-from-source)

## Steps to use this example

0. After building gRPC from source, you need to set the environment variable `GRPC_INSTALL_DIR` to the path where gRPC is installed which is also the path when you build with CMake flag `CMAKE_INSTALL_PREFIX`.

1. Clone the repository:
   ```sh
   git clone https://github.com/ggntju/grpc-starter-example-cpp
   cd grpc-starter-example-cpp
   ```
2. Look at the [protos](./protos/) folder, [starter.proto](./protos/starter.proto) is the example Protocol Buffers definition file.

3. Run the batch script [protoc_command.bat](./protos/protoc_command.bat) to generate the C++ code from the proto file.

4. Open the server and client solutions in [Hello](./Hello/) folder in Visual Studio 2022. 
