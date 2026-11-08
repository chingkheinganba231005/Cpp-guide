Systems programming means working directly with the **operating system**: files, memory, processes, threads and the network. C++ gives you portable standard-library tools for most of it, and direct access to the OS's own interfaces (POSIX on Linux and macOS, Win32 on Windows) when you need full control. This chapter covers the essentials and ends with a working TCP server.

## Files with the standard library

```cpp run
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

int main() {
    fs::create_directories("data/logs");
    {
        std::ofstream out("data/logs/run.csv");                // RAII: closed at the end of the block
        out << "step,loss\n";
        for (int i = 0; i < 5; ++i) out << i << ',' << 1.0 / (i + 1) << '\n';
    }
    std::ifstream in("data/logs/run.csv");
    std::string line;
    int lines = 0;
    while (std::getline(in, line)) ++lines;
    std::cout << "read " << lines << " lines\n";
    for (const auto& entry : fs::recursive_directory_iterator("data"))
        if (entry.is_regular_file()) std::cout << entry.path().string() << "  " << entry.file_size() << " bytes\n";
    fs::remove_all("data");
}
```

For large binary data, read and write in big blocks (`in.read(buffer, n)`), not character by character.

## Memory-mapped files: let the OS page data in

`mmap` maps a file straight into your address space. The OS loads pages lazily as you touch them, and multiple processes can share them. Databases, game asset loaders and market-data replay tools use it to treat huge files as plain arrays:

```cpp run
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

int main() {
    const char* path = "prices.bin";
    {                                                              // write 1 million doubles to a file
        std::FILE* f = std::fopen(path, "wb");
        for (int i = 0; i < 1'000'000; ++i) { double p = 100.0 + i * 0.001; std::fwrite(&p, sizeof p, 1, f); }
        std::fclose(f);
    }
    int fd = open(path, O_RDONLY);
    struct stat st{};
    fstat(fd, &st);
    void* mem = mmap(nullptr, static_cast<size_t>(st.st_size), PROT_READ, MAP_PRIVATE, fd, 0);   // POSIX: Linux, macOS
    if (mem == MAP_FAILED) { std::perror("mmap"); return 1; }
    const double* prices = static_cast<const double*>(mem);       // the file IS an array now
    std::size_t n = static_cast<size_t>(st.st_size) / sizeof(double);
    double sum = 0;
    for (std::size_t i = 0; i < n; ++i) sum += prices[i];
    std::cout << n << " prices, mean " << sum / n << ", last " << prices[n - 1] << '\n';
    munmap(mem, static_cast<size_t>(st.st_size));
    close(fd);
    std::remove(path);
}
```

On Windows the equivalent is `CreateFileMapping` and `MapViewOfFile`. Cross-platform code wraps these in a small RAII class (or uses Boost.Interprocess or `mio`).

## Processes

A **process** is a running program with its own address space. Threads share memory; processes don't, so they talk through pipes, sockets, files or shared memory. Launching another program with POSIX `posix_spawn` and reading its output through a pipe:

```cpp run
#include <array>
#include <cstdio>
#include <iostream>
#include <spawn.h>
#include <string>
#include <sys/wait.h>
#include <unistd.h>

extern char** environ;

int main() {
    int fds[2];
    if (pipe(fds) != 0) { std::perror("pipe"); return 1; }        // fds[0] read end, fds[1] write end
    posix_spawn_file_actions_t fa;
    posix_spawn_file_actions_init(&fa);
    posix_spawn_file_actions_adddup2(&fa, fds[1], STDOUT_FILENO);  // the child's stdout goes into the pipe
    posix_spawn_file_actions_addclose(&fa, fds[0]);
    char cmd[] = "echo", arg[] = "hello from a child process";
    char* argv[] = {cmd, arg, nullptr};
    pid_t pid;
    if (posix_spawnp(&pid, "echo", &fa, nullptr, argv, environ) != 0) { std::perror("spawn"); return 1; }
    close(fds[1]);
    std::string out;
    std::array<char, 256> buf;
    for (ssize_t n; (n = read(fds[0], buf.data(), buf.size())) > 0;) out.append(buf.data(), static_cast<size_t>(n));
    int status = 0;
    waitpid(pid, &status, 0);
    std::cout << "child " << pid << " said: " << out << "exit status " << WEXITSTATUS(status) << '\n';
    posix_spawn_file_actions_destroy(&fa);
}
```

