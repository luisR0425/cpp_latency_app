#include "infrastructure/UdpServer.hpp"
#include "application/MessageHandler.hpp"
#include "domain/Message.hpp"
#include "infrastructure/Logger.hpp"

#include <chrono>
#include <cstring>
#include <iostream>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
using socklen_t = int;
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
using SOCKET = int;
inline void closesocket(SOCKET s) { close(s); }
#endif

using namespace std::chrono;

namespace infrastructure {

static SOCKET create_udp_socket() {
#ifdef _WIN32
  WSADATA wsa;
  if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
    Logger::error("WSAStartup failed with error " +
                  std::to_string(WSAGetLastError()));
    return INVALID_SOCKET;
  }
#endif
  SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (s == INVALID_SOCKET) {
#ifdef _WIN32
    Logger::error("socket() failed with WSA " +
                  std::to_string(WSAGetLastError()));
#else
    Logger::error(std::string("socket() failed: ") + strerror(errno));
#endif
  }
  return s;
}

int UdpServer::run(const std::string &addr_port) {
  Logger::init();

  Logger::info("Logger initialized (console + latency.log)");
  Logger::info("Starting UDP server on " + addr_port);

  // Parse address and port
  size_t colon = addr_port.find(':');
  if (colon == std::string::npos) {
    Logger::error("Invalid address:port format");
    return 1;
  }

  std::string addr_str = addr_port.substr(0, colon);
  int port = std::stoi(addr_port.substr(colon + 1));

  SOCKET s = create_udp_socket();
  if (s == INVALID_SOCKET)
    return 1;

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  if (inet_pton(AF_INET, addr_str.c_str(), &addr.sin_addr) <= 0) {
    Logger::error("Invalid IP address: " + addr_str);
    closesocket(s);
#ifdef _WIN32
    WSACleanup();
#endif
    return 1;
  }

  if (bind(s, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) ==
      SOCKET_ERROR) {
#ifdef _WIN32
    int err = WSAGetLastError();
    Logger::error("bind() failed, WSAGetLastError=" + std::to_string(err));
#else
    Logger::error(std::string("bind() failed: ") + strerror(errno));
#endif
    closesocket(s);
#ifdef _WIN32
    WSACleanup();
#endif
    return 1;
  }

  Logger::info("UDP server running @ " + addr_port);

  sockaddr_in client{};
  socklen_t client_len = sizeof(client);
  char buffer[1024];

  while (true) {
    std::memset(buffer, 0, sizeof(buffer));
    int len = recvfrom(s, buffer, sizeof(buffer) - 1, 0,
                       reinterpret_cast<sockaddr *>(&client), &client_len);

    if (len < 0) {
#ifdef _WIN32
      int err = WSAGetLastError();
      Logger::error("recvfrom() failed, WSA=" + std::to_string(err));
#else
      Logger::error(std::string("recvfrom() failed: ") + strerror(errno));
#endif
      continue;
    }

    buffer[len] = '\0';
    std::string msg(buffer);

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client.sin_addr), client_ip, INET_ADDRSTRLEN);
    int client_port = ntohs(client.sin_port);

    Logger::info("Received from " + std::string(client_ip) + ":" +
                 std::to_string(client_port) + " msg='" + msg + "'");

    auto start = high_resolution_clock::now();
    std::string response = "Echo: " + msg;
    auto end = high_resolution_clock::now();
    auto dt = duration_cast<microseconds>(end - start).count();

    Logger::info("Latency: " + std::to_string(dt) + " us for message '" + msg +
                 "'");

    sendto(s, response.c_str(), static_cast<int>(response.size()), 0,
           reinterpret_cast<sockaddr *>(&client), client_len);

    if (msg == "exit" || msg == "quit") {
      Logger::info("Server shutting down by client request...");
      break;
    }
  }

  closesocket(s);
#ifdef _WIN32
  WSACleanup();
#endif
  Logger::info("Server stopped cleanly.");
  return 0;
}

} // namespace infrastructure
