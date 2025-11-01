#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int alive;
    char* pong;
} PingResponse;

PingResponse ping(const char* url) {
#ifdef _WIN32
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "ping -n 1 %s", url);
    FILE* pipe = _popen(cmd, "r");
#else
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "ping -c 1 %s", url);
    FILE* pipe = popen(cmd, "r");
#endif
    PingResponse resp;
    resp.alive = 0;
    resp.pong = NULL;

    if (!pipe) {
        resp.pong = (char*)malloc(64);
        if (resp.pong) strcpy(resp.pong, "Error opening pipe for ping command");
        return resp;
    }

    size_t cap = 8192;
    size_t len = 0;
    resp.pong = (char*)malloc(cap);
    if (!resp.pong) {
#ifdef _WIN32
        _pclose(pipe);
#else
        pclose(pipe);
#endif
        return resp;
    }
    resp.pong[0] = '\0';

    char buf[4096];
    while (fgets(buf, sizeof(buf), pipe) != NULL) {
        size_t blen = strlen(buf);
        if (len + blen + 1 > cap) {
            cap = (len + blen + 1) * 2;
            char* tmp = (char*)realloc(resp.pong, cap);
            if (!tmp) {
#ifdef _WIN32
                _pclose(pipe);
#else
                pclose(pipe);
#endif
                free(resp.pong);
                resp.pong = NULL;
                return resp;
            }
            resp.pong = tmp;
        }
        memcpy(resp.pong + len, buf, blen);
        len += blen;
        resp.pong[len] = '\0';
    }

#ifdef _WIN32
    int status = _pclose(pipe);
    resp.alive = (status == 0) ? 1 : 0;
#else
    int status = pclose(pipe);
    if (WIFEXITED(status)) {
        resp.alive = (WEXITSTATUS(status) == 0) ? 1 : 0;
    } else {
        resp.alive = 0;
    }
#endif
    return resp;
}

int main() {
    const char* tests[5] = {
        "localhost",
        "127.0.0.1",
        "example.com",
        "google.com",
        "invalid.invalid"
    };
    for (int i = 0; i < 5; ++i) {
        PingResponse r = ping(tests[i]);
        printf("URL: %s\n", tests[i]);
        printf("Alive: %s\n", r.alive ? "true" : "false");
        printf("Pong output length: %zu\n", r.pong ? strlen(r.pong) : 0u);
        printf("----\n");
        free(r.pong);
    }
    return 0;
}