#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#if defined(_WIN32)
#include <windows.h>
#endif

#if !defined(_WIN32)
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif

static int is_safe_char(char c) {
    if (isalnum((unsigned char)c)) return 1;
    const char* allowed = " _.,:@%+/=-/";
    for (const char* p = allowed; *p; ++p) {
        if (*p == c) return 1;
    }
    return 0;
}

static int is_safe(const char* s) {
    if (!s) return 0;
    size_t len = strlen(s);
    if (len > 200) return 0;
    for (size_t i = 0; i < len; ++i) {
        if (!is_safe_char(s[i])) return 0;
    }
    return 1;
}

char* run_echo(const char* input) {
    if (!is_safe(input)) {
        return NULL;
    }

#if defined(_WIN32)
    // Build command: cmd.exe /c echo <input>
    size_t cmdlen = strlen("cmd.exe /c echo ") + strlen(input) + 1;
    char* cmd = (char*)malloc(cmdlen);
    if (!cmd) return NULL;
    snprintf(cmd, cmdlen, "cmd.exe /c echo %s", input);

    FILE* pipe = _popen(cmd, "r");
    free(cmd);
    if (!pipe) return NULL;

    size_t cap = 256;
    size_t len = 0;
    char* out = (char*)malloc(cap);
    if (!out) { _pclose(pipe); return NULL; }
    int c;
    while ((c = fgetc(pipe)) != EOF) {
        if (len + 1 >= cap) {
            cap *= 2;
            char* tmp = (char*)realloc(out, cap);
            if (!tmp) { free(out); _pclose(pipe); return NULL; }
            out = tmp;
        }
        out[len++] = (char)c;
    }
    _pclose(pipe);
    // Trim trailing CR/LF
    while (len > 0 && (out[len-1] == '\n' || out[len-1] == '\r')) len--;
    out[len] = '\0';
    return out;
#else
    int fd[2];
    if (pipe(fd) == -1) return NULL;
    pid_t pid = fork();
    if (pid < 0) {
        close(fd[0]); close(fd[1]);
        return NULL;
    }
    if (pid == 0) {
        // Child
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]);
        close(fd[1]);
        execl("/bin/echo", "echo", input, (char*)NULL);
        _exit(127);
    } else {
        // Parent
        close(fd[1]);
        size_t cap = 256;
        size_t len = 0;
        char* out = (char*)malloc(cap);
        if (!out) { close(fd[0]); return NULL; }
        ssize_t nread;
        char buf[512];
        while ((nread = read(fd[0], buf, sizeof(buf))) > 0) {
            if (len + (size_t)nread + 1 > cap) {
                while (len + (size_t)nread + 1 > cap) cap *= 2;
                char* tmp = (char*)realloc(out, cap);
                if (!tmp) { free(out); close(fd[0]); return NULL; }
                out = tmp;
            }
            memcpy(out + len, buf, (size_t)nread);
            len += (size_t)nread;
        }
        close(fd[0]);
        int status = 0;
        waitpid(pid, &status, 0);
        // Trim trailing newline(s)
        while (len > 0 && (out[len-1] == '\n' || out[len-1] == '\r')) len--;
        out[len] = '\0';
        if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
            free(out);
            return NULL;
        }
        return out;
    }
#endif
}

int main(void) {
    // 5 test cases
    char* o1 = run_echo("Hello World");
    char* o2 = run_echo("Sample_123");
    char* o3 = run_echo("Path/with/slash");
    char* o4 = run_echo("Value+Plus=Equals");
    char* o5 = run_echo("Email: user@example.com");
    if (o1) { printf("%s\n", o1); free(o1); } else { printf("Error\n"); }
    if (o2) { printf("%s\n", o2); free(o2); } else { printf("Error\n"); }
    if (o3) { printf("%s\n", o3); free(o3); } else { printf("Error\n"); }
    if (o4) { printf("%s\n", o4); free(o4); } else { printf("Error\n"); }
    if (o5) { printf("%s\n", o5); free(o5); } else { printf("Error\n"); }

    // Interactive input
    char buf[512];
    if (!fgets(buf, sizeof(buf), stdin)) {
        buf[0] = '\0';
    } else {
        size_t l = strlen(buf);
        if (l && (buf[l-1] == '\n' || buf[l-1] == '\r')) buf[l-1] = '\0';
    }
    char* out = run_echo(buf);
    if (out) {
        printf("%s\n", out);
        free(out);
    } else {
        printf("Error\n");
    }
    return 0;
}