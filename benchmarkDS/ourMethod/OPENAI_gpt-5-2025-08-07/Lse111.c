/*
Secure C implementation of a simple "ping" handler:
- Extracts a "url" query parameter from a provided request URL string.
- Validates and sanitizes the target host.
- Executes the system's "ping" command without invoking a shell (no injection).
- Captures and returns the ping output.
- Includes a main() with 5 test cases.

Build: cc -Wall -Wextra -Werror -O2 ping_handler.c -o ping_handler
*/

#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif

#define MAX_INPUT_LEN 2048
#define MAX_HOST_LEN 255
#define MAX_OUTPUT_CAPTURE (64 * 1024)  /* 64 KiB */
#define PING_TIMEOUT_SEC 5              /* Overall timeout for ping process */

/* Utility: safe multiply (size_t) with overflow check */
static bool safe_mul_size(size_t a, size_t b, size_t *out) {
    if (a == 0 || b == 0) {
        *out = 0;
        return true;
    }
    if (a > SIZE_MAX / b) return false;
    *out = a * b;
    return true;
}

/* Percent-decode a URL-encoded string. Rejects if the decoded result exceeds max_out_len. */
static char *url_decode_alloc(const char *s, size_t max_out_len) {
    if (s == NULL) return NULL;
    size_t in_len = strnlen(s, MAX_INPUT_LEN + 1);
    if (in_len == 0 || in_len > MAX_INPUT_LEN) {
        return NULL;
    }

    /* Worst-case decoded length is <= in_len */
    size_t out_cap = in_len + 1;
    if (out_cap > max_out_len + 1) out_cap = max_out_len + 1;
    char *out = (char *)malloc(out_cap);
    if (!out) return NULL;

    size_t oi = 0;
    for (size_t i = 0; i < in_len; ++i) {
        if (oi + 1 >= out_cap) {
            /* overflow risk or no capacity */
            free(out);
            return NULL;
        }
        if (s[i] == '%' && i + 2 < in_len && isxdigit((unsigned char)s[i + 1]) && isxdigit((unsigned char)s[i + 2])) {
            char hi = s[i + 1];
            char lo = s[i + 2];
            int hv = isdigit((unsigned char)hi) ? (hi - '0') : (10 + (tolower((unsigned char)hi) - 'a'));
            int lv = isdigit((unsigned char)lo) ? (lo - '0') : (10 + (tolower((unsigned char)lo) - 'a'));
            unsigned char ch = (unsigned char)((hv << 4) | lv);
            out[oi++] = (char)ch;
            i += 2;
        } else {
            out[oi++] = s[i];
        }
    }
    out[oi] = '\0';
    return out;
}

/* Extract query parameter "url" from a request URL string. Returns malloc'd decoded value or NULL.
   If "?url=" is not present, treat the entire input as the host (decoded copy), for flexibility in tests. */
static char *extract_target_from_request(const char *request_url) {
    if (!request_url) return NULL;

    /* First, bound-copy the input into a temporary buffer to avoid scanning unbounded strings */
    size_t len = strnlen(request_url, MAX_INPUT_LEN + 1);
    if (len == 0 || len > MAX_INPUT_LEN) {
        return NULL;
    }

    const char *q = strchr(request_url, '?');
    if (!q) {
        /* No query part; treat entire string as already "host-like" input */
        return url_decode_alloc(request_url, MAX_HOST_LEN);
    }

    /* Scan query params for "url=" key (case sensitive) */
    const char *params = q + 1;
    while (*params != '\0') {
        const char *amp = strchr(params, '&');
        size_t pair_len = amp ? (size_t)(amp - params) : strnlen(params, MAX_INPUT_LEN);

        if (pair_len == 0 || pair_len > MAX_INPUT_LEN) return NULL;

        /* Check if this pair is "url=" prefix */
        const char key[] = "url=";
        size_t key_len = sizeof(key) - 1;
        if (pair_len >= key_len && strncmp(params, key, key_len) == 0) {
            const char *val = params + key_len;
            size_t val_len = pair_len - key_len;

            /* Bound the substring to decode */
            if (val_len > MAX_INPUT_LEN) return NULL;

            char *val_buf = (char *)malloc(val_len + 1);
            if (!val_buf) return NULL;
            memcpy(val_buf, val, val_len);
            val_buf[val_len] = '\0';

            char *decoded = url_decode_alloc(val_buf, MAX_HOST_LEN);
            free(val_buf);
            return decoded; /* may be NULL if decode failed */
        }

        if (!amp) break;
        params = amp + 1;
    }
    /* "url" not found; fallback to decoding whole input (for robustness) */
    return url_decode_alloc(request_url, MAX_HOST_LEN);
}

