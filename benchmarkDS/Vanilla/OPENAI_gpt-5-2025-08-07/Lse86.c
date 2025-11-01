// Note: This single-file example uses POSIX sockets and MySQL C API.
// It provides a minimal HTTP server at /api/temperature_for_location.
// Compile linking with -lmysqlclient and appropriate includes.
// On Windows, Winsock setup is not included.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef _WIN32
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include <mysql/mysql.h>

static const char* getenv_str_c(const char* k, const char* defv) {
    const char* v = getenv(k);
    return v ? v : defv;
}

static unsigned int getenv_uint_c(const char* k, unsigned int defv) {
    const char* v = getenv(k);
    if (!v) return defv;
    char* end = NULL;
    unsigned long x = strtoul(v, &end, 10);
    if (end == v) return defv;
    return (unsigned int)x;
}

struct QueryResult {
    int has_error;
    char error[512];
    int has_value;
    double value;
};

// Function to query max temperature with parameters, returns in struct
static struct QueryResult query_max_temperature_c(
    double latitude,
    double longitude,
    int year,
    int month,
    int day,
    const char* grib_file,
    const char* host,
    unsigned int port,
    const char* user,
    const char* pass,
    const char* db
) {
    struct QueryResult r;
    r.has_error = 0; r.error[0] = '\0'; r.has_value = 0; r.value = 0.0;

    MYSQL* conn = mysql_init(NULL);
    if (!conn) {
        r.has_error = 1; snprintf(r.error, sizeof(r.error), "mysql_init failed");
        return r;
    }
    if (!mysql_real_connect(conn, host, user, pass, db, port, NULL, 0)) {
        r.has_error = 1; snprintf(r.error, sizeof(r.error), "%s", mysql_error(conn));
        mysql_close(conn); return r;
    }

    const char* sql = "SELECT MAX(temperature) AS max_temp FROM temperatures "
                      "WHERE latitude=? AND longitude=? AND year=? AND month=? AND day=? AND grib_file=?";
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    if (!stmt) {
        r.has_error = 1; snprintf(r.error, sizeof(r.error), "mysql_stmt_init failed");
        mysql_close(conn); return r;
    }
    if (mysql_stmt_prepare(stmt, sql, (unsigned long)strlen(sql)) != 0) {
        r.has_error = 1; snprintf(r.error, sizeof(r.error), "%s", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt); mysql_close(conn); return r;
    }

    MYSQL_BIND bind[6];
    memset(bind, 0, sizeof(bind));
    double lat = latitude, lon = longitude;
    int y = year, m = month, d = day;
    unsigned long grib_len = (unsigned long)strlen(grib_file);

    bind[0].buffer_type = MYSQL_TYPE_DOUBLE; bind[0].buffer = (void*)&lat;
    bind[1].buffer_type = MYSQL_TYPE_DOUBLE; bind[1].buffer = (void*)&lon;
    bind[2].buffer_type = MYSQL_TYPE_LONG; bind[2].buffer = (void*)&y;
    bind[3].buffer_type = MYSQL_TYPE_LONG; bind[3].buffer = (void*)&m;
    bind[4].buffer_type = MYSQL_TYPE_LONG; bind[4].buffer = (void*)&d;
    bind[5].buffer_type = MYSQL_TYPE_STRING; bind[5].buffer = (void*)grib_file; bind[5].buffer_length = grib_len; bind[5].length = &grib_len;

    if (mysql_stmt_bind_param(stmt, bind) != 0) {
        r.has_error = 1; snprintf(r.error, sizeof(r.error), "%s", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt); mysql_close(conn); return r;
    }
    if (mysql_stmt_execute(stmt) != 0) {
        r.has_error = 1; snprintf(r.error, sizeof(r.error), "%s", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt); mysql_close(conn); return r;
    }

    double max_temp = 0.0;
    my_bool is_null = 0;
    MYSQL_BIND outb[1];
    memset(outb, 0, sizeof(outb));
    outb[0].buffer_type = MYSQL_TYPE_DOUBLE;
    outb[0].buffer = (void*)&max_temp;
    outb[0].is_null = &is_null;

    if (mysql_stmt_bind_result(stmt, outb) != 0) {
        r.has_error = 1; snprintf(r.error, sizeof(r.error), "%s", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt); mysql_close(conn); return r;
    }

    if (mysql_stmt_fetch(stmt) == 0) {
        if (!is_null) { r.has_value = 1; r.value = max_temp; }
    }

    mysql_stmt_close(stmt);
    mysql_close(conn);
    return r;
}

