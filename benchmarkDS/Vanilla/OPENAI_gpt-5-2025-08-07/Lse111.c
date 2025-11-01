#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* ping(const char* input) {
    const char* url = input; // store input in variable called url
    char cmd[512];
#ifdef _WIN32
    snprintf(cmd, sizeof(cmd), "ping -n 1 -w 3000 \"%s\" 2>&1", url);
#else
    snprintf(cmd, sizeof(cmd), "ping -c 1 -W 3 \"%s\" 2>&1", url);
#endif
#ifdef _WIN32
    FILE* fp = _popen(cmd, "r");
#else
    FILE* fp = popen(cmd, "r");
#endif
    if (!fp) {
        char* err = (char*)malloc(64);
        if (err) strcpy(err, "Failed to run ping command.");
        return err;
    }
    size_t cap = 1024;
    size_t len = 0;
    char* output = (char*)malloc(cap);
    if (!output) {
#ifdef _WIN32
        _pclose(fp);
#else
        pclose(fp);
#endif
        return NULL;
    }
    int ch;
    while ((ch = fgetc(fp)) != EOF) {
        if (len + 1 >= cap) {
            cap *= 2;
            char* tmp = (char*)realloc(output, cap);
            if (!tmp) {
                free(output);
#ifdef _WIN32
                _pclose(fp);
#else
                pclose(fp);
#endif
                return NULL;
            }
            output = tmp;
        }
        output[len++] = (char)ch;
    }
    output[len] = '\0';
#ifdef _WIN32
    _pclose(fp);
#else
    pclose(fp);
#endif
    return output;
}

int main(void) {
    const char* tests[5] = {
        "localhost",
        "127.0.0.1",
        "8.8.8.8",
        "example.com",
        "invalid.host"
    };
    for (int i = 0; i < 5; ++i) {
        printf("=== Pinging: %s ===\n", tests[i]);
        char* out = ping(tests[i]);
        if (out) {
            printf("%s\n", out);
            free(out);
        } else {
            printf("Error: Unable to capture output.\n");
        }
    }
    return 0;
}