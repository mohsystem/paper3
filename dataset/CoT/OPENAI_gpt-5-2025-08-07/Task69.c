/* Chain-of-Through process in code generation:
 * 1) Problem understanding: Delete a file specified as command-line argument; provide a function accepting the path and returning success/failure.
 * 2) Security requirements: Validate input, ensure the target is a regular file and not a directory or symlink, handle errors robustly.
 * 3) Secure coding generation: On POSIX use lstat+unlink; on Windows use GetFileAttributesA/DeleteFileA and reject reparse points.
 * 4) Code review: Checks for null/empty path, existence, file type checks, and safe deletion.
 * 5) Secure code output: Provide function and main with five test cases (if no args); otherwise process CLI args.
 */

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
  #include <windows.h>
  #include <io.h>
  #include <fcntl.h>
#else
  #include <sys/stat.h>
  #include <unistd.h>
  #include <errno.h>
  #include <stdlib.h>
  #include <fcntl.h>
  #include <time.h>
#endif

int delete_file_secure(const char* path) {
    if (path == NULL) return 0;
    while (*path == ' ' || *path == '\t' || *path == '\n' || *path == '\r') path++;
    if (*path == '\0') return 0;

#ifdef _WIN32
    DWORD attr = GetFileAttributesA(path);
    if (attr == INVALID_FILE_ATTRIBUTES) return 0;

    // Reject directories and reparse points (symlinks, mount points)
    if (attr & FILE_ATTRIBUTE_DIRECTORY) return 0;
    if (attr & FILE_ATTRIBUTE_REPARSE_POINT) return 0;

    if (DeleteFileA(path)) return 1;
    return 0;
#else
    struct stat st;
    if (lstat(path, &st) != 0) {
        return 0;
    }

    // Reject symlinks and directories; allow only regular files
    if (S_ISLNK(st.st_mode)) return 0;
    if (S_ISDIR(st.st_mode)) return 0;
    if (!S_ISREG(st.st_mode)) return 0;

    if (unlink(path) == 0) return 1;
    return 0;
#endif
}

static int create_temp_file(char* outPath, size_t outSize) {
#ifdef _WIN32
    char tempDir[MAX_PATH];
    if (GetTempPathA(MAX_PATH, tempDir) == 0) return 0;

    char tempFile[MAX_PATH];
    if (GetTempFileNameA(tempDir, "T69", 0, tempFile) == 0) return 0;

    // Ensure the file exists (GetTempFileName creates it)
    if (strlen(tempFile) + 1 > outSize) return 0;
    strcpy(outPath, tempFile);
    return 1;
#else
    const char* dir = "/tmp";
    char pattern[256];
    snprintf(pattern, sizeof(pattern), "%s/Task69_XXXXXX", dir);
    int fd = mkstemp(pattern);
    if (fd == -1) return 0;
    close(fd);
    if (strlen(pattern) + 1 > outSize) {
        unlink(pattern);
        return 0;
    }
    strcpy(outPath, pattern);
    return 1;
#endif
}

static int create_temp_dir(char* outPath, size_t outSize) {
#ifdef _WIN32
    char tempDir[MAX_PATH];
    if (GetTempPathA(MAX_PATH, tempDir) == 0) return 0;

    // Build a unique directory name
    char dirPath[MAX_PATH];
    SYSTEMTIME st;
    GetSystemTime(&st);
    DWORD pid = GetCurrentProcessId();
    snprintf(dirPath, sizeof(dirPath), "%sTask69_dir_%04u%02u%02u_%02u%02u%02u_%lu",
             tempDir,
             (unsigned)st.wYear, (unsigned)st.wMonth, (unsigned)st.wDay,
             (unsigned)st.wHour, (unsigned)st.wMinute, (unsigned)st.wSecond,
             (unsigned long)pid);

    if (!CreateDirectoryA(dirPath, NULL)) {
        // If it exists, try appending pid tick
        DWORD err = GetLastError();
        if (err != ERROR_ALREADY_EXISTS) return 0;
    }

    if (strlen(dirPath) + 1 > outSize) return 0;
    strcpy(outPath, dirPath);
    return 1;
#else
    const char* base = "/tmp/Task69_dir_XXXXXX";
    char pattern[256];
    strncpy(pattern, base, sizeof(pattern) - 1);
    pattern[sizeof(pattern) - 1] = '\0';

    // mkdtemp modifies pattern in place
    if (mkdtemp(pattern) == NULL) return 0;
    if (strlen(pattern) + 1 > outSize) return 0;
    strcpy(outPath, pattern);
    return 1;
#endif
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            int res = delete_file_secure(argv[i]);
            printf("%s -> %s\n", argv[i], res ? "deleted" : "failed");
        }
        return 0;
    }

    // Five test cases
    char path1[512];
    char path2[512];
    char dirpath[512];

    // Test 1: Create temp file and delete (expect pass)
    int ok1 = create_temp_file(path1, sizeof(path1));
    int t1 = 0;
    if (ok1) t1 = delete_file_secure(path1);
    printf("Test1 delete temp file: %s\n", (ok1 && t1) ? "PASS" : "FAIL");

    // Test 2: Delete same file again (expect fail)
    int t2 = delete_file_secure(path1);
    printf("Test2 delete non-existent file: %s\n", (!t2) ? "PASS" : "FAIL");

    // Test 3: Attempt to delete a directory (expect fail)
    int okDir = create_temp_dir(dirpath, sizeof(dirpath));
    int t3 = 0;
    if (okDir) t3 = delete_file_secure(dirpath);
    printf("Test3 delete directory: %s\n", (okDir && !t3) ? "PASS" : "FAIL");

    // Test 4: Empty path (expect fail)
    int t4 = delete_file_secure("   ");
    printf("Test4 empty path: %s\n", (!t4) ? "PASS" : "FAIL");

    // Test 5: Create another temp file and delete (expect pass)
    int ok2 = create_temp_file(path2, sizeof(path2));
    int t5 = 0;
    if (ok2) t5 = delete_file_secure(path2);
    printf("Test5 second temp file: %s\n", (ok2 && t5) ? "PASS" : "FAIL");

    // Cleanup: remove temp directory if still exists and empty
#ifdef _WIN32
    if (okDir) {
        RemoveDirectoryA(dirpath);
    }
#else
    if (okDir) {
        rmdir(dirpath);
    }
#endif

    return 0;
}