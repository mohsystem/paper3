#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* ping(const char* url) {
#ifdef _WIN32
    const char* fmt = "ping -n 1 %s";
#else
    const char* fmt = "ping -c 1 %s";
#endif
    size_t needed = snprintf(NULL, 0, fmt, url) + 1;
    char* cmd = (char*)malloc(needed);
    if (cmd) {
        snprintf(cmd, needed, fmt, url);
        system(cmd);
        free(cmd);
    }
    return url;
}

int main(void) {
    const char* tests[5] = {"localhost", "example.com", "127.0.0.1", "8.8.8.8", "github.com"};
    for (int i = 0; i < 5; ++i) {
        const char* ret = ping(tests[i]);
        printf("Returned URL: %s\n", ret);
    }
    return 0;
}