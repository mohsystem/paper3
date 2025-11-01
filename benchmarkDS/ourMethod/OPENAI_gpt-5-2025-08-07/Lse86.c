// Build: cc -std=c11 -O2 -pthread server.c -o server
// Note: This example includes a simple HTTP server and a stubbed MySQL query function.
// To integrate with MySQL C API, replace the stub in query_max_temperature with prepared statement logic.

#define _POSIX_C_SOURCE 200809L
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>

static const int PORT = 8084;
static const char* ENDPOINT = "/api/temperature_for_location";

typedef struct {
    bool success;
    double max_temperature;
    char error[48]; // "database_not_configured", "not_found", "internal_error"
} DbResult;

static bool is_number(const char* s) {
    if (!s) return false;
    size_t len = strlen(s);
    if (len == 0 || len > 64) return false;
    bool dot=false, sign=false, digit=false;
    for (size_t i=0;i<len;i++){
        char c=s[i];
        if (c=='+'||c=='-'){ if (i!=0||sign) return false; sign=true; }
        else if (c=='.'){ if (dot) return false; dot=true; }
        else if (!isdigit((unsigned char)c)) return false;
        else digit=true;
    }
    return digit;
}
static bool parse_double_in_range(const char* s, double mn, double mx, double* out) {
    if (!is_number(s)) return false;
    char* end=NULL;
    double v = strtod(s, &end);
    if (end==s || !isfinite(v)) return false;
    if (v < mn || v > mx) return false;
    *out = v;
    return true;
}
static bool parse_int_in_range(const char* s, int mn, int mx, int* out) {
    if (!s) return false;
    size_t len = strlen(s);
    if (len==0 || len>16) return false;
    for (size_t i=0;i<len;i++) if (!isdigit((unsigned char)s[i])) return false;
    long v = strtol(s, NULL, 10);
    if (v < mn || v > mx) return false;
    *out = (int)v;
    return true;
}

static char hex_val(char c) {
    if (c>='0'&&c<='9') return c-'0';
    if (c>='a'&&c<='f') return c-'a'+10;
    if (c>='A'&&c<='F') return c-'A'+10;
    return 0;
}
static char* url_decode(const char* s) {
    size_t len = strlen(s);
    char* out = (char*)malloc(len+1);
    if (!out) return NULL;
    size_t j=0;
    for (size_t i=0;i<len;i++){
        if (s[i]=='%' && i+2<len && isxdigit((unsigned char)s[i+1]) && isxdigit((unsigned char)s[i+2])) {
            char hi=hex_val(s[i+1]), lo=hex_val(s[i+2]);
            out[j++] = (char)((hi<<4) | lo);
            i+=2;
        } else if (s[i]=='+') out[j++]=' ';
        else out[j++]=s[i];
    }
    out[j]='\0';
    return out;
}

typedef struct {
    char key[65];
    char val[257];
} kv_t;
typedef struct {
    kv_t items[32];
    int count;
} query_t;

static void parse_query(const char* q, query_t* out) {
    out->count = 0;
    if (!q) return;
    size_t qlen = strlen(q);
    if (qlen==0 || qlen>2048) return;
    char* buf = (char*)malloc(qlen+1);
    if (!buf) return;
    memcpy(buf, q, qlen);
    buf[qlen]='\0';
    char* saveptr1=NULL;
    char* pair = strtok_r(buf, "&", &saveptr1);
    while (pair && out->count<32) {
        char* eq = strchr(pair, '=');
        if (eq) {
            *eq = '\0';
            char* kdec = url_decode(pair);
            char* vdec = url_decode(eq+1);
            if (kdec && vdec) {
                size_t kl = strlen(kdec), vl = strlen(vdec);
                if (kl<=64 && vl<=256) {
                    strncpy(out->items[out->count].key, kdec, 64);
                    out->items[out->count].key[64]='\0';
                    strncpy(out->items[out->count].val, vdec, 256);
                    out->items[out->count].val[256]='\0';
                    out->count++;
                }
            }
            if (kdec) free(kdec);
            if (vdec) free(vdec);
        }
        pair = strtok_r(NULL, "&", &saveptr1);
    }
    free(buf);
}
static const char* qget(const query_t* q, const char* key) {
    for (int i=0;i<q->count;i++) {
        if (strcmp(q->items[i].key, key)==0) return q->items[i].val;
    }
    return NULL;
}

