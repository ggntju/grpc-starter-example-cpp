#include "starter.grpc.pb.h"
#include "grpcpp/grpcpp.h"
#include <iostream>

void runClient() {

	std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel("127.0.0.1:50001", grpc::InsecureChannelCredentials());

	std::unique_ptr< Hello::Stub> stub = Hello::NewStub(channel);

	grpc::ClientContext context;

	Request request;
	request.set_name("grpc_client");
	request.set_number(100);

	Response response;
	grpc::Status status = stub->sayHello(&context, request, &response);

	if (status.ok()) {
		std::cout << response.message() << std::endl;
	}
	else {
		std::cerr << status.error_code() << " " << status.error_message() << std::endl;
	}

}

int main(){
	runClient();
	return 0;
}