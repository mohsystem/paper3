#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <mysql/mysql.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

static bool is_valid_grib_file(const char* s) {
    if (!s) return false;
    size_t len = strlen(s);
    if (len == 0 || len > 128) return false;
    for (size_t i = 0; i < len; ++i) {
        char c = s[i];
        if (!(isalnum((unsigned char)c) || c == '.' || c == '_' || c == '-')) return false;
    }
    return true;
}

static bool parse_double(const char* s, double* out) {
    if (!s || !*s) return false;
    char* end = NULL;
    errno = 0;
    double v = strtod(s, &end);
    if (errno != 0 || end == s || *end != '\0' || isnan(v) || isinf(v)) return false;
    *out = v;
    return true;
}

static bool parse_int(const char* s, int* out) {
    if (!s || !*s) return false;
    char* end = NULL;
    errno = 0;
    long v = strtol(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0' || v < -2147483648L || v > 2147483647L) return false;
    *out = (int)v;
    return true;
}

typedef struct {
    char* key;
    char* val;
} kvp;

typedef struct {
    kvp* items;
    size_t count;
} query_map;

static void query_map_free(query_map* m) {
    if (!m) return;
    for (size_t i = 0; i < m->count; ++i) {
        free(m->items[i].key);
        free(m->items[i].val);
    }
    free(m->items);
    m->items = NULL;
    m->count = 0;
}

static const char* query_map_get(query_map* m, const char* key) {
    for (size_t i = 0; i < m->count; ++i) {
        if (strcmp(m->items[i].key, key) == 0) return m->items[i].val;
    }
    return NULL;
}

static query_map parse_query(const char* q) {
    query_map m = {0};
    if (!q) return m;
    char* qs = strdup(q);
    char* token = strtok(qs, "&");
    while (token) {
        char* eq = strchr(token, '=');
        char* k = NULL; char* v = NULL;
        if (eq) {
            *eq = '\0';
            k = strdup(token);
            v = strdup(eq + 1);
        } else {
            k = strdup(token);
            v = strdup("");
        }
        m.items = (kvp*)realloc(m.items, sizeof(kvp) * (m.count + 1));
        m.items[m.count].key = k;
        m.items[m.count].val = v;
        m.count++;
        token = strtok(NULL, "&");
    }
    free(qs);
    return m;
}

static void json_escape(const char* in, char* out, size_t outsz) {
    size_t j = 0;
    for (size_t i = 0; in[i] && j + 2 < outsz; ++i) {
        char c = in[i];
        if (c == '"' || c == '\\') { if (j + 2 >= outsz) break; out[j++]='\\'; out[j++]=c; }
        else if (c == '\n') { if (j + 2 >= outsz) break; out[j++]='\\'; out[j++]='n'; }
        else { out[j++] = c; }
    }
    out[j] = '\0';
}

static bool mysql_query_max_temp(double latitude, double longitude, int year, int month, int day, const char* grib_file, double* out, char* errbuf, size_t errsz) {
    if (!is_valid_grib_file(grib_file)) { snprintf(errbuf, errsz, "invalid grib_file"); return false; }
    const char* host = getenv("DB_HOST"); if (!host) host = "127.0.0.1";
    const char* user = getenv("DB_USER"); if (!user) user = "root";
    const char* pass = getenv("DB_PASS"); if (!pass) pass = "";
    const char* db   = getenv("DB_NAME"); if (!db) db   = "weather";
    unsigned int port = 3306;
    if (const char* p = getenv("DB_PORT")) port = (unsigned int)strtoul(p, NULL, 10);

    MYSQL* conn = mysql_init(NULL);
    if (!conn) { snprintf(errbuf, errsz, "mysql_init failed"); return false; }
    unsigned int timeout = 10;
    mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
    mysql_options(conn, MYSQL_OPT_READ_TIMEOUT, &timeout);
    mysql_options(conn, MYSQL_OPT_WRITE_TIMEOUT, &timeout);

    if (!mysql_real_connect(conn, host, user, pass, db, port, NULL, 0)) {
        snprintf(errbuf, errsz, "%s", mysql_error(conn));
        mysql_close(conn);
        return false;
    }
    char query[1024];
    snprintf(query, sizeof(query),
             "SELECT MAX(temperature) AS max_temp FROM temperatures "
             "WHERE latitude=%.10f AND longitude=%.10f AND year=%d AND month=%d AND day=%d AND grib_file='%s'",
             latitude, longitude, year, month, day, grib_file);

    if (mysql_query(conn, query) != 0) {
        snprintf(errbuf, errsz, "%s", mysql_error(conn));
        mysql_close(conn);
        return false;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) {
        snprintf(errbuf, errsz, "%s", mysql_error(conn));
        mysql_close(conn);
        return false;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    bool ok = true;
    if (!row || !row[0]) {
        ok = false; // NULL result
        if (errbuf && errsz) errbuf[0] = '\0';
    } else {
        *out = atof(row[0]);
    }
    mysql_free_result(res);
    mysql_close(conn);
    return ok;
}

static void build_response_for_request(const char* query, char* out, size_t outsz) {
    query_map qm = parse_query(query);
    const char* slat = query_map_get(&qm, "latitude");
    const char* slon = query_map_get(&qm, "longitude");
    const char* syear = query_map_get(&qm, "year");
    const char* smonth = query_map_get(&qm, "month");
    const char* sday = query_map_get(&qm, "day");
    const char* sgrib = query_map_get(&qm, "grib_file");

    double lat, lon; int year, month, day;
    if (!slat || !parse_double(slat, &lat) || !(lat >= -90.0 && lat <= 90.0)) { snprintf(out, outsz, "{\"error\":\"invalid latitude\"}"); query_map_free(&qm); return; }
    if (!slon || !parse_double(slon, &lon) || !(lon >= -180.0 && lon <= 180.0)) { snprintf(out, outsz, "{\"error\":\"invalid longitude\"}"); query_map_free(&qm); return; }
    if (!syear || !parse_int(syear, &year) || !(year >= 1900 && year <= 2100)) { snprintf(out, outsz, "{\"error\":\"invalid year\"}"); query_map_free(&qm); return; }
    if (!smonth || !parse_int(smonth, &month) || !(month >= 1 && month <= 12)) { snprintf(out, outsz, "{\"error\":\"invalid month\"}"); query_map_free(&qm); return; }
    if (!sday || !parse_int(sday, &day) || !(day >= 1 && day <= 31)) { snprintf(out, outsz, "{\"error\":\"invalid day\"}"); query_map_free(&qm); return; }
    if (!sgrib || !is_valid_grib_file(sgrib)) { snprintf(out, outsz, "{\"error\":\"invalid grib_file\"}"); query_map_free(&qm); return; }

    double maxTemp = 0.0;
    char err[256] = {0};
    bool has = mysql_query_max_temp(lat, lon, year, month, day, sgrib, &maxTemp, err, sizeof(err));
    if (err[0]) {
        char esc[512]; json_escape(err, esc, sizeof(esc));
        snprintf(out, outsz, "{\"error\":\"%s\"}", esc);
    } else if (!has) {
        snprintf(out, outsz, "{\"max_temperature\":null}");
    } else {
        snprintf(out, outsz, "{\"max_temperature\":%.6f}", maxTemp);
    }
    query_map_free(&qm);
}

static void* http_server(void* arg) {
    (void)arg;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) pthread_exit(NULL);
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8083);
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) { close(fd); pthread_exit(NULL); }
    if (listen(fd, 5) != 0) { close(fd); pthread_exit(NULL); }

    while (1) {
        int cfd = accept(fd, NULL, NULL);
        if (cfd < 0) continue;
        char buf[4096];
        ssize_t n = recv(cfd, buf, sizeof(buf) - 1, 0);
        if (n <= 0) { close(cfd); continue; }
        buf[n] = '\0';
        char method[8] = {0}, path[2048] = {0}, proto[16] = {0};
        sscanf(buf, "%7s %2047s %15s", method, path, proto);

        int status = 200;
        char payload[1024];
        payload[0] = '\0';

        if (strcmp(method, "GET") != 0) {
            snprintf(payload, sizeof(payload), "{\"error\":\"Method Not Allowed\"}");
            status = 405;
        } else {
            char* q = strchr(path, '?');
            char* query = NULL;
            if (q) { *q = '\0'; query = q + 1; }
            if (strcmp(path, "/api/temperature_for_location") != 0) {
                snprintf(payload, sizeof(payload), "{\"error\":\"Not Found\"}");
                status = 404;
            } else {
                build_response_for_request(query ? query : "", payload, sizeof(payload));
            }
        }
        char header[512];
        int len = snprintf(header, sizeof(header),
                           "HTTP/1.1 %d OK\r\n"
                           "Content-Type: application/json; charset=utf-8\r\n"
                           "Content-Length: %zu\r\n"
                           "Connection: close\r\n\r\n",
                           status, strlen(payload));
        send(cfd, header, len, 0);
        send(cfd, payload, strlen(payload), 0);
        close(cfd);
    }
    close(fd);
    return NULL;
}

int main(void) {
    pthread_t tid;
    pthread_create(&tid, NULL, http_server, NULL);
    pthread_detach(tid);

    // 5 test cases invoking core logic
    const char* tests[5] = {
        "latitude=40.7128&longitude=-74.0060&year=2023&month=7&day=15&grib_file=sample1.grb",
        "latitude=34.0522&longitude=-118.2437&year=2022&month=12&day=1&grib_file=sample2.grb",
        "latitude=-33.8688&longitude=151.2093&year=2021&month=1&day=31&grib_file=sample3.grb",
        "latitude=51.5074&longitude=-0.1278&year=2020&month=6&day=21&grib_file=sample4.grb",
        "latitude=35.6895&longitude=139.6917&year=2019&month=3&day=10&grib_file=sample5.grb"
    };
    for (int i = 0; i < 5; ++i) {
        char out[1024];
        build_response_for_request(tests[i], out, sizeof(out));
        printf("%s\n", out);
    }
    return 0;
}