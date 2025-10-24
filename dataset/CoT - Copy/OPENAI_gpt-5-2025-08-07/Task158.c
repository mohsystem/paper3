/* Chain-of-Through process in code generation:
 * 1) Problem understanding: Safely use user input in an OS command (ping once).
 * 2) Security requirements: Avoid shell; validate input; enforce timeouts; safe IO capture.
 * 3) Secure coding generation: POSIX fork/exec/pipe with strict validation and timeout.
 * 4) Code review: No shell usage; character allowlist; kill child on timeout; cap output size.
 * 5) Secure code output: Final secure implementation below.
 *
 * Note: This C code targets POSIX systems (Linux/macOS).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <time.h>

static int isValidHostOrIPv4(const char* host) {
    if (host == NULL) return 0;
    size_t len = strlen(host);
    if (len == 0 || len > 253) return 0;
    for (size_t i = 0; i < len; i++) {
        char c = host[i];
        if (!(isalnum((unsigned char)c) || c == '-' || c == '.')) return 0;
    }
    // Compile regexes once per call (simple approach). In production, cache them.
    regex_t re_host, re_ipv4;
    int rc1 = regcomp(&re_host,
        "^(?=.{1,253}$)([A-Za-z0-9]([A-Za-z0-9-]{0,61}[A-Za-z0-9])?)(\\.[A-Za-z0-9]([A-Za-z0-9-]{0,61}[A-Za-z0-9])?)*$",
        REG_EXTENDED);
    int rc2 = regcomp(&re_ipv4,
        "^((25[0-5]|2[0-4][0-9]|1?[0-9]?[0-9])(\\.|$)){4}$",
        REG_EXTENDED);
    if (rc1 != 0 || rc2 != 0) {
        if (rc1 == 0) regfree(&re_host);
        if (rc2 == 0) regfree(&re_ipv4);
        return 0;
    }
    int match = (regexec(&re_host, host, 0, NULL, 0) == 0) || (regexec(&re_ipv4, host, 0, NULL, 0) == 0);
    regfree(&re_host);
    regfree(&re_ipv4);
    return match;
}

static char* safe_ping(const char* host, int timeout_sec, int* exit_code) {
    if (!isValidHostOrIPv4(host)) {
        if (exit_code) *exit_code = 1;
        char* msg = strdup("Invalid host input rejected.");
        return msg ? msg : NULL;
    }
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        if (exit_code) *exit_code = 1;
        char buf[128];
        snprintf(buf, sizeof(buf), "pipe failed: %s", strerror(errno));
        return strdup(buf);
    }
    pid_t pid = fork();
    if (pid < 0) {
        close(pipefd[0]); close(pipefd[1]);
        if (exit_code) *exit_code = 1;
        char buf[128];
        snprintf(buf, sizeof(buf), "fork failed: %s", strerror(errno));
        return strdup(buf);
    }
    if (pid == 0) {
        // Child
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        char secbuf[8];
        int sec = timeout_sec;
        if (sec < 1) sec = 1;
        if (sec > 10) sec = 10;
        snprintf(secbuf, sizeof(secbuf), "%d", sec);
        // execlp ping with args: -c 1 -W sec host
        execlp("ping", "ping", "-c", "1", "-W", secbuf, host, (char*)NULL);
        const char* msg = "exec failed\n";
        write(STDOUT_FILENO, msg, strlen(msg));
        _exit(127);
    }
    // Parent
    close(pipefd[1]);
    int flags = fcntl(pipefd[0], F_GETFL, 0);
    fcntl(pipefd[0], F_SETFL, flags | O_NONBLOCK);

    size_t cap = 4096;
    size_t len = 0;
    size_t max_out = 65536;
    char* output = (char*)malloc(cap);
    if (!output) {
        close(pipefd[0]);
        if (exit_code) *exit_code = 1;
        kill(pid, SIGKILL);
        waitpid(pid, NULL, 0);
        return strdup("memory allocation failed");
    }

    time_t start = time(NULL);
    int status = 0;
    int child_done = 0;

    for (;;) {
        char buf[2048];
        ssize_t n = read(pipefd[0], buf, sizeof(buf));
        if (n > 0) {
            size_t to_append = (len + (size_t)n > max_out) ? (max_out - len) : (size_t)n;
            if (to_append > 0) {
                if (len + to_append + 1 > cap) {
                    size_t newcap = cap;
                    while (len + to_append + 1 > newcap) newcap *= 2;
                    if (newcap > max_out + 1) newcap = max_out + 1;
                    char* tmp = (char*)realloc(output, newcap);
                    if (!tmp) {
                        free(output);
                        close(pipefd[0]);
                        if (exit_code) *exit_code = 1;
                        kill(pid, SIGKILL);
                        waitpid(pid, NULL, 0);
                        return strdup("memory allocation failed");
                    }
                    output = tmp;
                    cap = newcap;
                }
                memcpy(output + len, buf, to_append);
                len += to_append;
                output[len] = '\0';
            }
        }
        pid_t r = waitpid(pid, &status, WNOHANG);
        if (r == pid) child_done = 1;

        time_t now = time(NULL);
        if (child_done) break;
        if (now - start > timeout_sec) {
            kill(pid, SIGKILL);
            waitpid(pid, &status, 0);
            close(pipefd[0]);
            if (exit_code) *exit_code = 124;
            if (len == 0) {
                free(output);
                return strdup("Command timed out.");
            } else {
                // append timeout notice if space allows
                const char* tmsg = "\n[timeout]";
                size_t tlen = strlen(tmsg);
                if (len + tlen + 1 <= cap) {
                    memcpy(output + len, tmsg, tlen);
                    len += tlen;
                    output[len] = '\0';
                }
                return output;
            }
        }
        usleep(30000);
    }
    close(pipefd[0]);
    if (len == 0) {
        const char* msg = "No output.";
        size_t mlen = strlen(msg);
        if (cap < mlen + 1) {
            char* tmp = (char*)realloc(output, mlen + 1);
            if (!tmp) {
                free(output);
                if (exit_code) *exit_code = 1;
                return strdup("memory allocation failed");
            }
            output = tmp;
        }
        memcpy(output, msg, mlen + 1);
        len = mlen;
    } else {
        output[len] = '\0';
    }
    if (WIFEXITED(status)) {
        if (exit_code) *exit_code = WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        if (exit_code) *exit_code = 128 + WTERMSIG(status);
    } else {
        if (exit_code) *exit_code = 1;
    }
    return output;
}

int main(void) {
    const char* tests[5] = {
        "localhost",
        "127.0.0.1",
        "example.com",
        "invalid host", /* should be rejected */
        "8.8.8.8"
    };
    for (int i = 0; i < 5; i++) {
        int code = 0;
        char* out = safe_ping(tests[i], 3, &code);
        printf("Test host: %s\n", tests[i]);
        if (out) {
            printf("%s\n", out);
            free(out);
        } else {
            printf("No output (NULL)\n");
        }
        printf("----\n");
    }
    return 0;
}