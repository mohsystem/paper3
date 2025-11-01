#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

// Returns 1 if valid, 0 otherwise
int is_valid_host(const char* url) {
    if (url == NULL) return 0;
    size_t len = strlen(url);
    if (len == 0 || len > 253) return 0;

    // Reject control chars and newlines
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)url[i];
        if (c == '\n' || c == '\r' || c == '\0') return 0;
    }

    if (strcmp(url, "localhost") == 0 || strcmp(url, "LOCALHOST") == 0) return 1;

    regex_t ipv4;
    regex_t host;
    int rc1 = regcomp(&ipv4, "^(?:(?:25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?\\d)\\.){3}(?:25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?\\d)$", REG_EXTENDED | REG_NOSUB);
    int rc2 = regcomp(&host, "^(?=.{1,253}$)(?:[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?\\.)+[A-Za-z]{2,63}$", REG_EXTENDED | REG_NOSUB);

    int ok = 0;
    if (rc1 == 0 && regexec(&ipv4, url, 0, NULL, 0) == 0) ok = 1;
    if (!ok && rc2 == 0 && regexec(&host, url, 0, NULL, 0) == 0) ok = 1;

    if (rc1 == 0) regfree(&ipv4);
    if (rc2 == 0) regfree(&host);
    return ok;
}

// Returns 1 on success and copies url to out (null-terminated), 0 on failure
int secure_ping(const char* url, char* out, size_t outsz) {
    if (!is_valid_host(url) || out == NULL || outsz == 0) {
        return 0;
    }

    const char* prog = "ping";
    const char* args[] = {"ping", "-c", "1", "-W", "2", url, NULL};

    pid_t pid = fork();
    if (pid < 0) {
        return 0;
    } else if (pid == 0) {
        int fd = open("/dev/null", O_WRONLY);
        if (fd >= 0) {
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            if (fd > STDERR_FILENO) close(fd);
        }
        execvp(prog, (char* const*)args);
        _exit(127);
    } else {
        int status = 0;
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 100000000; // 100ms

        int loops = 30; // ~3 seconds
        while (loops-- > 0) {
            pid_t r = waitpid(pid, &status, WNOHANG);
            if (r == pid) {
                // Success (regardless of ping result code, we demonstrate execution)
                strncpy(out, url, outsz - 1);
                out[outsz - 1] = '\0';
                return 1;
            } else if (r < 0) {
                return 0;
            }
            nanosleep(&ts, NULL);
        }
        kill(pid, SIGKILL);
        waitpid(pid, NULL, 0);
        return 0;
    }
}

int main(void) {
    const char* tests[5] = {
        "example.com",
        "127.0.0.1",
        "8.8.8.8",
        "invalid;;host",
        "localhost"
    };
    char out[256];
    for (int i = 0; i < 5; ++i) {
        int ok = secure_ping(tests[i], out, sizeof(out));
        printf("Input: %s -> Returned: %s\n", tests[i], ok ? out : "null");
    }
    return 0;
}