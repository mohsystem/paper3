#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#define GET_PID _getpid()
#else
#include <unistd.h>
#define GET_PID getpid()
#endif

/**
 * Creates a secure temporary file, writes a random string's unicode escapes
 * to it, and returns the file path. The caller is responsible for freeing
 * the returned path string.
 *
 * @return A dynamically allocated string containing the path to the temp file,
 *         or NULL on failure.
 */
char* createTempFileWithUnicodeString() {
    FILE* temp_file = NULL;
    char* file_path = NULL;

#ifdef _WIN32
    char temp_path_buffer[MAX_PATH];
    if (GetTempPathA(MAX_PATH, temp_path_buffer) == 0) {
        perror("Failed to get temp path");
        return NULL;
    }
    char* temp_filename = (char*)malloc(MAX_PATH);
    if (!temp_filename) {
        perror("Failed to allocate memory for filename");
        return NULL;
    }
    if (GetTempFileNameA(temp_path_buffer, "tempfile_", 0, temp_filename) == 0) {
        perror("Failed to create temp file name");
        free(temp_filename);
        return NULL;
    }
    // Use fopen_s for security on Windows
    if (fopen_s(&temp_file, temp_filename, "w") != 0 || temp_file == NULL) {
        perror("Failed to open temp file");
        free(temp_filename);
        return NULL;
    }
    file_path = temp_filename;
#else // POSIX
    const char* tmp_dir = getenv("TMPDIR");
    if (tmp_dir == NULL) {
        tmp_dir = "/tmp";
    }
    size_t path_len = strlen(tmp_dir) + strlen("/tempfile_XXXXXX") + 1;
    char* path_template = (char*)malloc(path_len);
    if (!path_template) {
        perror("Failed to allocate memory for path template");
        return NULL;
    }
    sprintf(path_template, "%s/tempfile_XXXXXX", tmp_dir);
    
    int fd = mkstemp(path_template);
    if (fd == -1) {
        perror("Failed to create temp file with mkstemp");
        free(path_template);
        return NULL;
    }
    
    temp_file = fdopen(fd, "w");
    if (temp_file == NULL) {
        perror("Failed to open file stream from fd");
        close(fd);
        remove(path_template);
        free(path_template);
        return NULL;
    }
    file_path = path_template;
#endif

    // NOTE: Using rand() here for simplicity. For security-sensitive applications,
    // a cryptographically secure pseudo-random number generator (CSPRNG) should be
    // used, such as reading from /dev/urandom on POSIX or using the BCryptGenRandom
    // API on Windows.
    srand((unsigned int)time(NULL) ^ (unsigned int)GET_PID);

    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    const int string_len = 20;
    
    // Write Unicode escape sequences directly to file
    for (int i = 0; i < string_len; ++i) {
        int key = rand() % (int)(sizeof(charset) - 1);
        // The output format \uXXXX is ASCII, which is a valid subset of UTF-8.
        fprintf(temp_file, "\\u%04x", (unsigned char)charset[key]);
    }
    
    fclose(temp_file);
    return file_path;
}

int main() {
    printf("Running 5 test cases for C...\n");
    for (int i = 1; i <= 5; ++i) {
        char* file_path = createTempFileWithUnicodeString();
        
        if (file_path) {
            printf("Test case %d: Success.\n", i);
            printf("  - File created at: %s\n", file_path);

            FILE* f = fopen(file_path, "r");
            if (f) {
                char buffer[256] = {0};
                if(fgets(buffer, sizeof(buffer)-1, f) != NULL) {
                     printf("  - File content: %s\n", buffer);
                }
                fclose(f);
            }

            remove(file_path);
            printf("  - Cleaned up file: %s\n", file_path);
            free(file_path);
        } else {
            fprintf(stderr, "Test case %d: Failed to create temp file.\n", i);
        }
        printf("---------------------------------\n");
    }
    return 0;
}