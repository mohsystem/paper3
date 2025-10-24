#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

static int is_root_path(const char* path) {
    if (!path) return 0;
    size_t len = strlen(path);
    if (len == 0) return 0;
#ifdef _WIN32
    // Consider "C:\" or "C:/" or "C:" as roots
    if ((len == 2 && path[1] == ':') ||
        (len == 3 && path[1] == ':' && (path[2] == '\\' || path[2] == '/')))
        return 1;
    if ((len == 1) && (path[0] == '\\' || path[0] == '/')) return 1;
#else
    if (len == 1 && path[0] == '/') return 1;
#endif
    return 0;
}

int delete_file_secure(const char* path) {
    if (path == NULL || *path == '\0') return 0;
    if (is_root_path(path)) return 0;

#ifdef _WIN32
    DWORD attrs = GetFileAttributesA(path);
    if (attrs == INVALID_FILE_ATTRIBUTES) return 0;
    if (attrs & FILE_ATTRIBUTE_DIRECTORY) return 0; // refuse directories
    if (DeleteFileA(path)) return 1;
    return 0;
#else
    struct stat st;
    if (lstat(path, &st) != 0) return 0;
    if (S_ISDIR(st.st_mode)) return 0; // refuse directories
    if (remove(path) == 0) return 1;
    return 0;
#endif
}

/* Helpers for tests */
#ifdef _WIN32
static char* make_temp_file(void) {
    char tempPath[MAX_PATH];
    if (GetTempPathA(MAX_PATH, tempPath) == 0) return NULL;
    char tempFile[MAX_PATH];
    if (GetTempFileNameA(tempPath, "T69", 0, tempFile) == 0) return NULL;
    HANDLE h = CreateFileA(tempFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h != INVALID_HANDLE_VALUE) CloseHandle(h);
    char* out = (char*)malloc(strlen(tempFile) + 1);
    if (!out) return NULL;
    strcpy(out, tempFile);
    return out;
}
static char* make_temp_dir(void) {
    char tempPath[MAX_PATH];
    if (GetTempPathA(MAX_PATH, tempPath) == 0) return NULL;
    char dirPath[MAX_PATH];
    if (GetTempFileNameA(tempPath, "T69", 0, dirPath) == 0) return NULL;
    // GetTempFileName creates a file; delete it and create dir
    DeleteFileA(dirPath);
    if (!CreateDirectoryA(dirPath, NULL)) return NULL;
    char* out = (char*)malloc(strlen(dirPath) + 1);
    if (!out) return NULL;
    strcpy(out, dirPath);
    return out;
}
#else
#include <fcntl.h>
static char* make_temp_file(void) {
    char tmpl[] = "/tmp/task69_XXXXXX";
    int fd = mkstemp(tmpl);
    if (fd == -1) return NULL;
    close(fd);
    char* out = (char*)malloc(strlen(tmpl) + 1);
    if (!out) return NULL;
    strcpy(out, tmpl);
    return out;
}
static char* make_temp_dir(void) {
    char tmpl[] = "/tmp/task69dir_XXXXXX";
    char* d = mkdtemp(tmpl);
    if (!d) return NULL;
    char* out = (char*)malloc(strlen(d) + 1);
    if (!out) return NULL;
    strcpy(out, d);
    return out;
}
#endif

int main(int argc, char* argv[]) {
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            int ok = delete_file_secure(argv[i]);
            printf("%s -> %s\n", argv[i], ok ? "true" : "false");
        }
    } else {
        char* f1 = make_temp_file();
        char* f2 = make_temp_file();
        char* f3 = make_temp_file();
        char* d = make_temp_dir();

        char nonexist[1024] = {0};
        if (d) {
#ifdef _WIN32
            snprintf(nonexist, sizeof(nonexist), "%s\\nonexistent_task69.tmp", d);
#else
            snprintf(nonexist, sizeof(nonexist), "%s/nonexistent_task69.tmp", d);
#endif
        }

        printf("Test1 delete file: %s\n", (f1 && delete_file_secure(f1)) ? "true" : "false");
        printf("Test2 delete file: %s\n", (f2 && delete_file_secure(f2)) ? "true" : "false");
        printf("Test3 delete directory (should be false): %s\n", (d && delete_file_secure(d)) ? "true" : "false");
        printf("Test4 delete non-existent (should be false): %s\n", (d && delete_file_secure(nonexist)) ? "true" : "false");
        printf("Test5 delete file: %s\n", (f3 && delete_file_secure(f3)) ? "true" : "false");

#ifdef _WIN32
        if (d) RemoveDirectoryA(d);
#else
        if (d) rmdir(d);
#endif
        free(f1);
        free(f2);
        free(f3);
        free(d);
    }
    return 0;
}