#ifndef GRPC_QML_SERVER_READER_H
#define GRPC_QML_SERVER_READER_H

#include "grpc/qml/server_calldata.h"
#include "protobuf/qml/descriptors.h"
#include "protobuf/qml/server_method.h"

#include <grpc++/support/async_stream.h>
#include <grpc++/server_context.h>
#include <grpc++/impl/proto_utils.h>
#include <google/protobuf/message.h>

namespace grpc {
namespace qml {

class GrpcService;
class ServerReaderCallData;
class ServerReaderMethod : public ::protobuf::qml::ServerReaderMethod,
                           private CallDataStore<ServerReaderCallData> {
  Q_OBJECT

public:
  ServerReaderMethod(GrpcService* service,
                     int index,
                     ::grpc::ServerCompletionQueue* cq,
                     ::protobuf::qml::DescriptorWrapper* read,
                     ::protobuf::qml::DescriptorWrapper* write);

  // To be called from IO thread
  void onData(ServerReaderCallData* cdata);
  void onDataEnd(ServerReaderCallData* cdata);

  void startProcessing() final;
  bool respond(int tag,
               std::unique_ptr<google::protobuf::Message>) final;
  bool abort(int tag, int code, const QString& message) final;

private:
  ::protobuf::qml::DescriptorWrapper* read_;
  ::protobuf::qml::DescriptorWrapper* write_;
  ::grpc::ServerCompletionQueue* cq_;
  int index_;
  GrpcService* service_;
};

class ServerReaderCallData : public CallData {
public:
  ServerReaderCallData(ServerReaderMethod* method,
                       GrpcService* service,
                       int index,
                       ::grpc::ServerCompletionQueue* cq,
                       ::protobuf::qml::DescriptorWrapper* read,
                       ::protobuf::qml::DescriptorWrapper* write);

  void process(bool ok) final;
  void resume(std::unique_ptr<google::protobuf::Message>);
  void abort(int code, const QString& message);
  const std::shared_ptr<google::protobuf::Message>& data() const {
    return request_;
  }
  int tag = 0;

private:
  enum class Status {
    INIT,
    FIRST_READ,
    READ,
    FROZEN,
    WRITE,
    DONE,
  };

  Status status_ = Status::INIT;
  ::grpc::ServerContext context_;
  ::protobuf::qml::DescriptorWrapper* read_;
  ::protobuf::qml::DescriptorWrapper* write_;
  std::shared_ptr<google::protobuf::Message> request_;
  std::unique_ptr<google::protobuf::Message> response_;
  ServerReaderMethod* method_;
  ::grpc::ServerAsyncReader<google::protobuf::Message,
                            google::protobuf::Message> reader_;
  ::grpc::ServerCompletionQueue* cq_;
  int index_;
  int tag_;
  GrpcService* service_;
};
}
}
#endif  // GRPC_QML_SERVER_READER_H
