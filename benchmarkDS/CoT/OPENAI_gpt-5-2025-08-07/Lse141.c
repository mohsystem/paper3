#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

typedef struct {
    int listen_fd;
    int port;
} WebApp;

WebApp create_web_app(int port) {
    WebApp app;
    app.listen_fd = -1;
    app.port = port;

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return app;

    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons((unsigned short)port);

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(fd);
        return app;
    }
    if (listen(fd, 4) < 0) {
        close(fd);
        return app;
    }
    app.listen_fd = fd;
    return app;
}

const char* get_server_name() {
    return "localhost";
}

const char* get_database_name() {
    return "appdb";
}

const char* get_database_user() {
    return "appuser";
}

const char* get_database_password() {
    return "secret";
}

typedef struct {
    const char* server;
    const char* database;
    const char* user;
    const char* password;
    bool open;
} DbConnection;

DbConnection get_database_connection(const char* server,
                                     const char* database,
                                     const char* user,
                                     const char* password) {
    DbConnection c;
    c.server = server;
    c.database = database;
    c.user = user;
    c.password = password;
    c.open = true;
    return c;
}

int main() {
    WebApp app = create_web_app(8082);

    const char* s1 = get_server_name();
    printf("Test1 - Server Name: %s\n", s1);

    const char* s2 = get_database_name();
    printf("Test2 - Database Name: %s\n", s2);

    const char* s3 = get_database_user();
    printf("Test3 - Database User: %s\n", s3);

    const char* s4 = get_database_password();
    printf("Test4 - Database Password (masked): ");
    for (size_t i = 0; i < strlen(s4); ++i) putchar('*');
    putchar('\n');

    DbConnection conn = get_database_connection(s1, s2, s3, s4);
    printf("Test5 - Connection: server=%s db=%s user=%s open=%s\n",
           conn.server, conn.database, conn.user, conn.open ? "true" : "false");

    if (app.listen_fd >= 0) close(app.listen_fd);
    return 0;
}