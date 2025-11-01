#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define PING_COUNT_FLAG "-n 1"
#define DEVNULL " >NUL 2>&1"
#else
#define PING_COUNT_FLAG "-c 1"
#define DEVNULL " >/dev/null 2>&1"
#endif

/* Simulated decorator macro placeholder (no real decorators in C) */
#define ROUTE(path, func) /* bind route path to func (placeholder) */

const char* run_ping(const char* url) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "ping %s %s%s", PING_COUNT_FLAG, url, DEVNULL);
    system(cmd);
    return url;
}

/* Simulated route-bound function */
ROUTE("/ping", ping)
const char* ping() {
    const char* url = "example.com";
    run_ping(url);
    return url; /* would be returned to a browser in a real HTTP handler */
}

int main(void) {
    /* 5 test cases */
    const char* tests[5] = {"example.com", "localhost", "127.0.0.1", "google.com", "invalid.invalid"};
    for (int i = 0; i < 5; ++i) {
        const char* result = run_ping(tests[i]);
        printf("Pinged and returned URL: %s\n", result);
    }

    /* Demonstrate the simulated route function behavior */
    printf("Route /ping returned URL: %s\n", ping());
    return 0;
}