#ifndef GRPC_GRML_WRITER_H
#define GRPC_GRML_WRITER_H

#include "grpc/qml/base.h"
#include "grpc/qml/server_calldata.h"
#include "protobuf/qml/method.h"

#include <grpc++/impl/proto_utils.h>
#include <mutex>
#include <queue>
#include <unordered_map>

namespace grpc {
namespace qml {

class WriterMethod;

class WriterCallData : public CallData {
public:
  WriterCallData(int tag,
                 grpc::Channel* channel,
                 ::grpc::CompletionQueue* cq,
                 WriterMethod* method,
                 ::protobuf::qml::DescriptorWrapper* read);

  ~WriterCallData();

  void process(bool ok) final;

  bool write(std::unique_ptr<google::protobuf::Message> request);
  bool writesDone();
  int timeout() const { return timeout_; }
  void set_timeout(int timeout);

private:
  enum class Status {
    INIT,
    FROZEN,
    WRITE,
    DONE,
    FINISH,
  };
  void handleQueuedRequests();

  int timeout_ = -1;
  std::mutex mutex_;
  Status status_ = Status::INIT;
  ::grpc::CompletionQueue* cq_;
  ::grpc::ClientContext context_;
  grpc::Channel* channel_;
  ::protobuf::qml::DescriptorWrapper* read_;
  WriterMethod* method_;
  int tag_;
  std::queue<std::unique_ptr<google::protobuf::Message>> requests_;
  std::unique_ptr<google::protobuf::Message> request_;
  std::shared_ptr<google::protobuf::Message> response_;
  grpc::Status grpc_status_;
  grpc::ClientAsyncWriter<google::protobuf::Message> writer_;
};

class WriterMethod : public ::protobuf::qml::WriterMethod {
  Q_OBJECT

public:
  WriterMethod(const std::string& name,
               ::protobuf::qml::DescriptorWrapper* read,
               ::protobuf::qml::DescriptorWrapper* write,
               std::shared_ptr<grpc::Channel> channel,
               grpc::CompletionQueue* cq,
               QObject* p = nullptr)
      : ::protobuf::qml::WriterMethod(p),
        name_(name),
        read_(read),
        write_(write),
        cq_(cq),
        channel_(std::move(channel)),
        raw_(name.c_str(),
             grpc::RpcMethod::CLIENT_STREAMING,
             channel_) {}

  bool write(int tag, std::unique_ptr<google::protobuf::Message> data) final;

  bool writesDone(int tag) final;

  void deleteCall(int tag);

  int timeout(int tag) const final;
  void set_timeout(int tag, int milliseconds) final;

  const grpc::RpcMethod& raw() const { return raw_; }

private:
  WriterCallData* ensureCallData(int tag);

  std::string name_;
  ::protobuf::qml::DescriptorWrapper* read_;
  ::protobuf::qml::DescriptorWrapper* write_;
  grpc::CompletionQueue* cq_;
  std::shared_ptr<grpc::Channel> channel_;
  grpc::RpcMethod raw_;
  mutable std::mutex calls_mutex_;
  std::unordered_map<int, WriterCallData*> calls_;
};
}
}
#endif  // GRPC_GRML_WRITER_H
