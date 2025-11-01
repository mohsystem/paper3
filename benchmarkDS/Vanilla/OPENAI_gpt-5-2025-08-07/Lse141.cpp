#include <iostream>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

struct DbConfig {
    std::string server;
    std::string database;
    std::string user;
    std::string password;
};

struct DbConnection {
    std::string server;
    std::string database;
    std::string user;
    bool connected;
};

// Create a web app (listening socket). Returns listening socket fd or -1 on error.
int create_web_app(int port) {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -1;

    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(static_cast<uint16_t>(port));
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1

    if (bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        ::close(fd);
        return -1;
    }
    if (listen(fd, 10) < 0) {
        ::close(fd);
        return -1;
    }
    return fd;
}

std::string getServerName(const DbConfig& config) {
    return config.server;
}

std::string getDatabaseName(const DbConfig& config) {
    return config.database;
}

std::string getDatabaseUser(const DbConfig& config) {
    return config.user;
}

std::string getDatabasePassword(const DbConfig& config) {
    return config.password;
}

DbConnection getDatabaseConnection(const DbConfig& config) {
    DbConnection c{config.server, config.database, config.user, true};
    return c;
}

int main() {
    DbConfig cfg{"localhost", "sampledb", "admin", "secret"};

    // Test case 1: Server name
    std::cout << "Server Name: " << getServerName(cfg) << std::endl;

    // Test case 2: Database name
    std::cout << "Database Name: " << getDatabaseName(cfg) << std::endl;

    // Test case 3: Database user
    std::cout << "Database User: " << getDatabaseUser(cfg) << std::endl;

    // Test case 4: Database password
    std::cout << "Database Password: " << getDatabasePassword(cfg) << std::endl;

    // Test case 5: Database connection
    DbConnection conn = getDatabaseConnection(cfg);
    std::cout << "Database Connection: "
              << "{server='" << conn.server
              << "', database='" << conn.database
              << "', user='" << conn.user
              << "', connected=" << (conn.connected ? "true" : "false")
              << "}" << std::endl;

    int server_fd = create_web_app(9092);
    if (server_fd >= 0) {
        std::cout << "C++ web app listening on 127.0.0.1:9092" << std::endl;
        ::close(server_fd);
        std::cout << "C++ web app stopped." << std::endl;
    } else {
        std::cout << "Failed to start C++ web app." << std::endl;
    }

    return 0;
}