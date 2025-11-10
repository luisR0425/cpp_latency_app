#include "infrastructure/UdpClient.hpp"
#include "infrastructure/Logger.hpp"
#include <chrono>
#include <iostream>
#include <string>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#define closesocket close
#define SOCKET int
#endif

namespace {

int parse_addr(const std::string &addr_port, std::string &ip, int &port) {
  auto pos = addr_port.find(':');
  if (pos == std::string::npos)
    return -1;
  ip = addr_port.substr(0, pos);
  port = std::stoi(addr_port.substr(pos + 1));
  return 0;
}

} // namespace

namespace infrastructure {

int UdpClient::run(const std::string &addr_port) {
#ifdef _WIN32
  WSADATA wsa;
  WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
  std::string ip;
  int port;
  if (parse_addr(addr_port, ip, port) != 0) {
    Logger::error("Invalid address: " + addr_port);
    return 1;
  }

  SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (s < 0) {
    Logger::error("socket() failed");
    return 1;
  }

  sockaddr_in server{};
  server.sin_family = AF_INET;
  server.sin_port = htons(static_cast<uint16_t>(port));
  inet_pton(AF_INET, ip.c_str(), &server.sin_addr);

  if (connect(s, reinterpret_cast<const sockaddr *>(&server), sizeof(server)) <
      0) {
    Logger::error("connect() failed");
    closesocket(s);
#ifdef _WIN32
    WSACleanup();
#endif
    return 1;
  }

  Logger::info("Connected to " + addr_port +
               ". Type messages ('exit' to quit):");
  std::cout << "Connected to " << addr_port
            << ". Type messages ('exit' to quit):\n";

  char rx[1024] = {0};

  for (;;) {
    std::string line;
    if (!std::getline(std::cin, line))
      break;
    if (line == "exit") {
      Logger::info("Client exit requested");
      break;
    }

    auto t0 = std::chrono::high_resolution_clock::now();
    int sent = send(s, line.c_str(), static_cast<int>(line.size()), 0);
    if (sent <= 0) {
      Logger::error("send() failed");
      continue;
    }
    Logger::info("Sent: " + line);

    int n = recv(s, rx, sizeof(rx) - 1, 0);
    if (n > 0) {
      rx[n] = '\0';
      auto dt = std::chrono::duration_cast<std::chrono::microseconds>(
                    std::chrono::high_resolution_clock::now() - t0)
                    .count();
      std::string resp(rx, rx + n);
      std::cout << "Server: '" << resp << "' | RTT: " << dt << " us\n";
      Logger::info("Received: '" + resp + "' | RTT: " + std::to_string(dt) +
                   " us");
    } else {
      Logger::info("(no reply)");
      std::cout << "(no reply)\n";
    }
  }

  closesocket(s);
#ifdef _WIN32
  WSACleanup();
#endif
  Logger::info("Client closed");
  return 0;
}

} // namespace infrastructure