#ifndef _WIN32
static void send_all(int fd, const char* data, size_t len) {
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = send(fd, data + sent, len - sent, 0);
        if (n <= 0) break;
        sent += (size_t)n;
    }
}

static void handle_client_c(int fd) {
    char buf[4096];
    int n = recv(fd, buf, sizeof(buf)-1, 0);
    if (n <= 0) { close(fd); return; }
    buf[n] = '\0';
    char method[8] = {0}, target[2048] = {0};
    if (sscanf(buf, "%7s %2047s", method, target) != 2) {
        close(fd); return;
    }
    if (strcmp(method, "GET") != 0) {
        const char* body = "{\"error\":\"Method not allowed\"}";
        char hdr[256];
        snprintf(hdr, sizeof(hdr), "HTTP/1.1 405 Method Not Allowed\r\nContent-Type: application/json\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n", strlen(body));
        send_all(fd, hdr, strlen(hdr)); send_all(fd, body, strlen(body)); close(fd); return;
    }

    char path[2048]; strncpy(path, target, sizeof(path)-1); path[sizeof(path)-1] = '\0';
    char* q = strchr(path, '?');
    char* query = NULL;
    if (q) { *q = '\0'; query = q + 1; }

    if (strcmp(path, "/api/temperature_for_location") != 0) {
        const char* body = "{\"error\":\"Not Found\"}";
        char hdr[256];
        snprintf(hdr, sizeof(hdr), "HTTP/1.1 404 Not Found\r\nContent-Type: application/json\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n", strlen(body));
        send_all(fd, hdr, strlen(hdr)); send_all(fd, body, strlen(body)); close(fd); return;
    }

    // crude query parsing
    char *lat_s=NULL, *lon_s=NULL, *y_s=NULL, *m_s=NULL, *d_s=NULL, *g_s=NULL;
    if (query) {
        char* qcopy = strdup(query);
        char* tok = strtok(qcopy, "&");
        while (tok) {
            char* eq = strchr(tok, '=');
            if (eq) {
                *eq = '\0';
                char* key = tok; char* val = eq+1;
                if (strcmp(key,"latitude")==0) lat_s = strdup(val);
                else if (strcmp(key,"longitude")==0) lon_s = strdup(val);
                else if (strcmp(key,"year")==0) y_s = strdup(val);
                else if (strcmp(key,"month")==0) m_s = strdup(val);
                else if (strcmp(key,"day")==0) d_s = strdup(val);
                else if (strcmp(key,"grib_file")==0) g_s = strdup(val);
            }
            tok = strtok(NULL, "&");
        }
        free(qcopy);
    }
    const char* missing = NULL;
    if (!lat_s) missing="latitude";
    else if (!lon_s) missing="longitude";
    else if (!y_s) missing="year";
    else if (!m_s) missing="month";
    else if (!d_s) missing="day";
    else if (!g_s) missing="grib_file";

    if (missing) {
        char body[256];
        snprintf(body, sizeof(body), "{\"error\":\"Missing parameter: %s\"}", missing);
        char hdr[256];
        snprintf(hdr, sizeof(hdr), "HTTP/1.1 400 Bad Request\r\nContent-Type: application/json\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n", strlen(body));
        send_all(fd, hdr, strlen(hdr)); send_all(fd, body, strlen(body)); close(fd);
        goto cleanup;
    }

    double lat = atof(lat_s);
    double lon = atof(lon_s);
    int year = atoi(y_s);
    int month = atoi(m_s);
    int day = atoi(d_s);
    const char* grib = g_s;

    const char* host = getenv_str_c("DB_HOST", "127.0.0.1");
    unsigned int dbport = getenv_uint_c("DB_PORT", 3306);
    const char* user = getenv_str_c("DB_USER", "root");
    const char* pass = getenv_str_c("DB_PASS", "");
    const char* db = getenv_str_c("DB_NAME", "weather");

    struct QueryResult qr = query_max_temperature_c(lat, lon, year, month, day, grib, host, dbport, user, pass, db);
    if (qr.has_error) {
        char body[1024];
        snprintf(body, sizeof(body), "{\"error\":\"%s\"}", qr.error);
        char hdr[256];
        snprintf(hdr, sizeof(hdr), "HTTP/1.1 500 Internal Server Error\r\nContent-Type: application/json\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n", strlen(body));
        send_all(fd, hdr, strlen(hdr)); send_all(fd, body, strlen(body)); close(fd);
        goto cleanup;
    } else {
        char body[1024];
        if (qr.has_value) {
            snprintf(body, sizeof(body),
                "{\"latitude\":%.6f,\"longitude\":%.6f,\"year\":%d,\"month\":%d,\"day\":%d,\"grib_file\":\"%s\",\"max_temperature\":%.6f}",
                lat, lon, year, month, day, grib, qr.value);
        } else {
            snprintf(body, sizeof(body),
                "{\"latitude\":%.6f,\"longitude\":%.6f,\"year\":%d,\"month\":%d,\"day\":%d,\"grib_file\":\"%s\",\"max_temperature\":null}",
                lat, lon, year, month, day, grib);
        }
        char hdr[256];
        snprintf(hdr, sizeof(hdr), "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n", strlen(body));
        send_all(fd, hdr, strlen(hdr)); send_all(fd, body, strlen(body)); close(fd);
    }

cleanup:
    if (lat_s) free(lat_s);
    if (lon_s) free(lon_s);
    if (y_s) free(y_s);
    if (m_s) free(m_s);
    if (d_s) free(d_s);
    if (g_s) free(g_s);
}

