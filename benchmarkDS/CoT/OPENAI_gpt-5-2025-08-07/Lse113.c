#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#define popen _popen
#define pclose _pclose
#else
#include <sys/wait.h>
#endif

typedef struct {
    int alive;
    char* pong;
} PingResult;

static char* strdup_safe(const char* s) {
    if (!s) return NULL;
    size_t n = strlen(s);
    char* out = (char*)malloc(n + 1);
    if (!out) return NULL;
    memcpy(out, s, n + 1);
    return out;
}

static PingResult run_ping(const char* url) {
    PingResult res;
    res.alive = 0;
    res.pong = NULL;

    char cmd[1024];
#ifdef _WIN32
    snprintf(cmd, sizeof(cmd), "ping -n 1 %s", url);
    FILE* pipe = popen(cmd, "r");
#else
    snprintf(cmd, sizeof(cmd), "ping -c 1 %s", url);
    FILE* pipe = popen(cmd, "r");
#endif
    if (!pipe) {
        res.pong = strdup_safe("Failed to start ping");
        res.alive = 0;
        return res;
    }

    size_t cap = 4096;
    size_t len = 0;
    char* buf = (char*)malloc(cap);
    if (!buf) {
        pclose(pipe);
        res.pong = strdup_safe("Out of memory");
        res.alive = 0;
        return res;
    }
    buf[0] = '\0';

    char line[1024];
    while (fgets(line, sizeof(line), pipe) != NULL) {
        size_t l = strlen(line);
        if (len + l + 1 > cap) {
            cap = (len + l + 1) * 2;
            char* nb = (char*)realloc(buf, cap);
            if (!nb) {
                free(buf);
                pclose(pipe);
                res.pong = strdup_safe("Out of memory while reading");
                res.alive = 0;
                return res;
            }
            buf = nb;
        }
        memcpy(buf + len, line, l);
        len += l;
        buf[len] = '\0';
    }

    int code = pclose(pipe);
#ifdef _WIN32
    int exit_code = code;
#else
    int exit_code = WIFEXITED(code) ? WEXITSTATUS(code) : -1;
#endif
    res.alive = (exit_code == 0);
    res.pong = buf;
    return res;
}

PingResult ping(const char* url) {
    return run_ping(url);
}

typedef PingResult (*route_fn)(const char*);

typedef struct {
    const char* pattern;
    route_fn fn;
} Route;

static Route routes[1];

static void init_routes() {
    routes[0].pattern = "<url> ping";
    routes[0].fn = ping;
}

PingResult handle_request(const char* requestPath) {
    init_routes();
    const char* pattern = routes[0].pattern;
    size_t n = strlen(requestPath);
    const char* suffix = " ping";
    size_t s = strlen(suffix);
    if (strcmp(pattern, "<url> ping") == 0 && n >= s && strcmp(requestPath + n - s, suffix) == 0) {
        size_t url_len = n - s;
        while (url_len > 0 && (requestPath[url_len - 1] == ' ' || requestPath[url_len - 1] == '\t')) {
            url_len--;
        }
        char* url = (char*)malloc(url_len + 1);
        if (!url) {
            PingResult r = {0, strdup_safe("Out of memory for url")};
            return r;
        }
        memcpy(url, requestPath, url_len);
        url[url_len] = '\0';
        PingResult r = routes[0].fn(url);
        free(url);
        return r;
    }
    PingResult r = {0, strdup_safe("No route matched")};
    return r;
}

int main() {
    const char* tests[5] = {
        "localhost ping",
        "127.0.0.1 ping",
        "google.com ping",
        "example.com ping",
        "no.such.domain.invalid ping"
    };
    for (int i = 0; i < 5; ++i) {
        PingResult r = handle_request(tests[i]);
        printf("%s -> alive=%s, pong_len=%zu\n", tests[i], r.alive ? "true" : "false", r.pong ? strlen(r.pong) : 0u);
        free(r.pong);
    }
    return 0;
}