/* Trim IPv6 brackets, and strip :port if present (host:port), but preserve IPv6 addresses with multiple colons. */
static void normalize_host_inplace(char *host) {
    if (!host) return;
    size_t n = strnlen(host, MAX_HOST_LEN + 1);
    if (n == 0 || n > MAX_HOST_LEN) return;

    /* Remove leading/trailing whitespace (reject spaces elsewhere by validation routine; this is just normalization) */
    size_t start = 0;
    while (start < n && isspace((unsigned char)host[start])) start++;
    size_t end = n;
    while (end > start && isspace((unsigned char)host[end - 1])) end--;
    if (start > 0 || end < n) {
        memmove(host, host + start, end - start);
        host[end - start] = '\0';
        n = end - start;
    }

    if (n >= 2 && host[0] == '[') {
        char *rb = strchr(host, ']');
        if (rb && rb > host) {
            size_t inner_len = (size_t)(rb - (host + 1));
            memmove(host, host + 1, inner_len);
            host[inner_len] = '\0';
            n = inner_len;
        }
    }

    /* Count colons */
    size_t colon_count = 0;
    for (size_t i = 0; i < n; ++i) if (host[i] == ':') colon_count++;

    if (colon_count == 1) {
        /* Potential host:port; strip port if digits after colon */
        char *c = strrchr(host, ':');
        if (c && *(c + 1) != '\0') {
            bool digits_only = true;
            for (char *p = c + 1; *p; ++p) {
                if (!isdigit((unsigned char)*p)) {
                    digits_only = false;
                    break;
                }
            }
            if (digits_only) {
                *c = '\0';
            }
        }
    }
}

/* Validate host: length and allowed characters (alnum, '.', '-', ':'). No spaces or control chars. */
static bool validate_host_basic(const char *host) {
    if (!host) return false;
    size_t n = strnlen(host, MAX_HOST_LEN + 1);
    if (n == 0 || n > MAX_HOST_LEN) return false;

    for (size_t i = 0; i < n; ++i) {
        unsigned char c = (unsigned char)host[i];
        if (isalnum(c) || c == '.' || c == '-' || c == ':' ) {
            continue;
        }
        /* For safety, reject everything else, including spaces, slashes, etc. */
        return false;
    }
    return true;
}

/* Resolve host to determine address family (AF_INET or AF_INET6). Returns AF_INET/AF_INET6 or -1 on error. */
static int resolve_family(const char *host) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;      /* accept IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM;   /* arbitrary, to prefer usable records */
#ifdef AI_ADDRCONFIG
    hints.ai_flags = AI_ADDRCONFIG;   /* be cautious */
#endif

    struct addrinfo *res = NULL;
    int rc = getaddrinfo(host, NULL, &hints, &res);
    if (rc != 0 || !res) {
        if (res) freeaddrinfo(res);
        return -1;
    }

    int fam = -1;
    for (struct addrinfo *p = res; p != NULL; p = p->ai_next) {
        if (p->ai_family == AF_INET || p->ai_family == AF_INET6) {
            fam = p->ai_family;
            break;
        }
    }
    freeaddrinfo(res);
    return fam;
}

