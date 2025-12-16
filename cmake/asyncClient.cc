#include <grpcpp/grpcpp.h>

#include <iostream>
#include <memory>
#include <string>

#include "absl/log/check.h"
#include "absl/log/initialize.h"

#include "starter.grpc.pb.h"

class MyAsyncClient {

public:

    explicit MyAsyncClient(std::shared_ptr<grpc::Channel> channel)
        : stub_(Hello::NewStub(channel)) {
    }

    // Assembles the client's payload, sends it and presents the response back
    // from the server.
    std::string SayHello() {
        // Data we are sending to the server.
        Request request;
        request.set_name("grpc_client");
        request.set_number(100);

        // Container for the data we expect from the server.
        Response response;

        // Context for the client. It could be used to convey extra information to
        // the server and/or tweak certain RPC behaviors.
        grpc::ClientContext context;

        // The producer-consumer queue we use to communicate asynchronously with the
        // gRPC runtime.
        grpc::CompletionQueue cq;

        // Storage for the status of the RPC upon completion.
        grpc::Status status;

        std::unique_ptr<grpc::ClientAsyncResponseReader<Response> > rpc(
            stub_->AsyncsayHello(&context, request, &cq));

        // Request that, upon completion of the RPC, "reply" be updated with the
        // server's response; "status" with the indication of whether the operation
        // was successful. Tag the request with the integer 1.
        rpc->Finish(&response, &status, (void*)1);
        void* got_tag;
        bool ok = false;
        // Block until the next result is available in the completion queue "cq".
        // The return value of Next should always be checked. This return value
        // tells us whether there is any kind of event or the cq_ is shutting down.
        CHECK(cq.Next(&got_tag, &ok));

        // Verify that the result from "cq" corresponds, by its tag, our previous
        // request.
        CHECK_EQ(got_tag, (void*)1);
        // ... and that the request was completed successfully. Note that "ok"
        // corresponds solely to the request for updates introduced by Finish().
        CHECK(ok);

        // Act upon the status of the actual RPC.
        if (status.ok()) {
            return response.message();
        }
        else {
            return "RPC failed";
        }
    }

private:
    // Out of the passed in Channel comes the stub, stored here, our view of the
    // server's exposed services.
    std::unique_ptr<Hello::Stub> stub_;
};

int main() {

    MyAsyncClient client(
        grpc::CreateChannel("127.0.0.1:50001", grpc::InsecureChannelCredentials()));
    std::string response = client.SayHello();  // The actual RPC call!
    std::cout << "Response received: " << response << std::endl;

	return 0;
}