static bool grib_valid(const char* s) {
    if (!s) return false;
    size_t len = strlen(s);
    if (len==0 || len>64) return false;
    for (size_t i=0;i<len;i++) {
        char c=s[i];
        if (!(isalnum((unsigned char)c) || c=='_' || c=='.' || c=='-')) return false;
    }
    return true;
}

// Stubbed DB query. Replace with real MySQL client code if available.
static DbResult query_max_temperature(double latitude, double longitude, int year, int month, int day, const char* grib_file) {
    const char* host = getenv("DB_HOST");
    const char* port = getenv("DB_PORT");
    const char* db   = getenv("DB_NAME");
    const char* user = getenv("DB_USER");
    const char* pass = getenv("DB_PASSWORD");
    DbResult r;
    r.success = false;
    r.max_temperature = 0.0;
    strncpy(r.error, "database_not_configured", sizeof(r.error)-1);
    r.error[sizeof(r.error)-1]='\0';

    if (!(host && port && db && user && pass)) {
        return r;
    }

    // Implement actual MySQL prepared statement here if libmysqlclient is linked.
    // SELECT MAX(temp_c) FROM temperatures WHERE latitude=? AND longitude=? AND year=? AND month=? AND day=? AND grib_file=?

    return r;
}

static void send_json(int cfd, int status, const char* body) {
    char header[256];
    int blen = (int)strlen(body);
    int n = snprintf(header, sizeof(header),
        "HTTP/1.1 %d OK\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: %d\r\nConnection: close\r\n\r\n",
        status, blen);
    if (n < 0) return;
    send(cfd, header, (size_t)n, 0);
    send(cfd, body, (size_t)blen, 0);
}

static void handle_client(int cfd) {
    char buf[8192];
    memset(buf, 0, sizeof(buf));
    ssize_t n = recv(cfd, buf, sizeof(buf)-1, 0);
    if (n <= 0) { close(cfd); return; }
    char* first_line_end = strstr(buf, "\r\n");
    if (!first_line_end) { send_json(cfd, 400, "{\"error\":\"invalid_request\"}"); close(cfd); return; }
    *first_line_end = '\0';
    char method[8]={0}, target[4096]={0}, version[16]={0};
    if (sscanf(buf, "%7s %4095s %15s", method, target, version) != 3) {
        send_json(cfd, 400, "{\"error\":\"invalid_request\"}"); close(cfd); return;
    }
    if (strcmp(method, "GET") != 0) {
        send_json(cfd, 405, "{\"error\":\"method_not_allowed\"}"); close(cfd); return;
    }
    char path[4096]={0};
    char* qm = strchr(target, '?');
    if (qm) {
        size_t plen = (size_t)(qm - target);
        if (plen >= sizeof(path)) plen = sizeof(path)-1;
        memcpy(path, target, plen);
        path[plen]='\0';
    } else {
        strncpy(path, target, sizeof(path)-1);
    }
    if (strcmp(path, ENDPOINT) != 0) {
        send_json(cfd, 404, "{\"error\":\"not_found\"}"); close(cfd); return;
    }
    const char* qstr = qm ? qm+1 : "";
    query_t q; parse_query(qstr, &q);
    const char* required[] = {"latitude","longitude","year","month","day","grib_file"};
    for (int i=0;i<6;i++) {
        if (!qget(&q, required[i])) {
            send_json(cfd, 400, "{\"error\":\"missing_param\"}"); close(cfd); return;
        }
    }
    double lat, lon;
    int year, month, day;
    const char* s_lat = qget(&q,"latitude");
    const char* s_lon = qget(&q,"longitude");
    const char* s_year= qget(&q,"year");
    const char* s_month= qget(&q,"month");
    const char* s_day= qget(&q,"day");
    const char* grib= qget(&q,"grib_file");

    if (!parse_double_in_range(s_lat, -90.0, 90.0, &lat) ||
        !parse_double_in_range(s_lon, -180.0, 180.0, &lon) ||
        !parse_int_in_range(s_year, 1900, 2100, &year) ||
        !parse_int_in_range(s_month, 1, 12, &month) ||
        !parse_int_in_range(s_day, 1, 31, &day) ||
        !grib_valid(grib)) {
        send_json(cfd, 400, "{\"error\":\"invalid_param\"}"); close(cfd); return;
    }

    DbResult r = query_max_temperature(lat, lon, year, month, day, grib);
    if (!r.success) {
        if (strcmp(r.error, "database_not_configured")==0) send_json(cfd, 500, "{\"error\":\"database_not_configured\"}");
        else if (strcmp(r.error, "not_found")==0) send_json(cfd, 404, "{\"max_temperature\": null}");
        else send_json(cfd, 500, "{\"error\":\"internal_error\"}");
        close(cfd); return;
    }

    char body[128];
    int m = snprintf(body, sizeof(body), "{\"max_temperature\": %.6f}", r.max_temperature);
    if (m < 0) { send_json(cfd, 500, "{\"error\":\"internal_error\"}"); close(cfd); return; }
    send_json(cfd, 200, body);
    close(cfd);
}

