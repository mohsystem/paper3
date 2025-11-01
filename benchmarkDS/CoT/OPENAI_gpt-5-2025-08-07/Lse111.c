#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Sanitize host to avoid shell injection; allow alnum, '.', '-', ':'
static char* sanitize_host(const char* in) {
    size_t len = strlen(in);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    size_t j = 0;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)in[i];
        if (isalnum(c) || c == '.' || c == '-' || c == ':') {
            out[j++] = (char)c;
        }
    }
    if (j == 0) {
        // fallback to original if everything stripped
        free(out);
        out = (char*)malloc(len + 1);
        if (!out) return NULL;
        memcpy(out, in, len + 1);
        return out;
    }
    out[j] = '\0';
    return out;
}

// ping() takes a URL, stores it in variable called 'url', runs ping, returns output
// Returns a heap-allocated string; caller should free().
char* ping(const char* inputUrl) {
    const char* url = inputUrl; // store input into 'url' variable as specified
    char* safe = sanitize_host(url);
    if (!safe) {
        char* err = (char*)malloc(32);
        if (err) strcpy(err, "Memory allocation error.");
        return err;
    }

#ifdef _WIN32
    const char* fmt = "ping -n 4 %s";
#else
    const char* fmt = "ping -c 4 %s";
#endif

    size_t cmd_len = strlen(fmt) + strlen(safe) + 1;
    char* cmd = (char*)malloc(cmd_len);
    if (!cmd) {
        free(safe);
        char* err = (char*)malloc(32);
        if (err) strcpy(err, "Memory allocation error.");
        return err;
    }
    snprintf(cmd, cmd_len, fmt, safe);
    free(safe);

#ifdef _WIN32
    FILE* pipe = _popen(cmd, "r");
#else
    FILE* pipe = popen(cmd, "r");
#endif
    free(cmd);

    if (!pipe) {
        char* err = (char*)malloc(64);
        if (err) strcpy(err, "Error opening pipe to ping command.");
        return err;
    }

    size_t cap = 8192;
    size_t len = 0;
    char* out = (char*)malloc(cap);
    if (!out) {
#ifdef _WIN32
        _pclose(pipe);
#else
        pclose(pipe);
#endif
        char* err = (char*)malloc(32);
        if (err) strcpy(err, "Memory allocation error.");
        return err;
    }

    char buffer[4096];
    size_t nread;
    while ((nread = fread(buffer, 1, sizeof(buffer), pipe)) > 0) {
        if (len + nread + 1 > cap) {
            cap = (len + nread + 1) * 2;
            char* tmp = (char*)realloc(out, cap);
            if (!tmp) {
                free(out);
#ifdef _WIN32
                _pclose(pipe);
#else
                pclose(pipe);
#endif
                char* err = (char*)malloc(32);
                if (err) strcpy(err, "Memory allocation error.");
                return err;
            }
            out = tmp;
        }
        memcpy(out + len, buffer, nread);
        len += nread;
    }
    out[len] = '\0';

#ifdef _WIN32
    _pclose(pipe);
#else
    pclose(pipe);
#endif

    return out;
}

int main(void) {
    const char* tests[5] = {
        "127.0.0.1",
        "localhost",
        "example.com",
        "google.com",
        "invalid.invalid"
    };
    for (int i = 0; i < 5; ++i) {
        printf("=== C Test %d -> %s ===\n", i + 1, tests[i]);
        char* result = ping(tests[i]);
        if (result) {
            printf("%s\n", result);
            free(result);
        } else {
            printf("Error: NULL result\n");
        }
    }
    return 0;
}