static void server_loop_c(int port) {
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) { perror("socket"); return; }
    int opt = 1; setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr; memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET; addr.sin_addr.s_addr = htonl(INADDR_ANY); addr.sin_port = htons(port);
    if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) != 0) { perror("bind"); close(s); return; }
    if (listen(s, 16) != 0) { perror("listen"); close(s); return; }

    while (1) {
        int c = accept(s, NULL, NULL);
        if (c < 0) continue;
        handle_client_c(c);
    }
}
#endif

int main() {
#ifndef _WIN32
    int port = (int)getenv_uint_c("C_PORT", 8083);
    pid_t pid = fork();
    if (pid == 0) {
        server_loop_c(port);
        exit(0);
    }
#else
    // Windows: server not implemented in this snippet
#endif

    // 5 test cases calling the function directly
    const char* host = getenv_str_c("DB_HOST", "127.0.0.1");
    unsigned int dbport = getenv_uint_c("DB_PORT", 3306);
    const char* user = getenv_str_c("DB_USER", "root");
    const char* pass = getenv_str_c("DB_PASS", "");
    const char* db = getenv_str_c("DB_NAME", "weather");

    struct TC { double lat, lon; int y,m,d; const char* gf; } tests[5] = {
        {40.7128, -74.0060, 2024,1,15, "gfs_20240115.grb2"},
        {34.0522, -118.2437, 2023,7,4, "era5_20230704.grib"},
        {51.5074, -0.1278, 2022,12,25, "ukmet_20221225.grb"},
        {35.6895, 139.6917, 2021,8,1, "jma_20210801.grb2"},
        {-33.8688, 151.2093, 2020,2,29, "bom_20200229.grib"},
    };

    for (int i=0;i<5;i++) {
        struct QueryResult qr = query_max_temperature_c(
            tests[i].lat, tests[i].lon, tests[i].y, tests[i].m, tests[i].d, tests[i].gf,
            host, dbport, user, pass, db
        );
        if (qr.has_error) {
            printf("C test error: %s\n", qr.error);
        } else {
            if (qr.has_value) printf("C test result for %s: %.6f\n", tests[i].gf, qr.value);
            else printf("C test result for %s: null\n", tests[i].gf);
        }
    }
    return 0;
}