#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#ifndef _WIN32
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#endif

static char* str_duplicate(const char* s) {
    if (!s) return NULL;
    size_t n = strlen(s);
    char* d = (char*)malloc(n + 1);
    if (!d) return NULL;
    memcpy(d, s, n);
    d[n] = '\0';
    return d;
}

static char* trim_copy(const char* s) {
    if (!s) return str_duplicate("");
    size_t len = strlen(s);
    size_t i = 0, j = len;
    while (i < j && isspace((unsigned char)s[i])) i++;
    while (j > i && isspace((unsigned char)s[j-1])) j--;
    size_t n = j > i ? (j - i) : 0;
    char* out = (char*)malloc(n + 1);
    if (!out) return NULL;
    if (n) memcpy(out, s + i, n);
    out[n] = '\0';
    return out;
}

static char* extract_host(const char* inputUrl) {
    if (!inputUrl) return NULL;
    if (strlen(inputUrl) > 2048) return NULL;
    char* s = trim_copy(inputUrl);
    if (!s) return NULL;
    if (s[0] == '\0') { free(s); return NULL; }

    size_t start = 0;
    char* scheme = strstr(s, "://");
    if (scheme) {
        start = (size_t)(scheme - s) + 3;
    }
    size_t end = start;
    while (s[end] && s[end] != '/' && s[end] != '?' && s[end] != '#') {
        end++;
    }
    if (end <= start) { free(s); return NULL; }

    size_t hlen = end - start;
    char* host = (char*)malloc(hlen + 1);
    if (!host) { free(s); return NULL; }
    memcpy(host, s + start, hlen);
    host[hlen] = '\0';

    // Strip brackets for IPv6
    if (hlen >= 2 && host[0] == '[' && host[hlen - 1] == ']') {
        memmove(host, host + 1, hlen - 2);
        host[hlen - 2] = '\0';
    }
    free(s);
    return host;
}

static int is_valid_host(const char* host) {
    if (!host) return 0;
    size_t len = strlen(host);
    if (len == 0 || len > 253) return 0;
    if (host[0] == '.' || host[len-1] == '.' || host[0] == '-' || host[len-1] == '-') return 0;
    if (strstr(host, "..") != NULL) return 0;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)host[i];
        if (!(isalnum(c) || c == '.' || c == '-' || c == ':')) return 0;
    }

#ifndef _WIN32
    // IPv6?
    if (strchr(host, ':') != NULL) {
        struct in6_addr a6;
        if (inet_pton(AF_INET6, host, &a6) == 1) return 1;
        return 0;
    }
    // IPv4?
    struct in_addr a4;
    if (inet_pton(AF_INET, host, &a4) == 1) return 1;
#endif

    // hostname labels
    size_t i = 0;
    while (i < len) {
        size_t j = i;
        while (j < len && host[j] != '.') j++;
        size_t lablen = j - i;
        if (lablen == 0 || lablen > 63) return 0;
        if (!isalnum((unsigned char)host[i]) || !isalnum((unsigned char)host[j-1])) return 0;
        for (size_t k = i; k < j; ++k) {
            unsigned char c = (unsigned char)host[k];
            if (!(isalnum(c) || c == '-')) return 0;
        }
        if (j == len) break;
        i = j + 1;
    }
    return 1;
}

#ifndef _WIN32
static char* run_ping_posix(const char* host) {
    int pipefd[2];
    if (pipe(pipefd) != 0) {
        return str_duplicate("Ping failed");
    }
    pid_t pid = fork();
    if (pid < 0) {
        close(pipefd[0]); close(pipefd[1]);
        return str_duplicate("Ping failed");
    } else if (pid == 0) {
        // Child
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        for (int fd = 3; fd < 256; ++fd) {
            if (fd != pipefd[1]) close(fd);
        }
        execlp("ping", "ping", "-n", "-c", "1", "-W", "1", host, (char*)NULL);
        const char* msg = "Ping failed to exec\n";
        write(pipefd[1], msg, strlen(msg));
        _exit(127);
    } else {
        // Parent
        close(pipefd[1]);
        const size_t MAX_OUT = 65536;
        char* buf = (char*)malloc(MAX_OUT + 1);
        if (!buf) {
            close(pipefd[0]);
            waitpid(pid, NULL, 0);
            return str_duplicate("Ping failed");
        }
        size_t total = 0;
        ssize_t r = 0;
        while ((r = read(pipefd[0], buf + total, (MAX_OUT - total))) > 0) {
            total += (size_t)r;
            if (total >= MAX_OUT) break;
        }
        close(pipefd[0]);
        waitpid(pid, NULL, 0);
        buf[total] = '\0';
        // Allocate exact-size string
        char* out = (char*)malloc(total + 1);
        if (!out) {
            free(buf);
            return str_duplicate("Ping failed");
        }
        memcpy(out, buf, total + 1);
        free(buf);
        return out;
    }
}
#endif

char* ping(const char* inputUrl) {
    char* host = extract_host(inputUrl);
    if (!is_valid_host(host)) {
        if (host) free(host);
        return str_duplicate("Invalid host");
    }
#ifdef _WIN32
    (void)host;
    return str_duplicate("Ping not implemented for Windows in this demo");
#else
    char* out = run_ping_posix(host);
    free(host);
    return out;
#endif
}

int main(void) {
    const char* tests[5] = {
        "http://127.0.0.1",
        "https://localhost",
        "https://example.com",
        "http://[::1]/",
        "http://bad_host_$$$"
    };
    for (int i = 0; i < 5; ++i) {
        printf("=== Test: %s ===\n", tests[i]);
        char* out = ping(tests[i]);
        if (out) {
            if (strlen(out) > 500) {
                fwrite(out, 1, 500, stdout);
                printf("...(truncated)\n\n");
            } else {
                printf("%s\n", out);
            }
            free(out);
        } else {
            printf("Ping failed\n\n");
        }
    }
    return 0;
}