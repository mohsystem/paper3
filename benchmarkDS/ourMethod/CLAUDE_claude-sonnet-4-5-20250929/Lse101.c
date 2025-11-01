
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <pwd.h>
#endif

#define MAX_PATH_LEN 4096

/* Security: Get secure base directory path */
int getBaseDirectory(char* buffer, size_t bufferSize) {
    if (buffer == NULL || bufferSize == 0) return -1;
    
#ifdef _WIN32
    char home[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, home))) {
        int result = snprintf(buffer, bufferSize, "%s\\\\app_data", home);
        if (result < 0 || (size_t)result >= bufferSize) return -1;
        return 0;
    }
    return -1;
#else
    const char* home = getenv("HOME");
    if (home == NULL) {
        struct passwd* pw = getpwuid(getuid());
        if (pw != NULL) home = pw->pw_dir;
    }
    if (home == NULL) return -1;
    
    int result = snprintf(buffer, bufferSize, "%s/app_data", home);
    if (result < 0 || (size_t)result >= bufferSize) return -1;
    return 0;
#endif
}

/* Security: Create directory with restrictive permissions */
int createSecureDirectory(const char* path) {
    if (path == NULL) return -1;
    
#ifdef _WIN32
    if (!CreateDirectoryA(path, NULL)) {
        if (GetLastError() != ERROR_ALREADY_EXISTS) return -1;
    }
    return 0;
#else
    /* Security: Create with owner-only permissions (0700) */
    if (mkdir(path, S_IRWXU) != 0) {
        if (errno != EEXIST) return -1;
    }
    return 0;
#endif
}

/* Security: Validate filename to prevent path traversal */
int isValidFilename(const char* filename) {
    if (filename == NULL || filename[0] == '\\0') return 0;
    
    /* Security: Reject path traversal patterns */
    if (strstr(filename, "..") != NULL) return 0;
    if (strchr(filename, '/') != NULL) return 0;
    if (strchr(filename, '\\\\') != NULL) return 0;
    
    return 1;
}

/* Security: Write file with atomic operation and restrictive permissions */
int writeConfig(const char* filename, const char* content) {
    char baseDir[MAX_PATH_LEN];
    char targetPath[MAX_PATH_LEN];
    char tempPath[MAX_PATH_LEN];
    int result = -1;
    
    /* Security: Input validation */
    if (filename == NULL || content == NULL) {
        fprintf(stderr, "Invalid input: filename or content is NULL\\n");
        return -1;
    }
    
    if (!isValidFilename(filename)) {
        fprintf(stderr, "Invalid filename or path traversal attempt\\n");
        return -1;
    }
    
    /* Security: Get and create base directory */
    if (getBaseDirectory(baseDir, sizeof(baseDir)) != 0) {
        fprintf(stderr, "Failed to get base directory\\n");
        return -1;
    }
    
    if (createSecureDirectory(baseDir) != 0) {
        fprintf(stderr, "Failed to create base directory\\n");
        return -1;
    }
    
    /* Security: Build target path with bounds checking */
    result = snprintf(targetPath, sizeof(targetPath), "%s/%s", baseDir, filename);
    if (result < 0 || (size_t)result >= sizeof(targetPath)) {
        fprintf(stderr, "Path too long\\n");
        return -1;
    }
    
#ifdef _WIN32
    char tempFile[MAX_PATH];
    if (GetTempFileNameA(baseDir, "tmp", 0, tempFile) == 0) {
        fprintf(stderr, "Failed to create temporary file\\n");
        return -1;
    }
    
    /* Security: Write to temp file */
    HANDLE hFile = CreateFileA(tempFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Failed to open temp file\\n");
        return -1;
    }
    
    DWORD written;
    DWORD contentLen = (DWORD)strlen(content);
    if (!WriteFile(hFile, content, contentLen, &written, NULL) || written != contentLen) {
        CloseHandle(hFile);
        DeleteFileA(tempFile);
        fprintf(stderr, "Failed to write content\\n");
        return -1;
    }
    
    /* Security: Flush to disk */
    FlushFileBuffers(hFile);
    CloseHandle(hFile);
    
    /* Security: Atomic move */
    if (!MoveFileExA(tempFile, targetPath, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        DeleteFileA(tempFile);
        fprintf(stderr, "Failed to move temp file\\n");
        return -1;
    }
#else
    /* Security: Create temp file with mkstemp */
    result = snprintf(tempPath, sizeof(tempPath), "%s/.tmp_XXXXXX", baseDir);
    if (result < 0 || (size_t)result >= sizeof(tempPath)) {
        fprintf(stderr, "Temp path too long\\n");
        return -1;
    }
    
    /* Security: Create with owner-only permissions (0600) */
    int fd = mkstemp(tempPath);
    if (fd < 0) {
        fprintf(stderr, "Failed to create temp file: %s\\n", strerror(errno));
        return -1;
    }
    
    /* Security: Set restrictive permissions */
    if (fchmod(fd, S_IRUSR | S_IWUSR) != 0) {
        close(fd);
        unlink(tempPath);
        fprintf(stderr, "Failed to set permissions\\n");
        return -1;
    }
    
    /* Security: Write with bounds checking */
    size_t contentLen = strlen(content);
    ssize_t written = write(fd, content, contentLen);
    if (written < 0 || (size_t)written != contentLen) {
        close(fd);
        unlink(tempPath);
        fprintf(stderr, "Failed to write content\\n");
        return -1;
    }
    
    /* Security: Ensure data is written to disk */
    if (fsync(fd) != 0) {
        close(fd);
        unlink(tempPath);
        fprintf(stderr, "Failed to sync file\\n");
        return -1;
    }
    
    close(fd);
    
    /* Security: Atomic rename */
    if (rename(tempPath, targetPath) != 0) {
        unlink(tempPath);
        fprintf(stderr, "Failed to rename temp file: %s\\n", strerror(errno));
        return -1;
    }
#endif
    
    return 0;
}

int main(void) {
    /* Test case 1: Normal write operation */
    if (writeConfig("important_config", "important_config") == 0) {
        printf("Test 1 passed: File written successfully\\n");
    } else {
        fprintf(stderr, "Test 1 failed\\n");
    }
    
    /* Test case 2: Write with different content */
    if (writeConfig("important_config", "test_content_123") == 0) {
        printf("Test 2 passed: File updated successfully\\n");
    } else {
        fprintf(stderr, "Test 2 failed\\n");
    }
    
    /* Test case 3: Invalid filename (NULL) */
    if (writeConfig(NULL, "content") != 0) {
        printf("Test 3 passed: NULL filename rejected\\n");
    } else {
        fprintf(stderr, "Test 3 failed: Should have rejected NULL\\n");
    }
    
    /* Test case 4: Path traversal attempt */
    if (writeConfig("../../../etc/passwd", "malicious") != 0) {
        printf("Test 4 passed: Path traversal blocked\\n");
    } else {
        fprintf(stderr, "Test 4 failed: Path traversal not blocked\\n");
    }
    
    /* Test case 5: Empty content */
    if (writeConfig("empty_config", "") == 0) {
        printf("Test 5 passed: Empty content written\\n");
    } else {
        fprintf(stderr, "Test 5 failed\\n");
    }
    
    return 0;
}
