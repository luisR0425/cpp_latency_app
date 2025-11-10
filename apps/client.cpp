#include "infrastructure/Logger.hpp"
#include "infrastructure/UdpClient.hpp"
#include <filesystem>
#include <iostream>

int main() {
  namespace fs = std::filesystem;

  fs::path current = fs::current_path();
  while (current.has_parent_path() && current.filename() != "cpp_latency_app") {
    current = current.parent_path();
  }

  fs::path log_path = current / "latency_client.log";

  infrastructure::Logger::init(log_path.string());
  infrastructure::Logger::info("Client logger initialized at: " +
                               log_path.string());
  std::cout << "Log path: " << log_path << std::endl;

  return infrastructure::UdpClient::run("127.0.0.1:9000");
}