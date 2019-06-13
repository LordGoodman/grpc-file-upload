This is the gRPC module of the HLF POC, currently it is independent of Concord, but it will be integrated on the top of KV Blockchain eventually.

# Client 
  Send invoke/query proposal to peer and handle the response

# Server
  Handle "GetState" request from peer

# Build
## Install proto and gRPC
### On ubuntu 18.04
sudo apt-get install build-essential autoconf libtool pkg-config automake curl
  
git clone -b $(curl -L https://grpc.io/release) https://github.com/grpc/grpc
cd grpc
git submodule update --init
 
### Build and install protobuf
cd ./third_party/protobuf
./autogen.sh
./configure --prefix=/opt/protobuf
make -j `nproc`
sudo make install
  
### Build and install gRPC
 - cd ../..
 - make -j `nproc` PROTOC=/opt/protobuf/bin/protoc 
 - sudo make prefix=/opt/grpc install

## Cmake
 - cd client/server
 - mkdir build && cd build
 - cmake ..
 - make
