#pragma once
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>

namespace infrastructure {

class Logger {
public:
  static void init(const std::string &file = "latency.log");
  static void info(const std::string &msg);
  static void error(const std::string &msg);

private:
  static std::mutex mutex_;
  static std::ofstream file_;
  static bool initialized_;

  static void write(const std::string &level, const std::string &msg);
};

} // namespace infrastructure