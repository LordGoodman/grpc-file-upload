#include <iostream>
#include <fstream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include "fileUpload.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReader;
using grpc::Status;
using fileupload::UploadService;
using fileupload::Chunk;
using fileupload::UploadStatus;

using fileupload::UploadStatus_StatusType_Ok;
using fileupload::UploadStatus_StatusType_Failed;
using fileupload::UploadStatus_StatusType_Unknown;

using std::cout;
using std::endl;
using std::ofstream;
using std::ios;

// Logic and data behind the server's behavior.
class UploadServiceImpl final : public UploadService::Service {
  Status Upload(ServerContext* context, ServerReader<Chunk>* reader,
                UploadStatus* reply) override {

    Chunk buffer;
    ofstream output("test.tar.gz", ios::binary|ios::app);

    while(reader->Read(&buffer)) {
      if (output.is_open()) {
        const char *bytes = buffer.data().c_str();
        cout << "receiving " << buffer.data().length() << "Bytes of data" << endl;
        output.write(bytes, buffer.data().length());
        buffer.clear_data();
      } else {
        reply->set_msg("Output file closed unexpectedly");
        reply->set_status(UploadStatus_StatusType_Failed);
        return Status::CANCELLED;
      }
    }

    output.close();

    reply->set_msg("You are doing great");
    reply->set_status(UploadStatus_StatusType_Ok);

    return Status::OK;
  }
};

void RunServer() {
  std::string server_address("0.0.0.0:50051");
  UploadServiceImpl upload_service;

  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&upload_service);
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;
  
  server->Wait();
}

int main(int argc, char** argv) {
  RunServer();

  return 0;
}