/* Get current time in milliseconds (monotonic) */
static int64_t now_ms(void) {
    struct timespec ts;
#if defined(CLOCK_MONOTONIC)
    clock_gettime(CLOCK_MONOTONIC, &ts);
#else
    clock_gettime(CLOCK_REALTIME, &ts);
#endif
    return (int64_t)ts.tv_sec * 1000 + (int64_t)(ts.tv_nsec / 1000000);
}

/* Run ping safely (no shell). Returns malloc'd output string on success (may be partial on timeout); NULL on failure. */
static char *run_ping_capture(const char *host, int family, int timeout_sec, size_t max_capture) {
    if (!host) return NULL;
    if (timeout_sec <= 0) timeout_sec = PING_TIMEOUT_SEC;
    if (max_capture == 0 || max_capture > MAX_OUTPUT_CAPTURE) max_capture = MAX_OUTPUT_CAPTURE;

    int pipefd[2] = {-1, -1};
    if (pipe(pipefd) != 0) {
        return NULL;
    }

    pid_t pid = fork();
    if (pid < 0) {
        close(pipefd[0]);
        close(pipefd[1]);
        return NULL;
    }

    if (pid == 0) {
        /* Child */
        /* Set process group so parent can kill the whole group on timeout */
        (void)setpgid(0, 0);

        /* Redirect stdout and stderr to pipe write end */
        if (dup2(pipefd[1], STDOUT_FILENO) < 0 || dup2(pipefd[1], STDERR_FILENO) < 0) {
            /* If dup2 fails, best effort write an error and exit */
            const char *msg = "error: dup2 failed\n";
            (void)write(pipefd[1], msg, strlen(msg));
            _exit(127);
        }

        /* Close fds not needed by child */
        close(pipefd[0]);
        close(pipefd[1]);

        const char *argv[8];
        size_t ai = 0;
        argv[ai++] = "ping";
        if (family == AF_INET6) {
            argv[ai++] = "-6";
        } else {
            argv[ai++] = "-4";
        }
        argv[ai++] = "-c";
        argv[ai++] = "1";
#if defined(__APPLE__) || defined(__MACH__)
        /* macOS uses -W for timeout in milliseconds; fallback to 3000 ms */
        argv[ai++] = "-W";
        argv[ai++] = "3000";
#else
        /* Linux: -w is deadline in seconds for entire ping */
        argv[ai++] = "-w";
        argv[ai++] = "3";
#endif
        argv[ai++] = host;
        argv[ai] = NULL;

        execvp("ping", (char * const *)argv);

        /* If exec fails, write a constant error (no internal details) and exit */
        const char *msg = "error: ping execution failed\n";
        (void)write(STDOUT_FILENO, msg, strlen(msg));
        _exit(127);
    }

    /* Parent */
    close(pipefd[1]);  /* close write end */

    /* Optional: set read end non-blocking for responsive timeout handling */
    int flags = fcntl(pipefd[0], F_GETFL);
    if (flags != -1) {
        (void)fcntl(pipefd[0], F_SETFL, flags | O_NONBLOCK);
    }

    char *outbuf = (char *)malloc(max_capture + 1);
    if (!outbuf) {
        close(pipefd[0]);
        /* Ensure child is reaped and killed */
        kill(pid, SIGKILL);
        (void)waitpid(pid, NULL, 0);
        return NULL;
    }

    size_t outlen = 0;
    bool timed_out = false;
    int64_t deadline = now_ms() + (int64_t)timeout_sec * 1000;

    struct pollfd pfd;
    pfd.fd = pipefd[0];
    pfd.events = POLLIN;

    for (;;) {
        int64_t now = now_ms();
        int64_t remain = deadline - now;
        if (remain <= 0) {
            timed_out = true;
            break;
        }
        int timeout_ms = (remain > INT32_MAX) ? INT32_MAX : (int)remain;

        int pr = poll(&pfd, 1, timeout_ms);
        if (pr < 0) {
            if (errno == EINTR) continue;
            break;
        } else if (pr == 0) {
            /* timeout of poll; loop to check overall deadline */
            continue;
        } else {
            if (pfd.revents & POLLIN) {
                char buf[4096];
                ssize_t r = read(pipefd[0], buf, sizeof(buf));
                if (r > 0) {
                    size_t to_copy = (size_t)r;
                    if (outlen + to_copy > max_capture) {
                        to_copy = max_capture - outlen;
                    }
                    if (to_copy > 0) {
                        memcpy(outbuf + outlen, buf, to_copy);
                        outlen += to_copy;
                    }
                    /* If buffer full, we can stop reading further */
                    if (outlen >= max_capture) {
                        break;
                    }
                } else if (r == 0) {
                    /* EOF */
                    break;
                } else {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        continue;
                    }
                    break;
                }
            } else if (pfd.revents & (POLLHUP | POLLERR | POLLNVAL)) {
                /* peer closed or error */
                break;
            }
        }
    }

    /* If timed out, kill child; otherwise, reap if finished */
    if (timed_out) {
        /* Kill process group if possible; else kill process */
        (void)kill(-pid, SIGKILL);
        (void)kill(pid, SIGKILL);
    }

    /* Drain remaining data quickly after timeout/termination, with a short poll */
    for (;;) {
        char buf[4096];
        ssize_t r = read(pipefd[0], buf, sizeof(buf));
        if (r > 0) {
            size_t to_copy = (size_t)r;
            if (outlen + to_copy > max_capture) {
                to_copy = max_capture - outlen;
            }
            if (to_copy > 0) {
                memcpy(outbuf + outlen, buf, to_copy);
                outlen += to_copy;
            }
            if (outlen >= max_capture) break;
        } else {
            break;
        }
    }

    close(pipefd[0]);
    (void)waitpid(pid, NULL, 0);

    outbuf[outlen] = '\0';
    /* If nothing captured and timeout occurred, provide a minimal generic error */
    if (outlen == 0 && timed_out) {
        const char *msg = "error: operation timed out\n";
        size_t msg_len = strlen(msg);
        if (msg_len > max_capture) msg_len = max_capture;
        memcpy(outbuf, msg, msg_len);
        outbuf[msg_len] = '\0';
    }
    return outbuf;
}

