#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
  #include <windows.h>
  #include <io.h>
  #include <sys/stat.h>
#else
  #include <unistd.h>
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <sys/wait.h>
  #include <fcntl.h>
#endif

static int is_regular_and_readable(const char* path) {
    if (path == NULL) return 0;
#ifdef _WIN32
    struct _stat st;
    if (_stat(path, &st) != 0) return 0;
    if ((st.st_mode & _S_IFREG) == 0) return 0;
    FILE* f = fopen(path, "rb");
    if (!f) return 0;
    fclose(f);
    return 1;
#else
    struct stat st;
    if (stat(path, &st) != 0) return 0;
    if (!S_ISREG(st.st_mode)) return 0;
    FILE* f = fopen(path, "rb");
    if (!f) return 0;
    fclose(f);
    return 1;
#endif
}

#ifdef _WIN32
static char* quote_windows_arg(const char* s) {
    size_t len = strlen(s);
    // Worst-case expansion: escape every char plus quotes + 2 quotes
    size_t cap = len * 2 + 3;
    char* out = (char*)malloc(cap);
    if (!out) return NULL;
    size_t j = 0;
    out[j++] = '"';
    for (size_t i = 0; i < len; ++i) {
        if (s[i] == '"') {
            if (j + 2 >= cap) {
                cap *= 2;
                char* tmp = (char*)realloc(out, cap);
                if (!tmp) { free(out); return NULL; }
                out = tmp;
            }
            out[j++] = '\\';
            out[j++] = '"';
        } else {
            if (j + 1 >= cap) {
                cap *= 2;
                char* tmp = (char*)realloc(out, cap);
                if (!tmp) { free(out); return NULL; }
                out = tmp;
            }
            out[j++] = s[i];
        }
    }
    if (j + 2 >= cap) {
        cap += 2;
        char* tmp = (char*)realloc(out, cap);
        if (!tmp) { free(out); return NULL; }
        out = tmp;
    }
    out[j++] = '"';
    out[j] = '\0';
    return out;
}
#endif

// Returns a newly allocated string with the file contents (or error); caller must free().
char* display_file_secure(const char* filename) {
    if (filename == NULL || *filename == '\0') {
        char* msg = (char*)malloc(24); if (!msg) return NULL;
        strcpy(msg, "Error: Invalid filename.");
        return msg;
    }
    for (const char* p = filename; *p; ++p) {
        if (*p == '\0') {
            char* msg = (char*)malloc(36); if (!msg) return NULL;
            strcpy(msg, "Error: Invalid character in filename.");
            return msg;
        }
    }

#ifdef _WIN32
    char full[MAX_PATH];
    if (!_fullpath(full, filename, MAX_PATH)) {
        char* msg = (char*)malloc(24); if (!msg) return NULL;
        strcpy(msg, "Error: Invalid filename.");
        return msg;
    }
    // Normalize by removing .. etc is non-trivial; rely on _fullpath + checks
    if (!is_regular_and_readable(full)) {
        struct _stat st;
        if (_stat(full, &st) != 0) {
            char* msg = (char*)malloc(27); if (!msg) return NULL;
            strcpy(msg, "Error: File does not exist.");
            return msg;
        }
        if ((st.st_mode & _S_IFREG) == 0) {
            char* msg = (char*)malloc(27); if (!msg) return NULL;
            strcpy(msg, "Error: Not a regular file.");
            return msg;
        }
        char* msg = (char*)malloc(28); if (!msg) return NULL;
        strcpy(msg, "Error: File is not readable.");
        return msg;
    }

    SECURITY_ATTRIBUTES sa;
    ZeroMemory(&sa, sizeof(sa));
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    HANDLE hRead = NULL, hWrite = NULL;
    if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
        char* msg = (char*)malloc(30); if (!msg) return NULL;
        strcpy(msg, "Error: CreatePipe failed.");
        return msg;
    }

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    ZeroMemory(&pi, sizeof(pi));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = hWrite;
    si.hStdError = hWrite;

    char* q = quote_windows_arg(full);
    if (!q) {
        CloseHandle(hRead); CloseHandle(hWrite);
        return NULL;
    }
    size_t cmdlen = strlen("more ") + strlen(q) + 1;
    char* cmd = (char*)malloc(cmdlen);
    if (!cmd) {
        free(q); CloseHandle(hRead); CloseHandle(hWrite);
        return NULL;
    }
    snprintf(cmd, cmdlen, "more %s", q);
    free(q);

    // CreateProcess may modify command line; make it mutable
    BOOL ok = CreateProcessA(
        NULL,
        cmd,
        NULL,
        NULL,
        TRUE,
        CREATE_NO_WINDOW,
        NULL,
        NULL,
        &si,
        &pi
    );
    // Parent doesn't need write end
    CloseHandle(hWrite);
    if (!ok) {
        CloseHandle(hRead);
        free(cmd);
        char* msg = (char*)malloc(32); if (!msg) return NULL;
        strcpy(msg, "Error: CreateProcess failed.");
        return msg;
    }
    free(cmd);

    size_t cap = 8192;
    size_t len = 0;
    char* out = (char*)malloc(cap + 1);
    if (!out) {
        CloseHandle(hRead);
        CloseHandle(pi.hThread);
        TerminateProcess(pi.hProcess, 1);
        CloseHandle(pi.hProcess);
        return NULL;
    }

    DWORD bytesRead = 0;
    char buffer[8192];
    for (;;) {
        BOOL r = ReadFile(hRead, buffer, sizeof(buffer), &bytesRead, NULL);
        if (!r || bytesRead == 0) break;
        if (len + bytesRead >= cap) {
            while (len + bytesRead >= cap) cap *= 2;
            char* tmp = (char*)realloc(out, cap + 1);
            if (!tmp) {
                free(out);
                CloseHandle(hRead);
                CloseHandle(pi.hThread);
                TerminateProcess(pi.hProcess, 1);
                CloseHandle(pi.hProcess);
                return NULL;
            }
            out = tmp;
        }
        memcpy(out + len, buffer, bytesRead);
        len += bytesRead;
    }
    CloseHandle(hRead);

    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    out[len] = '\0';
    if (exitCode != 0) {
        const char* prefix = "Error: Command failed with exit code ";
        char codebuf[32];
        _snprintf(codebuf, sizeof(codebuf), "%lu", (unsigned long)exitCode);
        const char* mid = ". Output: ";
        size_t newlen = strlen(prefix) + strlen(codebuf) + strlen(mid) + len + 1;
        char* msg = (char*)malloc(newlen);
        if (!msg) { free(out); return NULL; }
        strcpy(msg, prefix);
        strcat(msg, codebuf);
        strcat(msg, mid);
        strcat(msg, out);
        free(out);
        return msg;
    }
    return out;

