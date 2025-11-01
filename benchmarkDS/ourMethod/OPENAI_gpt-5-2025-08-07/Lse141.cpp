#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <cerrno>
#include <csignal>
#include <cstring>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <vector>

struct WebApp {
    int listen_fd{-1};
    uint16_t port{0};
    std::atomic<bool> running{false};
    std::thread worker;

    WebApp() = default;
    WebApp(const WebApp&) = delete;
    WebApp& operator=(const WebApp&) = delete;

    ~WebApp() {
        stop();
    }

    void stop() {
        if (running.exchange(false)) {
            if (listen_fd >= 0) {
                close(listen_fd);
                listen_fd = -1;
            }
            if (worker.joinable()) {
                worker.join();
            }
        }
    }
};

static void server_loop(WebApp* app) {
    const char* ok_resp = "HTTP/1.1 200 OK\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: 2\r\nConnection: close\r\n\r\nOK";
    while (app->running.load()) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(app->listen_fd, &rfds);
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 200000; // 200ms
        int sel = select(app->listen_fd + 1, &rfds, nullptr, nullptr, &tv);
        if (sel < 0) {
            if (errno == EINTR) continue;
            break;
        }
        if (sel == 0) {
            continue;
        }
        if (FD_ISSET(app->listen_fd, &rfds)) {
            struct sockaddr_in cli;
            socklen_t clilen = sizeof(cli);
            int cfd = accept(app->listen_fd, reinterpret_cast<struct sockaddr*>(&cli), &clilen);
            if (cfd < 0) {
                if (errno == EINTR) continue;
                continue;
            }
            char buf[1024];
            ssize_t r = recv(cfd, buf, sizeof(buf) - 1, 0);
            (void)r; // request not used; avoid leaking details
            send(cfd, ok_resp, strlen(ok_resp), 0);
            close(cfd);
        }
    }
}

// Create a minimal HTTP server on 127.0.0.1:port (0 for ephemeral)
WebApp create_web_app(uint16_t port) {
    WebApp app;
    app.listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (app.listen_fd < 0) {
        std::cerr << "Failed to create socket\n";
        return app;
    }
    int yes = 1;
    setsockopt(app.listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1
    addr.sin_port = htons(port);

    if (bind(app.listen_fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        std::cerr << "Bind failed\n";
        close(app.listen_fd);
        app.listen_fd = -1;
        return app;
    }

    // discover actual port if ephemeral
    socklen_t len = sizeof(addr);
    if (getsockname(app.listen_fd, reinterpret_cast<struct sockaddr*>(&addr), &len) == 0) {
        app.port = ntohs(addr.sin_port);
    }

    if (listen(app.listen_fd, 16) < 0) {
        std::cerr << "Listen failed\n";
        close(app.listen_fd);
        app.listen_fd = -1;
        return app;
    }

    app.running = true;
    app.worker = std::thread(server_loop, &app);
    return app;
}

// Securely read env (no hardcoded secrets)
std::string get_server_name() {
    const char* v = std::getenv("DB_HOST");
    return (v && *v) ? std::string(v) : std::string("127.0.0.1");
}

std::string get_database_name() {
    const char* v = std::getenv("DB_NAME");
    return (v && *v) ? std::string(v) : std::string("appdb");
}

std::string get_database_user() {
    const char* v = std::getenv("DB_USER");
    return (v) ? std::string(v) : std::string();
}

// Do not print or log this value
std::string get_database_password() {
    const char* v = std::getenv("DB_PASSWORD");
    return (v) ? std::string(v) : std::string();
}

struct DBConnection {
    std::string host;
    std::string db;
    std::string user;
    // No password stored here for safety in this demo
};

// Returns a "connection object" only if explicitly enabled. No real DB I/O here.
std::unique_ptr<DBConnection> get_database_connection() {
    const char* flag = std::getenv("DB_CONNECT");
    if (!(flag && std::string(flag) == "true")) {
        return nullptr;
    }
    auto conn = std::make_unique<DBConnection>();
    conn->host = get_server_name();
    conn->db = get_database_name();
    conn->user = get_database_user();
    return conn;
}

int main() {
    // Start web app on ephemeral port
    WebApp app = create_web_app(0);
    if (app.listen_fd >= 0) {
        std::cout << "[Test1] Web app started on 127.0.0.1:" << app.port << "\n";
    } else {
        std::cout << "[Test1] Web app failed to start\n";
    }

    // Test2: Server name
    std::cout << "[Test2] DB Server Name: " << get_server_name() << "\n";

    // Test3: Database name
    std::cout << "[Test3] DB Name: " << get_database_name() << "\n";

    // Test4: Database user
    auto user = get_database_user();
    std::cout << "[Test4] DB User: " << (user.empty() ? "(not set)" : user) << "\n";

    // Test5: Password presence (do not print secret)
    auto pwd = get_database_password();
    std::cout << "[Test5] DB Password set: " << (!pwd.empty() ? "true" : "false") << "\n";

    // Optional: connection object (no real DB connection)
    auto conn = get_database_connection();
    if (conn) {
        std::cout << "[Extra] Connection object prepared for " << conn->user << "@" << conn->host << "/" << conn->db << "\n";
    } else {
        std::cout << "[Extra] Connection not prepared (expected unless DB_CONNECT=true)\n";
    }

    app.stop();
    std::cout << "[Done] Web app stopped.\n";
    return 0;
}