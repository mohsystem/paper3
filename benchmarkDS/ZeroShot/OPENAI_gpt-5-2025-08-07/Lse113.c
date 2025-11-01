#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <spawn.h>
#include <fcntl.h>
extern char **environ;
#endif

typedef struct {
    int alive;
    char* pong;
    char* hostAscii;
} PingResult;

static char* str_trim(const char* s) {
    if (!s) return NULL;
    const char* a = s;
    while (*a && isspace((unsigned char)*a)) ++a;
    const char* b = s + strlen(s);
    while (b > a && isspace((unsigned char)*(b-1))) --b;
    size_t len = (size_t)(b - a);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, a, len);
    out[len] = '\0';
    return out;
}

static int is_safe_host_ascii(const char* h) {
    if (!h || !*h) return 0;
    size_t len = strlen(h);
    if (len > 253) return 0;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)h[i];
        if (!(isalnum(c) || c == '-' || c == '.' || c == ':')) return 0;
    }
    return 1;
}

static char* extract_host_ascii(const char* url) {
    char* u = str_trim(url);
    if (!u || !*u) { free(u); return NULL; }
    size_t start = 0;
    char* scheme = strstr(u, "://");
    if (scheme) start = (size_t)(scheme - u) + 3;
    size_t end = start;
    while (u[end] && u[end] != '/' && u[end] != '?' && u[end] != '#') ++end;
    size_t len = end - start;
    if (len == 0) { free(u); return NULL; }
    char* host = (char*)malloc(len + 1);
    if (!host) { free(u); return NULL; }
    memcpy(host, u + start, len);
    host[len] = '\0';
    if (len >= 2 && host[0] == '[' && host[len-1] == ']') {
        // strip brackets
        host[len-1] = '\0';
        memmove(host, host + 1, len - 1);
    }
    free(u);
    if (!is_safe_host_ascii(host)) {
        free(host);
        return NULL;
    }
    return host;
}

static PingResult make_error(const char* msg) {
    PingResult r;
    r.alive = 0;
    r.hostAscii = NULL;
    r.pong = (char*)malloc(strlen(msg) + 1);
    if (r.pong) strcpy(r.pong, msg);
    return r;
}