#else
    char full[4096];
    if (!realpath(filename, full)) {
        char* msg = (char*)malloc(27); if (!msg) return NULL;
        strcpy(msg, "Error: File does not exist.");
        return msg;
    }
    if (!is_regular_and_readable(full)) {
        struct stat st;
        if (stat(full, &st) != 0) {
            char* msg = (char*)malloc(27); if (!msg) return NULL;
            strcpy(msg, "Error: File does not exist.");
            return msg;
        }
        if (!S_ISREG(st.st_mode)) {
            char* msg = (char*)malloc(27); if (!msg) return NULL;
            strcpy(msg, "Error: Not a regular file.");
            return msg;
        }
        char* msg = (char*)malloc(28); if (!msg) return NULL;
        strcpy(msg, "Error: File is not readable.");
        return msg;
    }

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        char* msg = (char*)malloc(22); if (!msg) return NULL;
        strcpy(msg, "Error: pipe failed.");
        return msg;
    }

    pid_t pid = fork();
    if (pid < 0) {
        close(pipefd[0]); close(pipefd[1]);
        char* msg = (char*)malloc(22); if (!msg) return NULL;
        strcpy(msg, "Error: fork failed.");
        return msg;
    }

    if (pid == 0) {
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        execlp("cat", "cat", full, (char*)NULL);
        _exit(127);
    }

    close(pipefd[1]);
    size_t cap = 8192, len = 0;
    char* out = (char*)malloc(cap + 1);
    if (!out) {
        close(pipefd[0]);
        return NULL;
    }
    ssize_t r;
    char buf[8192];
    while ((r = read(pipefd[0], buf, sizeof(buf))) > 0) {
        if (len + (size_t)r >= cap) {
            while (len + (size_t)r >= cap) cap *= 2;
            char* tmp = (char*)realloc(out, cap + 1);
            if (!tmp) { free(out); close(pipefd[0]); return NULL; }
            out = tmp;
        }
        memcpy(out + len, buf, (size_t)r);
        len += (size_t)r;
    }
    close(pipefd[0]);
    out[len] = '\0';

    int status = 0;
    waitpid(pid, &status, 0);
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        int code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
        const char* prefix = "Error: Command failed with exit code ";
        char codebuf[32];
        snprintf(codebuf, sizeof(codebuf), "%d", code);
        const char* mid = ". Output: ";
        size_t newlen = strlen(prefix) + strlen(codebuf) + strlen(mid) + len + 1;
        char* msg = (char*)malloc(newlen);
        if (!msg) { free(out); return NULL; }
        strcpy(msg, prefix);
        strcat(msg, codebuf);
        strcat(msg, mid);
        strcat(msg, out);
        free(out);
        return msg;
    }
    return out;
