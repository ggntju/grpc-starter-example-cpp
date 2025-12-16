#include <grpcpp/grpcpp.h>

#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "absl/log/check.h"
#include "absl/strings/str_format.h"

#include "starter.grpc.pb.h"

class MyAsyncService {

public:

    // Take in the "service" instance (in this case representing an asynchronous
    // server) and the completion queue "cq" used for asynchronous communication
    // with the gRPC runtime.
    MyAsyncService(Hello::AsyncService* service, grpc::ServerCompletionQueue* cq)
        : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE) {
        // Invoke the serving logic right away.
        Proceed();
    }

    void Proceed() {
        if (status_ == CREATE) {
            // Make this instance progress to the PROCESS state.
            status_ = PROCESS;

            // As part of the initial CREATE state, we *request* that the system
            // start processing SayHello requests. In this request, "this" acts are
            // the tag uniquely identifying the request (so that different MyAsyncService
            // instances can serve different requests concurrently), in this case
            // the memory address of this MyAsyncService instance.
            service_->RequestsayHello(&ctx_, &request_, &responder_, cq_, cq_, this);     
        }
        else if (status_ == PROCESS) {
            // Spawn a new MyAsyncService instance to serve new clients while we process
            // the one for this MyAsyncService. The instance will deallocate itself as
            // part of its FINISH state.
            new MyAsyncService(service_, cq_);

            // The actual processing.

            std::cout << "Name: " << request_.name() << " Number: " << request_.number() << std::endl;

            std::ostringstream message;
            message << "sayHello Name: " << request_.name() << " Number: " << request_.number();
            response_.set_message(message.str());

            // And we are done! Let the gRPC runtime know we've finished, using the
            // memory address of this instance as the uniquely identifying tag for
            // the event.
            status_ = FINISH;
            responder_.Finish(response_, grpc::Status::OK, this);
        }
        else {
            CHECK_EQ(status_, FINISH);
            // Once in the FINISH state, deallocate ourselves (AsyncService).
            delete this;
        }
    }

private:
    // The means of communication with the gRPC runtime for an asynchronous
    // server.
    Hello::AsyncService* service_;
    // The producer-consumer queue where for asynchronous server notifications.
    grpc::ServerCompletionQueue* cq_;
    // Context for the rpc, allowing to tweak aspects of it such as the use
    // of compression, authentication, as well as to send metadata back to the
    // client.
    grpc::ServerContext ctx_;

    // What we get from the client.
    Request request_;
    // What we send back to the client.
    Response response_;

    // The means to get back to the client.
    grpc::ServerAsyncResponseWriter<Response> responder_;

    // Let's implement a tiny state machine with the following states.
    enum CallStatus { CREATE, PROCESS, FINISH };
    CallStatus status_;  // The current serving state.
};

class MyAsyncServer {

public:

    ~MyAsyncServer() {
        server_->Shutdown();
        // Always shutdown the completion queue after the server.
        cq_->Shutdown();
    }

    // There is no shutdown handling in this code.
    void Run(uint16_t port) {
        std::string server_address = absl::StrFormat("0.0.0.0:%d", port);

        grpc::ServerBuilder builder;
        // Listen on the given address without any authentication mechanism.
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        // Register "service_" as the instance through which we'll communicate with
        // clients. In this case it corresponds to an *asynchronous* service.
        builder.RegisterService(&service_);
        // Get hold of the completion queue used for the asynchronous communication
        // with the gRPC runtime.
        cq_ = builder.AddCompletionQueue();
        // Finally assemble the server.
        server_ = builder.BuildAndStart();
        std::cout << "Server listening on " << server_address << std::endl;

        // Proceed to the server's main loop.
        HandleRpcs();
    }

private:

    // This can be run in multiple threads if needed.
    void HandleRpcs() {
        // Spawn a new MyAsyncService instance to serve new clients.
        new MyAsyncService(&service_, cq_.get());
        void* tag;  // uniquely identifies a request.
        bool ok;
        while (true) {
            // Block waiting to read the next event from the completion queue. The
            // event is uniquely identified by its tag, which in this case is the
            // memory address of a MyAsyncService instance.
            // The return value of Next should always be checked. This return value
            // tells us whether there is any kind of event or cq_ is shutting down.
            CHECK(cq_->Next(&tag, &ok));
            CHECK(ok);
            static_cast<MyAsyncService*>(tag)->Proceed();
        }
    }

    std::unique_ptr<grpc::ServerCompletionQueue> cq_;
    Hello::AsyncService service_;
    std::unique_ptr<grpc::Server> server_;
};

int main() {

    MyAsyncServer server;
    server.Run(50001);

	return 0;
}