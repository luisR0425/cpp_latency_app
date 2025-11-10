#pragma once
#include <string>

namespace domain {

struct Message {
  std::string content;
  explicit Message(std::string c) : content(std::move(c)) {}
  std::string process() const { return "Echo: " + content; }
};

} // namespace domain
