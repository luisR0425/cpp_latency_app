#pragma once
#include <string>

namespace infrastructure {

class UdpServer {
public:
  static int run(const std::string &addr_port);
};

} // namespace infrastructure