static void* server_thread(void* arg) {
    (void)arg;
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    int opt=1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(sfd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        perror("bind");
        return NULL;
    }
    if (listen(sfd, 8) != 0) {
        perror("listen");
        close(sfd);
        return NULL;
    }

    for (;;) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(sfd, &rfds);
        struct timeval tv;
        tv.tv_sec = 1; tv.tv_usec = 0;
        int r = select(sfd+1, &rfds, NULL, NULL, &tv);
        if (r > 0 && FD_ISSET(sfd, &rfds)) {
            struct sockaddr_in cli;
            socklen_t cl = sizeof(cli);
            int cfd = accept(sfd, (struct sockaddr*)&cli, &cl);
            if (cfd >= 0) handle_client(cfd);
        }
        // The main thread will exit the process after tests, so loop forever here
    }
    close(sfd);
    return NULL;
}

static char* http_get_local(const char* path_and_query) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return strdup("{\"error\":\"internal_error\"}");
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        close(fd);
        return strdup("{\"error\":\"internal_error\"}");
    }
    char req[1024];
    snprintf(req, sizeof(req), "GET %s HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: close\r\n\r\n", path_and_query);
    send(fd, req, strlen(req), 0);
    char* resp = NULL;
    size_t cap = 0, len = 0;
    char buf[1024];
    ssize_t n;
    while ((n = recv(fd, buf, sizeof(buf), 0)) > 0) {
        if (len + (size_t)n + 1 > cap) {
            size_t newcap = cap ? cap*2 : 4096;
            while (newcap < len + (size_t)n + 1) newcap *= 2;
            char* tmp = (char*)realloc(resp, newcap);
            if (!tmp) { free(resp); close(fd); return strdup("{\"error\":\"internal_error\"}"); }
            resp = tmp; cap = newcap;
        }
        memcpy(resp + len, buf, (size_t)n);
        len += (size_t)n;
        resp[len] = '\0';
    }
    close(fd);
    if (!resp) return strdup("{\"error\":\"internal_error\"}");
    char* body = strstr(resp, "\r\n\r\n");
    if (!body) { free(resp); return strdup("{\"error\":\"internal_error\"}"); }
    body += 4;
    char* out = strdup(body);
    free(resp);
    return out;
}

int main(void) {
    pthread_t th;
    if (pthread_create(&th, NULL, server_thread, NULL) != 0) {
        fprintf(stderr, "Failed to start server\n");
        return 1;
    }
    // Allow server to start
    usleep(300000);

    const char* tests[5] = {
        "/api/temperature_for_location?latitude=40.7128&longitude=-74.0060&year=2023&month=5&day=23&grib_file=nyc_20230523.grb2",
        "/api/temperature_for_location?latitude=123.45&longitude=-74.0060&year=2023&month=5&day=23&grib_file=nyc_20230523.grb2",
        "/api/temperature_for_location?latitude=40.7128&longitude=-74.0060&year=2023&month=5&day=23&grib_file=../../etc/passwd",
        "/api/temperature_for_location?latitude=40.7128&longitude=-74.0060&year=2023&month=5&grib_file=nyc_20230523.grb2",
        "/api/temperature_for_location?latitude=-90&longitude=180&year=1900&month=12&day=31&grib_file=edge_case.grb"
    };
    for (int i=0;i<5;i++) {
        printf("Request: %s\n", strchr(tests[i], '?') ? strchr(tests[i], '?')+1 : tests[i]);
        char* resp = http_get_local(tests[i]);
        if (resp) {
            printf("Response: %s\n", resp);
            free(resp);
        } else {
            printf("Response: {\"error\":\"internal_error\"}\n");
        }
        printf("---\n");
    }

    // Exit process; server thread will be terminated by process exit.
    return 0;
}