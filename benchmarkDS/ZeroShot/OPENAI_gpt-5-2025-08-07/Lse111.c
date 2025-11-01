#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifndef _WIN32
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <signal.h>
#endif

static int is_allowed_chars(const char* s) {
    if (!s || !*s) return 0;
    size_t len = strlen(s);
    if (len > 253) return 0;
    for (size_t i = 0; i < len; i++) {
        char c = s[i];
        if (!(isalnum((unsigned char)c) || c=='.' || c=='-' || c==':')) return 0;
    }
    return 1;
}

static int is_ipv4(const char* s) {
    int a,b,c,d;
    char tail;
    if (sscanf(s, "%d.%d.%d.%d%c", &a, &b, &c, &d, &tail) == 4) {
        if (a>=0 && a<=255 && b>=0 && b<=255 && c>=0 && c<=255 && d>=0 && d<=255) return 1;
    }
    return 0;
}

static int is_hostname(const char* s) {
    size_t len = strlen(s);
    if (len == 0 || len > 253) return 0;
    const char* p = s;
    int label_len = 0;
    int has_char = 0;
    while (*p) {
        char c = *p;
        if (c == '.') {
            if (label_len == 0 || label_len > 63) return 0;
            label_len = 0;
            p++;
            continue;
        }
        if (!(isalnum((unsigned char)c) || c=='-')) return 0;
        if ((label_len == 0 && c=='-')) return 0;
        label_len++;
        has_char = 1;
        p++;
    }
    if (label_len == 0 || label_len > 63) return 0;
    return has_char;
}

static int is_ipv6_like(const char* s) {
    // loose check: segments up to 8, hex or empty (for ::)
    int parts = 0;
    const char* p = s;
    while (*p) {
        const char* start = p;
        while (*p && *p != ':') {
            if (!isxdigit((unsigned char)*p)) return 0;
            if ((p - start) >= 4) return 0;
            p++;
        }
        parts++;
        if (*p == ':') p++;
        if (parts > 8) return 0;
    }
    return parts >= 2 && parts <= 8;
}

static int is_valid_host(const char* host) {
    if (!host) return 0;
    while (isspace((unsigned char)*host)) host++;
    if (!*host) return 0;
    if (!is_allowed_chars(host)) return 0;
    if (is_ipv4(host)) return 1;
    if (strchr(host, ':') != NULL) {
        return is_ipv6_like(host);
    }
    return is_hostname(host);
}

char* ping(const char* url) {
    if (!url) {
        char* r = strdup("Invalid host");
        return r;
    }
    // trim
    while (isspace((unsigned char)*url)) url++;
    size_t len = strlen(url);
    while (len > 0 && isspace((unsigned char)url[len-1])) len--;
    char* host = (char*)malloc(len + 1);
    if (!host) {
        return strdup("Memory error");
    }
    memcpy(host, url, len);
    host[len] = '\0';

    if (!is_valid_host(host)) {
        free(host);
        return strdup("Invalid host");
    }

#ifdef _WIN32
    free(host);
    return strdup("Ping not supported in this C demo on Windows");
#else
    // Build argv
    const char* argv[7];
    argv[0] = "ping";
    argv[1] = "-c";
    argv[2] = "2";
    argv[3] = "-W";
    argv[4] = "2";
    argv[5] = host;
    argv[6] = NULL;

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        free(host);
        return strdup("Failed to create pipe");
    }

    pid_t pid = fork();
    if (pid < 0) {
        close(pipefd[0]); close(pipefd[1]);
        free(host);
        return strdup("Failed to fork");
    } else if (pid == 0) {
        // Child
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        execvp("ping", (char* const*)argv);
        const char* msg = "exec failed\n";
        write(STDOUT_FILENO, msg, strlen(msg));
        _exit(127);
    }

    // Parent
    close(pipefd[1]);
    size_t cap = 4096;
    size_t used = 0;
    char* out = (char*)malloc(cap);
    if (!out) {
        close(pipefd[0]);
        waitpid(pid, NULL, 0);
        free(host);
        return strdup("Memory error");
    }

    int timedOut = 0;
    const int TIMEOUT_SEC = 8;
    while (1) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(pipefd[0], &rfds);
        struct timeval tv;
        tv.tv_sec = TIMEOUT_SEC;
        tv.tv_usec = 0;
        int sel = select(pipefd[0] + 1, &rfds, NULL, NULL, &tv);
        if (sel > 0 && FD_ISSET(pipefd[0], &rfds)) {
            char buf[2048];
            ssize_t n = read(pipefd[0], buf, sizeof(buf));
            if (n > 0) {
                if (used + (size_t)n + 1 > cap) {
                    size_t newcap = cap * 2;
                    if (newcap < used + (size_t)n + 1) newcap = used + (size_t)n + 1;
                    if (newcap > 100000) newcap = 100000;
                    char* tmp = (char*)realloc(out, newcap);
                    if (!tmp) {
                        // stop reading on memory error
                        break;
                    }
                    out = tmp;
                    cap = newcap;
                }
                size_t tocopy = (size_t)n;
                if (used + tocopy > 100000) tocopy = 100000 - used;
                memcpy(out + used, buf, tocopy);
                used += tocopy;
                if (used >= 100000) {
                    break;
                }
            } else {
                // EOF or error
                break;
            }
        } else if (sel == 0) {
            timedOut = 1;
            kill(pid, SIGKILL);
            break;
        } else {
            // error
            break;
        }
    }

    close(pipefd[0]);
    waitpid(pid, NULL, 0);

    if (used + 1 > cap) {
        char* tmp = (char*)realloc(out, used + 1);
        if (tmp) { out = tmp; cap = used + 1; }
    }
    out[used] = '\0';
    if (timedOut) {
        const char* msg = "\n[Timed out]";
        size_t mlen = strlen(msg);
        char* tmp = (char*)realloc(out, used + mlen + 1);
        if (tmp) {
            out = tmp;
            memcpy(out + used, msg, mlen);
            used += mlen;
            out[used] = '\0';
        }
    }
    free(host);
    return out;
#endif
}

int main(void) {
    const char* tests[5] = {"localhost", "127.0.0.1", "::1", "example.com", "invalid_host!"};
    for (int i = 0; i < 5; i++) {
        char* res = ping(tests[i]);
        printf("== ping(%s) ==\n", tests[i]);
        if (res) {
            printf("%s\n", res);
            free(res);
        } else {
            printf("(null)\n");
        }
    }
    return 0;
}