/* Public function: takes a request URL string, extracts 'url' param, runs ping, returns captured output (malloc'd). */
char *ping(const char *request_url) {
    /* Extract target from URL */
    char *target = extract_target_from_request(request_url);
    if (!target) {
        char *err = (char *)malloc(32);
        if (err) strcpy(err, "error: invalid input\n");
        return err;
    }

    /* Normalize host representation (strip brackets, port) */
    normalize_host_inplace(target);

    /* Validate basic charset and length */
    if (!validate_host_basic(target)) {
        free(target);
        char *err = (char *)malloc(32);
        if (err) strcpy(err, "error: invalid input\n");
        return err;
    }

    /* Resolve to determine family */
    int fam = resolve_family(target);
    if (fam != AF_INET && fam != AF_INET6) {
        free(target);
        char *err = (char *)malloc(32);
        if (err) strcpy(err, "error: invalid host\n");
        return err;
    }

    /* Run ping and capture output */
    char *output = run_ping_capture(target, fam, PING_TIMEOUT_SEC, MAX_OUTPUT_CAPTURE);
    free(target);

    if (!output) {
        char *err = (char *)malloc(32);
        if (err) strcpy(err, "error: internal failure\n");
        return err;
    }
    return output;
}

/* Test harness with 5 test cases */
int main(void) {
    const char *tests[5] = {
        "http://localhost/ping?url=127.0.0.1",
        "http://localhost/ping?url=%3A%3A1",         /* ::1 */
        "http://localhost/ping?url=localhost",
        "http://localhost/ping?url=",
        "http://localhost/ping?url=127.0.0.1;cat/etc/passwd"
    };

    for (int i = 0; i < 5; ++i) {
        printf("=== Test %d ===\n", i + 1);
        char *res = ping(tests[i]);
        if (res) {
            /* Print captured output */
            printf("%s", res);
            free(res);
        } else {
            printf("error: null result\n");
        }
        printf("\n");
    }
    return 0;
}