#include "infrastructure/Logger.hpp"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>

namespace {
std::mutex mtx;
std::ofstream out;
bool initialized = false;

std::string timestamp() {
  using namespace std::chrono;
  auto now = system_clock::now();
  auto t = system_clock::to_time_t(now);
  std::tm tm{};
#ifdef _WIN32
  localtime_s(&tm, &t);
#else
  localtime_r(&t, &tm);
#endif
  std::ostringstream ss;
  ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
  return ss.str();
}

void write_line(const char *level, const std::string &msg) {
  std::lock_guard<std::mutex> lock(mtx);
  std::string line = "[" + timestamp() + "] [" + level + "] " + msg + "\n";
  std::cout << line << std::flush;
  if (initialized && out.is_open()) {
    out << line << std::flush;
  }
}
} // namespace

namespace infrastructure {

void Logger::init(const std::string &file) {
  std::lock_guard<std::mutex> lock(mtx);
  if (initialized)
    return;

  try {
    namespace fs = std::filesystem;
    fs::path p(file);
    if (p.has_parent_path()) {
      fs::create_directories(p.parent_path());
    }
    out.open(p, std::ios::app);
    if (!out.is_open()) {
      std::cout << "Logger: failed to open: " << fs::absolute(p).string()
                << "\n"
                << std::flush;
    } else {
      std::cout << "Logger initialized -> " << fs::absolute(p).string() << "\n"
                << std::flush;
      initialized = true;
    }
  } catch (const std::exception &e) {
    std::cout << "Logger exception: " << e.what() << "\n" << std::flush;
  }
}

void Logger::info(const std::string &msg) { write_line("INFO", msg); }
void Logger::error(const std::string &msg) { write_line("ERROR", msg); }

} // namespace infrastructure