## Networking: a TCP server and client

Sockets are the OS interface to the network. Here a server thread listens on a local port, and the main thread connects as a client and sends a few messages, which the server echoes back in upper case. Every web server, database and trading gateway starts from these calls: `socket`, `bind`, `listen`, `accept`, `connect`, `send` and `recv`.

```cpp run
#include <arpa/inet.h>
#include <cctype>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

class Socket {                                                     // RAII: the file descriptor is always closed
public:
    explicit Socket(int fd) : fd_(fd) {}
    ~Socket() { if (fd_ >= 0) close(fd_); }
    Socket(Socket&& o) noexcept : fd_(o.fd_) { o.fd_ = -1; }
    Socket(const Socket&) = delete;
    int fd() const { return fd_; }
private:
    int fd_;
};

int main() {
    Socket listener(socket(AF_INET, SOCK_STREAM, 0));
    int yes = 1;
    setsockopt(listener.fd(), SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);                 // 127.0.0.1 only
    addr.sin_port = 0;                                             // let the OS pick a free port
    bind(listener.fd(), reinterpret_cast<sockaddr*>(&addr), sizeof addr);
    listen(listener.fd(), 1);
    socklen_t len = sizeof addr;
    getsockname(listener.fd(), reinterpret_cast<sockaddr*>(&addr), &len);
    std::cout << "server listening on 127.0.0.1:" << ntohs(addr.sin_port) << '\n';

    std::jthread server([&] {
        Socket conn(accept(listener.fd(), nullptr, nullptr));
        char buf[256];
        for (ssize_t n; (n = recv(conn.fd(), buf, sizeof buf, 0)) > 0;) {      // 0 means the client closed the connection
            for (ssize_t i = 0; i < n; ++i) buf[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(buf[i])));
            send(conn.fd(), buf, static_cast<size_t>(n), 0);
        }
    });

    Socket client(socket(AF_INET, SOCK_STREAM, 0));
    connect(client.fd(), reinterpret_cast<sockaddr*>(&addr), sizeof addr);
    setsockopt(client.fd(), IPPROTO_TCP, TCP_NODELAY, &yes, sizeof yes);       // send small messages immediately (no Nagle delay)
    for (std::string msg : {"buy 100 aapl", "sell 50 msft", "cancel 7"}) {
        send(client.fd(), msg.data(), msg.size(), 0);
        char buf[256];
        ssize_t n = recv(client.fd(), buf, sizeof buf, 0);
        std::cout << "sent '" << msg << "', got back '" << std::string(buf, static_cast<size_t>(n)) << "'\n";
    }
    shutdown(client.fd(), SHUT_WR);                                // tell the server we're done
}
```

TCP is a **byte stream**, not a stream of messages: one `send` can arrive split across several `recv` calls, or merged with the next one. Real protocols add framing, such as a length prefix before each message or a delimiter like `\n`. This example works because each message is small and the client waits for each reply.

Servers that handle thousands of connections use **event loops** (`epoll` on Linux, `kqueue` on macOS, IOCP on Windows) instead of one thread per connection. Libraries such as Asio (also in Boost) wrap them portably with modern C++. The lowest-latency trading systems go further and bypass the kernel's network stack entirely (DPDK, OpenOnload).

## Talking to C: the systems lingua franca

Operating-system interfaces are C APIs. Wrapping them in small RAII classes, as `Socket` above does, is idiomatic C++: you keep the full power of the C interface, and leaks and forgotten `close` calls become impossible. `extern "C"` lets C++ code expose functions that C, Python (through `ctypes` or `cffi`), Rust and others can call.

::quiz
? What does mmap give you?
+ A file's contents mapped into memory, accessed like an array and loaded lazily by the OS
- A faster std::map
- A copy of the file in a vector
> The OS pages data in when it is touched.
? A client sends "AB" then "CD". What might a single recv on the server return?
+ "ABCD", "AB", or even just "A": TCP is a byte stream
- Always exactly "AB"
- An error
> Message boundaries must be added by your own protocol (framing).
::
