#include <iostream>
#include <fstream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include "fileUpload.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using grpc::ClientWriter;

using fileupload::Chunk;
using fileupload::UploadService;
using fileupload::UploadStatus;

using fileupload::UploadStatus_StatusType_Ok;
using fileupload::UploadStatus_StatusType_Failed;
using fileupload::UploadStatus_StatusType_Unknown;

using std::cout;
using std::endl;
using std::string;
using std::ifstream;
using std::ios;

class UploadServiceClient {
 public:
  UploadServiceClient(std::shared_ptr<Channel> channel)
      : stub_(UploadService::NewStub(channel)) {}

  UploadStatus Upload(string file_path) {
    UploadStatus response;

    ifstream f(file_path, ios::in|ios::binary);
    if (!f.is_open()) {
      response.set_status(UploadStatus_StatusType_Failed);
      response.set_msg("Unabld to open file: " + file_path);
      return response;
    }

    ClientContext context;
    std::unique_ptr<ClientWriter<Chunk>> writer(
          stub_->Upload(&context, &response));

    // Every chunk sent to server is limited to 1 MB
    f.seekg(0, f.end);
    int length = f.tellg();
    f.seekg(0, f.beg);

    cout << "Read file size: "
         << length/1024/1024
         << " MB" << endl;

    // 1 MB 
    const int standard = 1024 * 1024;

    int rounds = length / standard;
    int remain = length % standard;

    cout << "After " << rounds << " rounds"
         << " still has " << remain / 1024 << "KB remaining"
         << endl;
    
    // handle rounds
    for(int i = 0; i < rounds; i++){
      Chunk *chunk = new Chunk;
      char *buffer = new char[standard];

      f.read(buffer, standard);

      chunk->set_data(buffer, standard);

      writer->Write(*chunk);

      delete [] buffer;
      delete chunk;
    }

    // handle remain
    Chunk *chunk = new Chunk;
    char *buffer = new char[remain];

    f.read(buffer, remain);
    chunk->set_data(buffer, remain);
    writer->Write(*chunk);
    delete [] buffer;
    delete chunk;

    // finish writing
    f.close();

    writer->WritesDone();

    Status status = writer->Finish(); 

    if (!status.ok()) {
      response.set_status(UploadStatus_StatusType_Failed);
    }

    return response;
  }

 private:
  std::unique_ptr<UploadService::Stub> stub_;
};


int main(int argc, char** argv) {

  UploadServiceClient client(grpc::CreateChannel(
      "localhost:50051", grpc::InsecureChannelCredentials()));

  string file_path = "node-v10.15.3-linux-x64.tar.xz";
  if (argc > 1) {
    file_path = argv[1];
  }

  UploadStatus reply = client.Upload(file_path);
  std::cout << "client received status: " << reply.status() << std::endl;
  std::cout << "client received message: " << reply.msg() << std::endl;
  return 0;
}
