#include <grpcpp/grpcpp.h>

#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

#include "starter.grpc.pb.h"


ABSL_FLAG(std::string, target, "localhost:8687", "Server address");

class StarterClient {
public:
    StarterClient(std::shared_ptr<grpc::Channel> channel)
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

        // The actual RPC.
        std::mutex mu;
        std::condition_variable cv;
        bool done = false;
        grpc::Status status;
        stub_->async()->sayHello(&context, &request, &response,
            [&mu, &cv, &done, &status](grpc::Status s) {
                status = std::move(s);
                std::lock_guard<std::mutex> lock(mu);
                done = true;
                cv.notify_one();
            });

        std::unique_lock<std::mutex> lock(mu);
        while (!done) {
            cv.wait(lock);
        }

        // Act upon its status.
        if (status.ok()) {
            return response.message();
        }
        else {
            std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
            return "RPC failed";
        }
    }

private:
    std::unique_ptr<Hello::Stub> stub_;
};

int main(int argc, char** argv) {
    absl::ParseCommandLine(argc, argv);
    // Instantiate the client. It requires a channel, out of which the actual RPCs
    // are created. This channel models a connection to an endpoint specified by
    // the argument "--target=" which is the only expected argument.
    std::string target_str = absl::GetFlag(FLAGS_target);
    // We indicate that the channel isn't authenticated (use of
    // InsecureChannelCredentials()).
    StarterClient greeter(
        grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));
    std::string response = greeter.SayHello();
    std::cout << "Response received: " << response << std::endl;

    return 0;
}