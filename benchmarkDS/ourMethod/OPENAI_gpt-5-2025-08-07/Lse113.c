#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#if defined(_WIN32)
#include <windows.h>
#define POPEN _popen
#define PCLOSE _pclose
#else
#include <sys/wait.h>
#define POPEN popen
#define PCLOSE pclose
#endif

#define MAX_URL_LEN 2048
#define MAX_HOST_LEN 253
#define MAX_OUTPUT 8192

typedef struct {
    char* url;
    char* pong;
    int alive;
    int exit_code;
} PingResult;

typedef PingResult (*RouteHandler)(const char* url);

typedef struct {
    const char* path;
    RouteHandler handler;
} RouteEntry;

static int is_allowed_host_char(char ch) {
    if ((ch >= 'a' && ch <= 'z') ||
        (ch >= 'A' && ch <= 'Z') ||
        (ch >= '0' && ch <= '9') ||
        ch == '.' || ch == '-' || ch == ':') {
        return 1;
    }
    return 0;
}

static int extract_and_validate_host(const char* url, char* out_host, size_t out_sz) {
    if (!url || !out_host || out_sz == 0) return 0;
    size_t len = strnlen(url, MAX_URL_LEN + 1);
    if (len == 0 || len > MAX_URL_LEN) return 0;

    // trim spaces
    const char* start = url;
    while (*start && isspace((unsigned char)*start)) start++;
    const char* end = url + len;
    while (end > start && isspace((unsigned char)*(end - 1))) end--;
    if (end <= start) return 0;

    char tmp[MAX_URL_LEN + 1];
    size_t tlen = (size_t)(end - start);
    if (tlen > MAX_URL_LEN) return 0;
    memcpy(tmp, start, tlen);
    tmp[tlen] = '\0';

    char host[MAX_URL_LEN + 1];
    host[0] = '\0';

    char* scheme = strstr(tmp, "://");
    if (scheme) {
        char* hstart = scheme + 3;
        char* slash = strchr(hstart, '/');
        size_t hlen = slash ? (size_t)(slash - hstart) : strlen(hstart);
        if (hlen >= sizeof(host)) return 0;
        memcpy(host, hstart, hlen);
        host[hlen] = '\0';
        // strip userinfo
        char* at = strrchr(host, '@');
        if (at) {
            memmove(host, at + 1, strlen(at + 1) + 1);
        }
        if (host[0] != '[') {
            char* colon = strchr(host, ':');
            if (colon) *colon = '\0';
        }
    } else {
        if (tlen >= sizeof(host)) return 0;
        memcpy(host, tmp, tlen + 1);
    }

    // strip brackets
    size_t hlen = strlen(host);
    if (hlen >= 2 && host[0] == '[' && host[hlen - 1] == ']') {
        memmove(host, host + 1, hlen - 2);
        host[hlen - 2] = '\0';
        hlen -= 2;
    }

    if (hlen == 0 || hlen > MAX_HOST_LEN) return 0;

    for (size_t i = 0; i < hlen; i++) {
        char ch = host[i];
        if (!is_allowed_host_char(ch)) return 0;
    }
    if (strpbrk(host, " ;|&$`\\") != NULL) return 0;

    if (strlen(host) + 1 > out_sz) return 0;
    strcpy(out_host, host);
    return 1;
}

PingResult ping(const char* url) {
    PingResult res;
    res.url = NULL;
    res.pong = NULL;
    res.alive = 0;
    res.exit_code = -1;

    if (!url) {
        res.pong = (char*)malloc(16);
        if (res.pong) strcpy(res.pong, "invalid url/host");
        return res;
    }
    size_t ulen = strnlen(url, MAX_URL_LEN + 1);
    res.url = (char*)malloc(ulen + 1);
    if (res.url) memcpy(res.url, url, ulen + 1);

    char host[MAX_HOST_LEN + 1];
    if (!extract_and_validate_host(url, host, sizeof(host))) {
        res.pong = (char*)malloc(18);
        if (res.pong) strcpy(res.pong, "invalid url/host");
        return res;
    }

#if defined(_WIN32)
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "ping -n 1 -w 2000 %s", host);
#else
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "ping -c 1 -W 2 %s", host);
#endif

    FILE* pipe = POPEN(cmd, "r");
    if (!pipe) {
        res.pong = (char*)malloc(20);
        if (res.pong) strcpy(res.pong, "failed to run ping");
        res.exit_code = -1;
        return res;
    }

    res.pong = (char*)malloc(MAX_OUTPUT + 1);
    if (!res.pong) {
        // memory error
        (void)PCLOSE(pipe);
        res.exit_code = -1;
        return res;
    }
    size_t total = 0;
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        size_t blen = strnlen(buffer, sizeof(buffer));
        if (total + blen > MAX_OUTPUT) {
            size_t copy = (MAX_OUTPUT > total) ? (MAX_OUTPUT - total) : 0;
            if (copy > 0) memcpy(res.pong + total, buffer, copy);
            total += copy;
            break;
        } else {
            memcpy(res.pong + total, buffer, blen);
            total += blen;
        }
    }
    res.pong[total] = '\0';

    int status = PCLOSE(pipe);
#if defined(_WIN32)
    res.exit_code = status;
#else
    if (WIFEXITED(status)) {
        res.exit_code = WEXITSTATUS(status);
    } else {
        res.exit_code = -1;
    }
#endif
    res.alive = (res.exit_code == 0) ? 1 : 0;
    return res;
}

PingResult ping_handler(const char* url) {
    return ping(url);
}

static RouteEntry ROUTES[] = {
    { "/ping", ping_handler }
};

static PingResult handle_request(const char* path, const char* url) {
    size_t n = sizeof(ROUTES) / sizeof(ROUTES[0]);
    for (size_t i = 0; i < n; i++) {
        if (strcmp(ROUTES[i].path, path) == 0) {
            return ROUTES[i].handler(url);
        }
    }
    PingResult r;
    r.url = NULL;
    r.pong = (char*)malloc(16);
    if (r.pong) strcpy(r.pong, "route not found");
    r.alive = 0;
    r.exit_code = -1;
    return r;
}

static void free_result(PingResult* r) {
    if (!r) return;
    if (r->url) free(r->url);
    if (r->pong) free(r->pong);
    r->url = NULL;
    r->pong = NULL;
}

int main(void) {
    const char* tests[5] = {
        "127.0.0.1",
        "localhost",
        "https://example.com",
        "invalid host!!",
        "8.8.8.8"
    };
    for (int i = 0; i < 5; i++) {
        PingResult r = handle_request("/ping", tests[i]);
        size_t pong_len = r.pong ? strlen(r.pong) : 0;
        printf("url=%s, alive=%s, exit_code=%d, pong_len=%zu\n",
               tests[i],
               r.alive ? "true" : "false",
               r.exit_code,
               pong_len);
        if (r.pong) {
            char sample[124];
            size_t copy = pong_len > 120 ? 120 : pong_len;
            memcpy(sample, r.pong, copy);
            sample[copy] = '\0';
            if (pong_len > 120) {
                printf("Sample of pong: %s...\n", sample);
            } else {
                printf("Sample of pong: %s\n", sample);
            }
        } else {
            printf("Sample of pong: \n");
        }
        printf("-----\n");
        free_result(&r);
    }
    return 0;
}