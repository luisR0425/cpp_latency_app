# C++ Latency App (Clean Architecture, UDP)

This is a C++ port of the Rust project: a **clean architecture** UDP echo system with
- `server`: listens on `127.0.0.1:9000`, echoes messages and logs **latency** per message
- `client`: interactive CLI that sends messages and prints **RTT**

## Build (CMake)

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

### Windows (MSVC/MinGW)
- Requires CMake >= 3.16 and a C++17 compiler.
- Links against `ws2_32` automatically.

### Linux/macOS
- No special deps.

## Run

**Server**
```bash
./build/server
```
windows
```bash
.\build\Release\server.exe
```

**Client**
```bash
./build/client
```
windows
```bash
.\build\Release\client.exe
```

Windows generic client with ncat
```bash
ncat -u 127.0.0.1 9000
```

Linux generic client
```bash
nc -u 127.0.0.1 9000
```

Type messages in the client (e.g., `hello`, `exit`).

## Architecture
- `domain/Message.hpp`: entity + domain rule
- `application/MessageHandler`: use case; depends on `MessagePort` interface
- `infrastructure/UdpServer` & `UdpClient`: adapters (sockets)
- `infrastructure/Logger`: simple console+file logger (no ANSI codes)
