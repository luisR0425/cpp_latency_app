#include "application/MessageHandler.hpp"
#include "infrastructure/Logger.hpp"
#include <chrono>

using namespace std::chrono;

namespace application {

MessageHandler::MessageHandler(std::unique_ptr<MessagePort> port)
    : port_(std::move(port)) {}

void MessageHandler::handle(const std::string &raw) {
  domain::Message msg{raw};
  auto t0 = high_resolution_clock::now();

  auto response = msg.process();
  port_->send(domain::Message{response});

  auto dt =
      duration_cast<microseconds>(high_resolution_clock::now() - t0).count();
  infrastructure::Logger::info("Latency: " + std::to_string(dt) +
                               " us for message '" + raw + "'");
}

} // namespace application
