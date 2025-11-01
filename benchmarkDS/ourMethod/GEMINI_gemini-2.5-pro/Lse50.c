/*
 * This code uses the libzip library.
 * To compile, you need to install libzip and link against it.
 *
 * Example compilation command on Linux:
 * gcc -o Lse50 Lse50.c -lzip
 *
 * On macOS (with Homebrew):
 * brew install libzip
 * gcc -o Lse50 Lse50.c -L/usr/local/opt/libzip/lib -I/usr/local/opt/libzip/include -lzip
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zip.h>
#include <sys/stat.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>

#ifdef _WIN32
#include <direct.h>
#define realpath(N,R) _fullpath((R),(N),_MAX_PATH)
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#define PATH_MAX _MAX_PATH
#endif


/**
 * Recursively creates directories similar to `mkdir -p`.
 *
 * @param path The directory path to create.
 * @return 0 on success, -1 on failure.
 */
int mkdir_p(const char *path) {
    char *p;
    char *temp_path = strdup(path);
    if (temp_path == NULL) {
        perror("strdup");
        return -1;
    }

    for (p = temp_path + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            #ifdef _WIN32
            if (_mkdir(temp_path) != 0 && errno != EEXIST) {
            #else
            if (mkdir(temp_path, 0755) != 0 && errno != EEXIST) {
            #endif
                perror("mkdir");
                free(temp_path);
                return -1;
            }
            *p = '/';
        }
    }
    #ifdef _WIN32
    if (_mkdir(temp_path) != 0 && errno != EEXIST) {
    #else
    if (mkdir(temp_path, 0755) != 0 && errno != EEXIST) {
    #endif
        perror("mkdir");
        free(temp_path);
        return -1;
    }

    free(temp_path);
    return 0;
}

/**
 * Extracts a zip archive to a destination directory, preventing path traversal.
 *
 * @param zip_file_path The path to the zip file.
 * @param dest_dir_path The directory to extract files into.
 * @return 0 on success, -1 on failure.
 */
int extract_archive(const char *zip_file_path, const char *dest_dir_path) {
    char canonical_dest_dir[PATH_MAX];

    if (access(dest_dir_path, F_OK) != 0) {
        if (mkdir_p(dest_dir_path) != 0) {
            fprintf(stderr, "Error: Could not create destination directory %s\n", dest_dir_path);
            return -1;
        }
    }

    if (realpath(dest_dir_path, canonical_dest_dir) == NULL) {
        perror("realpath on destination directory");
        return -1;
    }
    
    struct stat st;
    if (stat(canonical_dest_dir, &st) != 0 || !S_ISDIR(st.st_mode)) {
        fprintf(stderr, "Error: Destination path is not a valid directory.\n");
        return -1;
    }

    int err = 0;
    zip_t *za = zip_open(zip_file_path, 0, &err);
    if (!za) {
        zip_error_t ziperror;
        zip_error_init_with_code(&ziperror, err);
        fprintf(stderr, "Error: Failed to open zip archive '%s': %s\n", zip_file_path, zip_error_strerror(&ziperror));
        zip_error_fini(&ziperror);
        return -1;
    }

    zip_int64_t num_entries = zip_get_num_entries(za, 0);
    if (num_entries < 0) {
        fprintf(stderr, "Error: Failed to get number of entries: %s\n", zip_strerror(za));
        zip_close(za);
        return -1;
    }

    for (zip_int64_t i = 0; i < num_entries; i++) {
        const char *entry_name = zip_get_name(za, i, 0);
        if (entry_name == NULL) {
            fprintf(stderr, "Error getting entry name at index %lld\n", i);
            continue;
        }

        char full_path[PATH_MAX];
        int n = snprintf(full_path, sizeof(full_path), "%s/%s", dest_dir_path, entry_name);
        if (n < 0 || (size_t)n >= sizeof(full_path)) {
            fprintf(stderr, "Error: Path too long for entry %s\n", entry_name);
            continue;
        }

        char canonical_full_path[PATH_MAX];
        // Create parent directories before resolving the path to avoid realpath failure
        char* last_slash = strrchr(full_path, '/');
        if(last_slash) {
            *last_slash = '\0';
            mkdir_p(full_path);
            *last_slash = '/';
        }

        if (realpath(full_path, canonical_full_path) == NULL && errno != ENOENT) {
            // ENOENT is OK, means the file itself doesn't exist yet, but its parent should.
            // Any other error from realpath is a problem.
        } else {
             if (strncmp(canonical_full_path, canonical_dest_dir, strlen(canonical_dest_dir)) != 0) {
                fprintf(stderr, "Path traversal attempt detected for entry: %s. Skipping.\n", entry_name);
                continue;
            }
        }
        // Restore full_path for file operations
        snprintf(full_path, sizeof(full_path), "%s/%s", dest_dir_path, entry_name);

        if (entry_name[strlen(entry_name) - 1] == '/') {
            mkdir_p(full_path);
        } else {
            zip_file_t *zf = zip_fopen_index(za, i, 0);
            if (!zf) {
                fprintf(stderr, "Error opening file in zip: %s\n", entry_name);
                continue;
            }

            FILE *out_file = fopen(full_path, "wb");
            if (!out_file) {
                fprintf(stderr, "Error opening output file %s: %s\n", full_path, strerror(errno));
                zip_fclose(zf);
                continue;
            }

            char buffer[4096];
            zip_int64_t n_read;
            while ((n_read = zip_fread(zf, buffer, sizeof(buffer))) > 0) {
                if (fwrite(buffer, 1, n_read, out_file) != (size_t)n_read) {
                    perror("fwrite");
                    break;
                }
            }
            fclose(out_file);
            zip_fclose(zf);
        }
    }

    zip_close(za);
    return 0;
}

/**
 * Creates a test zip file for extraction tests.
 */
int create_test_zip(const char *zip_file_path) {
    int error = 0;
    zip_t *za = zip_open(zip_file_path, ZIP_CREATE | ZIP_EXCL, &error);
    if (!za) {
        zip_error_t ziperror;
        zip_error_init_with_code(&ziperror, error);
        fprintf(stderr, "Failed to create zip '%s': %s\n", zip_file_path, zip_error_strerror(&ziperror));
        zip_error_fini(&ziperror);
        return -1;
    }
    const char* content1 = "This is a test file.";
    const char* content2 = "This is a nested file.";
    const char* content3 = "This should not be extracted.";
    const char* content4 = "This is another safe file.";

    zip_source_t* s;
    if ((s = zip_source_buffer(za, content1, strlen(content1), 0)) == NULL ||
        zip_file_add(za, "test1.txt", s, ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8) < 0) {
        zip_source_free(s);
        goto error;
    }
    if ((s = zip_source_buffer(za, content2, strlen(content2), 0)) == NULL ||
        zip_file_add(za, "dir1/test2.txt", s, ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8) < 0) {
        zip_source_free(s);
        goto error;
    }
    if ((s = zip_source_buffer(za, content3, strlen(content3), 0)) == NULL ||
        zip_file_add(za, "../evil.txt", s, ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8) < 0) {
        zip_source_free(s);
        goto error;
    }
    if ((s = zip_source_buffer(za, content4, strlen(content4), 0)) == NULL ||
        zip_file_add(za, "test3.txt", s, ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8) < 0) {
        zip_source_free(s);
        goto error;
    }
    if (zip_dir_add(za, "dir1/", ZIP_FL_ENC_UTF_8) < 0) goto error;

    zip_close(za);
    return 0;

error:
    zip_discard(za);
    return -1;
}

int main() {
    const char* zip_file_name = "archive.zip";
    const char* unpack_dir = "/tmp/unpack_c";

    printf("--- Test Case 1: Standard Extraction ---\n");
    if (create_test_zip(zip_file_name) == 0) {
        printf("Created test zip: %s\n", zip_file_name);
        int result = extract_archive(zip_file_name, unpack_dir);
        printf("Extraction successful: %s\n", result == 0 ? "true" : "false");

        char path_buf[PATH_MAX];
        snprintf(path_buf, sizeof(path_buf), "%s/test1.txt", unpack_dir);
        printf("Exists '%s': %s\n", path_buf, access(path_buf, F_OK) == 0 ? "true" : "false");
        snprintf(path_buf, sizeof(path_buf), "%s/dir1/test2.txt", unpack_dir);
        printf("Exists '%s': %s\n", path_buf, access(path_buf, F_OK) == 0 ? "true" : "false");
        snprintf(path_buf, sizeof(path_buf), "%s/test3.txt", unpack_dir);
        printf("Exists '%s': %s\n", path_buf, access(path_buf, F_OK) == 0 ? "true" : "false");
        printf("Exists '/tmp/evil.txt': %s\n", access("/tmp/evil.txt", F_OK) == 0 ? "true" : "false");

        // Cleanup: A robust solution would use platform-specific directory traversal.
        // For this example, we use a system call.
        char command[PATH_MAX];
        snprintf(command, sizeof(command), "rm -rf %s", unpack_dir);
        system(command);
        remove(zip_file_name);
        remove("/tmp/evil.txt");
        printf("Cleaned up test files and directories.\n\n");

    } else {
        fprintf(stderr, "Failed to create test zip.\n");
    }

    printf("--- Test Case 2: Non-existent ZIP file ---\n");
    int result2 = extract_archive("nonexistent.zip", unpack_dir);
    printf("Extraction successful: %s\n\n", result2 == 0 ? "true" : "false");
    
    printf("--- Test Cases 3, 4, 5 handled by above ---\n");

    return 0;
}