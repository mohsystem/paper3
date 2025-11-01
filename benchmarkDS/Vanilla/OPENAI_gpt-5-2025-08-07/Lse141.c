#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct DbConfig {
    const char* server;
    const char* database;
    const char* user;
    const char* password;
};

struct DbConnection {
    const char* server;
    const char* database;
    const char* user;
    int connected; // 1 true, 0 false
};

// Create a web app (listening socket). Returns listening socket fd or -1 on error.
int create_web_app(int port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -1;

    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((unsigned short)port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(fd);
        return -1;
    }
    if (listen(fd, 10) < 0) {
        close(fd);
        return -1;
    }
    return fd;
}

const char* getServerName(const struct DbConfig* config) {
    return config->server;
}

const char* getDatabaseName(const struct DbConfig* config) {
    return config->database;
}

const char* getDatabaseUser(const struct DbConfig* config) {
    return config->user;
}

const char* getDatabasePassword(const struct DbConfig* config) {
    return config->password;
}

struct DbConnection getDatabaseConnection(const struct DbConfig* config) {
    struct DbConnection c;
    c.server = config->server;
    c.database = config->database;
    c.user = config->user;
    c.connected = 1;
    return c;
}

int main(void) {
    struct DbConfig cfg = {"localhost", "sampledb", "admin", "secret"};

    // Test case 1: Server name
    printf("Server Name: %s\n", getServerName(&cfg));

    // Test case 2: Database name
    printf("Database Name: %s\n", getDatabaseName(&cfg));

    // Test case 3: Database user
    printf("Database User: %s\n", getDatabaseUser(&cfg));

    // Test case 4: Database password
    printf("Database Password: %s\n", getDatabasePassword(&cfg));

    // Test case 5: Database connection
    struct DbConnection conn = getDatabaseConnection(&cfg);
    printf("Database Connection: {server='%s', database='%s', user='%s', connected=%s}\n",
           conn.server, conn.database, conn.user, conn.connected ? "true" : "false");

    int server_fd = create_web_app(9093);
    if (server_fd >= 0) {
        printf("C web app listening on 127.0.0.1:9093\n");
        close(server_fd);
        printf("C web app stopped.\n");
    } else {
        printf("Failed to start C web app.\n");
    }

    return 0;
}