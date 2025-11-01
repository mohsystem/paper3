#include <iostream>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

struct WebApp {
    int listen_fd;
    int port;
};

WebApp create_web_app(int port) {
    WebApp app{ -1, port };
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return app;

    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(static_cast<uint16_t>(port));

    if (bind(fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        ::close(fd);
        return app;
    }
    if (listen(fd, 4) < 0) {
        ::close(fd);
        return app;
    }
    app.listen_fd = fd;
    return app;
}

std::string get_server_name() {
    return "localhost";
}

std::string get_database_name() {
    return "appdb";
}

std::string get_database_user() {
    return "appuser";
}

std::string get_database_password() {
    return "secret";
}

struct DbConnection {
    std::string server;
    std::string database;
    std::string user;
    std::string password;
    bool open;
};

DbConnection get_database_connection(const std::string& server,
                                     const std::string& database,
                                     const std::string& user,
                                     const std::string& password) {
    DbConnection c{server, database, user, password, true};
    return c;
}

int main() {
    WebApp app = create_web_app(8081);

    std::string s1 = get_server_name();
    std::cout << "Test1 - Server Name: " << s1 << "\n";

    std::string s2 = get_database_name();
    std::cout << "Test2 - Database Name: " << s2 << "\n";

    std::string s3 = get_database_user();
    std::cout << "Test3 - Database User: " << s3 << "\n";

    std::string s4 = get_database_password();
    std::cout << "Test4 - Database Password (masked): " << std::string(s4.size(), '*') << "\n";

    DbConnection conn = get_database_connection(s1, s2, s3, s4);
    std::cout << "Test5 - Connection: server=" << conn.server
              << " db=" << conn.database << " user=" << conn.user
              << " open=" << (conn.open ? "true" : "false") << "\n";

    if (app.listen_fd >= 0) ::close(app.listen_fd);
    return 0;
}