#endif
}

int main(void) {
    // Prepare 5 test cases
#ifdef _WIN32
    char tempPath[MAX_PATH];
    GetTempPathA(MAX_PATH, tempPath);
    char dirPath[MAX_PATH];
    snprintf(dirPath, sizeof(dirPath), "%s%s", tempPath, "task39_c_tests\\");
    CreateDirectoryA(dirPath, NULL);

    char f1[MAX_PATH], f2[MAX_PATH], f3[MAX_PATH], invalid[MAX_PATH], dirSub[MAX_PATH];
    snprintf(f1, sizeof(f1), "%sfile1.txt", dirPath);
    snprintf(f2, sizeof(f2), "%sfile with spaces.txt", dirPath);
    snprintf(f3, sizeof(f3), "%sempty.txt", dirPath);
    snprintf(invalid, sizeof(invalid), "%sno_such_file.txt", dirPath);
    snprintf(dirSub, sizeof(dirSub), "%ssubdir", dirPath);

    {
        FILE* fp = fopen(f1, "wb"); if (fp) { fputs("Hello from file1\nLine 2\n", fp); fclose(fp); }
    }
    {
        FILE* fp = fopen(f2, "wb"); if (fp) { fputs("Content with spaces in filename\n", fp); fclose(fp); }
    }
    {
        FILE* fp = fopen(f3, "wb"); if (fp) fclose(fp);
    }
    CreateDirectoryA(dirSub, NULL);
#else
    char dirPath[] = "/tmp/task39_c_testsXXXXXX";
    if (!mkdtemp(dirPath)) {
        fprintf(stderr, "Failed to create temp dir\n");
        return 1;
    }
    char f1[4096], f2[4096], f3[4096], invalid[4096], dirSub[4096];
    snprintf(f1, sizeof(f1), "%s/%s", dirPath, "file1.txt");
    snprintf(f2, sizeof(f2), "%s/%s", dirPath, "file with spaces.txt");
    snprintf(f3, sizeof(f3), "%s/%s", dirPath, "empty.txt");
    snprintf(invalid, sizeof(invalid), "%s/%s", dirPath, "no_such_file.txt");
    snprintf(dirSub, sizeof(dirSub), "%s/%s", dirPath, "subdir");

    {
        FILE* fp = fopen(f1, "wb"); if (fp) { fputs("Hello from file1\nLine 2\n", fp); fclose(fp); }
    }
    {
        FILE* fp = fopen(f2, "wb"); if (fp) { fputs("Content with spaces in filename\n", fp); fclose(fp); }
    }
    {
        FILE* fp = fopen(f3, "wb"); if (fp) fclose(fp);
    }
    mkdir(dirSub, 0700);
#endif

    printf("Test 1 (regular file):\n");
    char* out1 = display_file_secure(f1);
    if (out1) { printf("%s\n", out1); free(out1); } else { printf("Error: allocation failed\n"); }

    printf("Test 2 (file with spaces):\n");
    char* out2 = display_file_secure(f2);
    if (out2) { printf("%s\n", out2); free(out2); } else { printf("Error: allocation failed\n"); }

    printf("Test 3 (empty file):\n");
    char* out3 = display_file_secure(f3);
    if (out3) { printf("%s\n", out3); free(out3); } else { printf("Error: allocation failed\n"); }

    printf("Test 4 (invalid path):\n");
    char* out4 = display_file_secure(invalid);
    if (out4) { printf("%s\n", out4); free(out4); } else { printf("Error: allocation failed\n"); }

    printf("Test 5 (directory path):\n");
    char* out5 = display_file_secure(dirSub);
    if (out5) { printf("%s\n", out5); free(out5); } else { printf("Error: allocation failed\n"); }

    return 0;
}