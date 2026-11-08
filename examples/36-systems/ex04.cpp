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
