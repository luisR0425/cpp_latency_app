#pragma once
#include <string>

namespace infrastructure {

class UdpClient {
public:
  static int run(const std::string &addr_port);
};

} // namespace infrastructure
