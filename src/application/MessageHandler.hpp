#pragma once
#include "domain/Message.hpp"
#include <memory>
#include <string>

namespace application {

/// Interface (Port) for sending messages
struct MessagePort {
  virtual ~MessagePort() = default;
  virtual void send(const domain::Message &msg) = 0;
};

/// Application service (use case layer)
class MessageHandler {
public:
  explicit MessageHandler(std::unique_ptr<MessagePort> port);
  void handle(const std::string &raw);

private:
  std::unique_ptr<MessagePort> port_;
};

} // namespace application
