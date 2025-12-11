#include "starter.grpc.pb.h"
#include "grpcpp/grpcpp.h"
#include <iostream>


class MyHelloService : public Hello::Service {
	virtual grpc::Status sayHello(grpc::ServerContext* context, const Request* request, Response* response) {
		std::cout << "Name: " << request->name() << " Number: " << request->number() << std::endl;
		std::ostringstream message;
		message << "sayHello Name: " << request->name() << " Number: " << request->number();
		response->set_message(message.str());
		return grpc::Status::OK;
	}
};

void runServer() {
	grpc::ServerBuilder builder;
	builder.AddListeningPort("0.0.0.0:50001", grpc::InsecureServerCredentials());

	MyHelloService service;
	builder.RegisterService(&service);

	std::unique_ptr<grpc::Server> server = builder.BuildAndStart();
	server->Wait();
}

int main() {

	runServer();

	return 0;
}