cmake_minimum_required(VERSION 3.16)
project(cpp_latency_app LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

add_library(core
    src/domain/Message.cpp
    src/application/MessageHandler.cpp
    src/infrastructure/Logger.cpp
    src/infrastructure/UdpServer.cpp
    src/infrastructure/UdpClient.cpp
)

target_include_directories(core PUBLIC src)

if(WIN32)
  target_link_libraries(core PUBLIC ws2_32)
endif()

add_executable(server apps/server.cpp)
target_link_libraries(server PRIVATE core)

add_executable(client apps/client.cpp)
target_link_libraries(client PRIVATE core)

target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)