#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#if defined(_WIN32)
  #include <windows.h>
#else
  #include <unistd.h>
  #include <sys/wait.h>
  #include <fcntl.h>
#endif

static int is_digits(const char* s) {
    if (!s || *s == '\0') return 0;
    for (const char* p = s; *p; ++p) {
        if (!isdigit((unsigned char)*p)) return 0;
    }
    return 1;
}

static int is_ipv4(const char* s) {
    if (!s) return 0;
    int dots = 0;
    for (const char* p = s; *p; ++p) if (*p == '.') dots++;
    if (dots != 3) return 0;

    const char* start = s;
    for (int i = 0; i < 4; i++) {
        const char* dot = strchr(start, '.');
        size_t len = dot ? (size_t)(dot - start) : strlen(start);
        if (len == 0 || len > 3) return 0;
        char part[4] = {0};
        memcpy(part, start, len);
        if (!is_digits(part)) return 0;
        int val = atoi(part);
        if (val < 0 || val > 255) return 0;
        if (!dot) break;
        start = dot + 1;
    }
    return 1;
}

static int is_hostname_label(const char* s, size_t len) {
    if (len == 0 || len > 63) return 0;
    if (!isalnum((unsigned char)s[0])) return 0;
    if (!isalnum((unsigned char)s[len-1])) return 0;
    for (size_t i = 0; i < len; i++) {
        char c = s[i];
        if (!(isalnum((unsigned char)c) || c == '-')) return 0;
    }
    return 1;
}

static int is_hostname(const char* s) {
    if (!s) return 0;
    size_t n = strlen(s);
    if (n == 0 || n > 253) return 0;
    if (strcmp(s, "localhost") == 0 || strcmp(s, "LOCALHOST") == 0) return 1;
    const char* start = s;
    for (size_t i = 0; i <= n; i++) {
        if (i == n || s[i] == '.') {
            size_t len = (size_t)(&s[i] - start);
            if (!is_hostname_label(start, len)) return 0;
            start = s + i + 1;
        } else {
            char c = s[i];
            if (!(isalnum((unsigned char)c) || c == '-' || c == '.')) return 0;
        }
    }
    return 1;
}

static void trim(const char* in, char* out, size_t outsz) {
    size_t len = in ? strlen(in) : 0;
    size_t i = 0, j = len;
    while (i < len && isspace((unsigned char)in[i])) i++;
    while (j > i && isspace((unsigned char)in[j-1])) j--;
    size_t n = (j > i) ? (j - i) : 0;
    if (n >= outsz) n = outsz - 1;
    memcpy(out, in + i, n);
    out[n] = '\0';
}

static void extract_host(const char* input, char* host, size_t hostsz) {
    host[0] = '\0';
    if (!input) return;
    char s[1024];
    trim(input, s, sizeof(s));
    if (s[0] == '\0') return;
    const char* p = strstr(s, "://");
    const char* start = p ? (p + 3) : s;
    // cut at / ? #
    const char* cut = start;
    while (*cut && *cut != '/' && *cut != '?' && *cut != '#') cut++;
    size_t n = (size_t)(cut - start);
    if (n >= hostsz) n = hostsz - 1;
    memcpy(host, start, n);
    host[n] = '\0';
}

static int is_valid_host(const char* host) {
    if (!host) return 0;
    size_t n = strlen(host);
    if (n == 0 || n > 253) return 0;
    for (size_t i = 0; i < n; i++) {
        char c = host[i];
        if (!(isalnum((unsigned char)c) || c == '.' || c == '-')) return 0;
    }
    if (strcmp(host, "localhost") == 0 || strcmp(host, "LOCALHOST") == 0) return 1;
    if (is_ipv4(host)) return 1;
    return is_hostname(host);
}

static int run_ping(const char* host) {
#if defined(_WIN32)
    // CreateProcess without shell; command line must be mutable
    char cmd[1024];
    // -n 1 one echo, -w 2000 timeout ms
    if (snprintf(cmd, sizeof(cmd), "ping -n 1 -w 2000 %s", host) >= (int)sizeof(cmd)) {
        return 0;
    }
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);
    BOOL ok = CreateProcessA(NULL, cmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    if (!ok) return 0;
    DWORD waitRes = WaitForSingleObject(pi.hProcess, 3000);
    if (waitRes == WAIT_TIMEOUT) {
        TerminateProcess(pi.hProcess, 1);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        return 0;
    }
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return 1;
#else
    pid_t pid = fork();
    if (pid < 0) return 0;
    if (pid == 0) {
        int devnull = open("/dev/null", O_WRONLY);
        if (devnull >= 0) {
            dup2(devnull, STDOUT_FILENO);
            dup2(devnull, STDERR_FILENO);
            close(devnull);
        }
        execlp("ping", "ping", "-c", "1", "-W", "2", host, (char*)NULL);
        _exit(127);
    } else {
        int status = 0;
        if (waitpid(pid, &status, 0) < 0) return 0;
        return 1;
    }
#endif
}

// Returns a newly allocated string; caller must free().
char* ping_url(const char* input) {
    char host[512];
    extract_host(input ? input : "", host, sizeof(host));
    if (!is_valid_host(host)) {
        const char* msg = "Error: invalid or unsafe URL";
        char* res = (char*)malloc(strlen(msg) + 1);
        if (!res) return NULL;
        strcpy(res, msg);
        return res;
    }
    if (!run_ping(host)) {
        const char* msg = "Error: failed to run ping or timeout";
        char* res = (char*)malloc(strlen(msg) + 1);
        if (!res) return NULL;
        strcpy(res, msg);
        return res;
    }
    const char* prefix = "URL: ";
    size_t len = strlen(prefix) + strlen(host) + 1;
    char* res = (char*)malloc(len);
    if (!res) return NULL;
    snprintf(res, len, "%s%s", prefix, host);
    return res;
}

int main(void) {
    const char* tests[5] = {
        "http://example.com",
        "example.com",
        "127.0.0.1",
        "bad; rm -rf /",
        "https://localhost/path?q=1"
    };
    for (int i = 0; i < 5; i++) {
        char* out = ping_url(tests[i]);
        if (out) {
            printf("Input: %s -> %s\n", tests[i], out);
            free(out);
        } else {
            printf("Input: %s -> Error: allocation failed\n", tests[i]);
        }
    }
    return 0;
}