PingResult pingUrl(const char* url) {
    PingResult res;
    res.alive = 0;
    res.pong = NULL;
    res.hostAscii = extract_host_ascii(url);
    if (!res.hostAscii) {
        return make_error("Error: invalid url/host");
    }

#if defined(_WIN32)
    // Build command line: ping -n 1 -w 2000 host
    // hostAscii is validated ASCII
    const char* prefix = "ping -n 1 -w 2000 ";
    size_t cmdlen = strlen(prefix) + strlen(res.hostAscii) + 1;
    wchar_t* wcmd = (wchar_t*)calloc(cmdlen, sizeof(wchar_t));
    if (!wcmd) {
        free(res.hostAscii);
        return make_error("Error: OOM");
    }
    // Convert ASCII to wide
    size_t plen = mbstowcs(NULL, prefix, 0);
    size_t hlen = mbstowcs(NULL, res.hostAscii, 0);
    if (plen == (size_t)-1 || hlen == (size_t)-1) {
        free(wcmd); free(res.hostAscii);
        return make_error("Error: conversion failed");
    }
    mbstowcs(wcmd, prefix, plen + 1);
    mbstowcs(wcmd + plen, res.hostAscii, hlen + 1);

    SECURITY_ATTRIBUTES sa;
    ZeroMemory(&sa, sizeof(sa));
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;

    HANDLE outRead = NULL, outWrite = NULL;
    if (!CreatePipe(&outRead, &outWrite, &sa, 0)) {
        free(wcmd); free(res.hostAscii);
        return make_error("Error: CreatePipe failed");
    }
    SetHandleInformation(outRead, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOW si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags |= STARTF_USESTDHANDLES;
    si.hStdOutput = outWrite;
    si.hStdError  = outWrite;
    si.hStdInput  = NULL;

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    // CreateProcess requires a mutable buffer
    size_t total = wcslen(wcmd);
    wchar_t* cmdBuf = (wchar_t*)malloc((total + 1) * sizeof(wchar_t));
    if (!cmdBuf) {
        CloseHandle(outRead); CloseHandle(outWrite);
        free(wcmd); free(res.hostAscii);
        return make_error("Error: OOM");
    }
    wcscpy(cmdBuf, wcmd);

    BOOL ok = CreateProcessW(
        NULL, cmdBuf, NULL, NULL, TRUE,
        CREATE_NO_WINDOW, NULL, NULL, &si, &pi
    );
    free(wcmd);
    free(cmdBuf);
    CloseHandle(outWrite); // child holds a copy
    if (!ok) {
        CloseHandle(outRead);
        free(res.hostAscii);
        return make_error("Error: CreateProcess failed");
    }

    DWORD waitRes = WaitForSingleObject(pi.hProcess, 5000);
    if (waitRes == WAIT_TIMEOUT) {
        TerminateProcess(pi.hProcess, 1);
    }

    // Read output
    size_t cap = 8192, len = 0;
    char* out = (char*)malloc(cap);
    if (!out) out = NULL;
    char buf[4096];
    DWORD n;
    while (ReadFile(outRead, buf, sizeof(buf), &n, NULL) && n > 0) {
        if (!out) {
            out = (char*)malloc(n + 1);
            if (!out) break;
            memcpy(out, buf, n);
            len = n;
        } else {
            if (len + n + 1 > cap) {
                size_t ncap = (cap * 2 > len + n + 1) ? cap * 2 : (len + n + 1);
                char* tmp = (char*)realloc(out, ncap);
                if (!tmp) break;
                out = tmp; cap = ncap;
            }
            memcpy(out + len, buf, n);
            len += n;
        }
    }
    CloseHandle(outRead);
    if (out) out[len] = '\0';
    res.pong = out;

    DWORD exitCode = 1;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    res.alive = (exitCode == 0) ? 1 : 0;
    return res;

#else
    int pipefd[2];
    if (pipe(pipefd) != 0) {
        free(res.hostAscii);
        return make_error("Error: pipe failed");
    }

    posix_spawn_file_actions_t actions;
    posix_spawn_file_actions_init(&actions);
    posix_spawn_file_actions_adddup2(&actions, pipefd[1], STDOUT_FILENO);
    posix_spawn_file_actions_adddup2(&actions, pipefd[1], STDERR_FILENO);
    posix_spawn_file_actions_addclose(&actions, pipefd[0]);

    char* argv[7];
    argv[0] = "ping";
    argv[1] = "-c";
    argv[2] = "1";
#if defined(__APPLE__)
    argv[3] = "-W";
    argv[4] = (char*)"2000";
#else
    argv[3] = "-W";
    argv[4] = (char*)"2";
#endif
    argv[5] = res.hostAscii;
    argv[6] = NULL;

    pid_t pid;
    int spawnRes = posix_spawnp(&pid, "ping", &actions, NULL, argv, environ);
    posix_spawn_file_actions_destroy(&actions);
    close(pipefd[1]); // parent will read

    if (spawnRes != 0) {
        close(pipefd[0]);
        free(res.hostAscii);
        return make_error("Error: posix_spawnp failed");
    }

    // Read output
    size_t cap = 8192, len = 0;
    char* out = (char*)malloc(cap);
    if (!out) { cap = 0; }
    char buf[4096];
    ssize_t nread;
    while ((nread = read(pipefd[0], buf, sizeof(buf))) > 0) {
        if (!out) {
            out = (char*)malloc((size_t)nread + 1);
            if (!out) break;
            memcpy(out, buf, (size_t)nread);
            len = (size_t)nread;
            cap = len + 1;
        } else {
            if (len + (size_t)nread + 1 > cap) {
                size_t ncap = cap * 2;
                if (ncap < len + (size_t)nread + 1) ncap = len + (size_t)nread + 1;
                char* tmp = (char*)realloc(out, ncap);
                if (!tmp) break;
                out = tmp; cap = ncap;
            }
            memcpy(out + len, buf, (size_t)nread);
            len += (size_t)nread;
        }
    }
    close(pipefd[0]);
    if (out) out[len] = '\0';
    res.pong = out;

    int status = 0;
    waitpid(pid, &status, 0);
    res.alive = (WIFEXITED(status) && WEXITSTATUS(status) == 0) ? 1 : 0;
    return res;
#endif
}

int main(void) {
    const char* tests[5] = {
        "https://example.com",
        "example.com",
        "8.8.8.8",
        "http://[::1]",
        "bad host!!"
    };
    for (int i = 0; i < 5; ++i) {
        PingResult r = pingUrl(tests[i]);
        printf("Test: %s -> alive=%s host=%s pongLen=%zu\n",
               tests[i],
               r.alive ? "true" : "false",
               r.hostAscii ? r.hostAscii : "(null)",
               r.pong ? strlen(r.pong) : 0u);
        free(r.pong);
        free(r.hostAscii);
    }
    return 0;
}