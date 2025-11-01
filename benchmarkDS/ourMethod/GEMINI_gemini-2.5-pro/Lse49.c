// To compile: gcc main.c -lzip -o unzip_app_c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zip.h>
#include <sys/stat.h>
#include <errno.h>
#include <libgen.h>
#include <limits.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path, mode) _mkdir(path)
#else
#define MKDIR(path, mode) mkdir(path, mode)
#endif

int create_dir_recursive(char *path) {
    char *p = path;
    if (p[0] == '/') p++;
    while ((p = strchr(p, '/'))) {
        *p = '\0';
        if (MKDIR(path, 0755) && errno != EEXIST) {
            perror("mkdir");
            *p = '/';
            return -1;
        }
        *p = '/';
        p++;
    }
    return 0;
}

int unzip_archive(const char* archive_path, const char* extract_dir) {
    char canon_extract_dir[PATH_MAX];

    struct stat st;
    if (stat(archive_path, &st) != 0 || !S_ISREG(st.st_mode)) {
        fprintf(stderr, "Error: Archive file not found or is not a regular file: %s\n", archive_path);
        return -1;
    }

    if (stat(extract_dir, &st) != 0) {
        if (MKDIR(extract_dir, 0755) != 0) {
            fprintf(stderr, "Error: Could not create directory %s\n", extract_dir);
            return -1;
        }
    }

    #ifdef _WIN32
    if (!_fullpath(canon_extract_dir, extract_dir, PATH_MAX)) {
         fprintf(stderr, "Error: Could not resolve path for %s\n", extract_dir);
         return -1;
    }
    #else
    if (realpath(extract_dir, canon_extract_dir) == NULL) {
        fprintf(stderr, "Error: Could not resolve path for %s\n", extract_dir);
        return -1;
    }
    #endif

    int err = 0;
    zip_t* za = zip_open(archive_path, 0, &err);
    if (!za) {
        fprintf(stderr, "Error: Failed to open archive %s\n", archive_path);
        return -1;
    }

    zip_int64_t num_entries = zip_get_num_entries(za, 0);
    for (zip_int64_t i = 0; i < num_entries; i++) {
        const char* name = zip_get_name(za, i, 0);
        if (name == NULL) {
            fprintf(stderr, "Error: Failed to get name for entry %lld\n", i);
            continue;
        }

        // Security check for path traversal
        if (strstr(name, "..") != NULL || name[0] == '/') {
            fprintf(stderr, "Error: Malicious path detected in archive: %s\n", name);
            zip_close(za);
            return -1;
        }

        char dest_path[PATH_MAX];
        snprintf(dest_path, sizeof(dest_path), "%s/%s", canon_extract_dir, name);
        
        // This is a simplified check. On systems with symlinks, a more
        // robust check would be needed after resolving the real path.
        if (strncmp(dest_path, canon_extract_dir, strlen(canon_extract_dir)) != 0) {
            fprintf(stderr, "Error: Path traversal attempt detected for entry '%s'\n", name);
            zip_close(za);
            return -1;
        }

        if (name[strlen(name) - 1] == '/') {
            MKDIR(dest_path, 0755);
        } else {
            char* dest_path_copy = strdup(dest_path);
            if (dest_path_copy) {
                create_dir_recursive(dirname(dest_path_copy));
                free(dest_path_copy);
            }
            
            zip_file_t* zf = zip_fopen_index(za, i, 0);
            if (!zf) {
                fprintf(stderr, "Error opening file in archive: %s\n", name);
                continue;
            }

            FILE* of = fopen(dest_path, "wb");
            if (!of) {
                fprintf(stderr, "Error opening output file: %s\n", dest_path);
                zip_fclose(zf);
                continue;
            }

            char buffer[4096];
            zip_int64_t n;
            while ((n = zip_fread(zf, buffer, sizeof(buffer))) > 0) {
                fwrite(buffer, 1, n, of);
            }
            
            fclose(of);
            zip_fclose(zf);
        }
    }

    zip_close(za);
    return 0;
}

int create_test_zip(const char* zip_path, const char* names[], const char* contents[], int count, int is_malicious) {
    int error = 0;
    zip_t* za = zip_open(zip_path, ZIP_CREATE | ZIP_TRUNCATE, &error);
    if (!za) return -1;

    for (int i = 0; i < count; i++) {
        char name_buffer[256];
        strncpy(name_buffer, names[i], sizeof(name_buffer) - 1);
        name_buffer[sizeof(name_buffer) - 1] = '\0';

        if (is_malicious && i == count - 1) {
            snprintf(name_buffer, sizeof(name_buffer), "../%s", names[i]);
        }
        
        size_t content_len = contents[i] ? strlen(contents[i]) : 0;
        zip_source_t* source = zip_source_buffer(za, contents[i], content_len, 0);
        if (!source) { zip_close(za); return -1; }

        if (zip_file_add(za, name_buffer, source, ZIP_FL_OVERWRITE) < 0) {
            zip_source_free(source);
            zip_close(za);
            return -1;
        }
    }
    zip_close(za);
    return 0;
}


int main() {
    const char* base_tmp_dir = getenv("TMPDIR");
    if (!base_tmp_dir) base_tmp_dir = "/tmp";

    char archive_path[PATH_MAX];
    char unpack_dir[PATH_MAX];

    // Test Case 1: Valid archive
    printf("--- Test Case 1: Valid Archive ---\n");
    snprintf(archive_path, PATH_MAX, "%s/archive1.zip", base_tmp_dir);
    snprintf(unpack_dir, PATH_MAX, "%s/unpack1", base_tmp_dir);
    const char* names1[] = {"file1.txt", "dir1/file2.txt"};
    const char* contents1[] = {"content1", "content2"};
    create_test_zip(archive_path, names1, contents1, 2, 0);
    printf("Unzipping %s to %s\n", archive_path, unpack_dir);
    if (unzip_archive(archive_path, unpack_dir) == 0) {
        printf("Test Case 1: Success.\n");
    } else {
        fprintf(stderr, "Test Case 1: Failed.\n");
    }
    // Cleanup would involve recursively deleting unpack_dir and removing archive_path.
    
    // Test Case 2: Malicious archive
    printf("\n--- Test Case 2: Malicious Archive (Path Traversal) ---\n");
    snprintf(archive_path, PATH_MAX, "%s/malicious.zip", base_tmp_dir);
    snprintf(unpack_dir, PATH_MAX, "%s/unpack2", base_tmp_dir);
    const char* names2[] = {"good.txt", "evil.txt"};
    const char* contents2[] = {"good", "evil"};
    create_test_zip(archive_path, names2, contents2, 2, 1);
    printf("Unzipping %s to %s\n", archive_path, unpack_dir);
    if (unzip_archive(archive_path, unpack_dir) != 0) {
        printf("Test Case 2: Success (correctly identified traversal).\n");
    } else {
        fprintf(stderr, "Test Case 2: Failed (did not block traversal).\n");
    }

    // Test Case 3: Non-existent archive
    printf("\n--- Test Case 3: Non-existent Archive ---\n");
    snprintf(archive_path, PATH_MAX, "%s/nonexistent.zip", base_tmp_dir);
    snprintf(unpack_dir, PATH_MAX, "%s/unpack3", base_tmp_dir);
    printf("Unzipping %s to %s\n", archive_path, unpack_dir);
    if (unzip_archive(archive_path, unpack_dir) != 0) {
        printf("Test Case 3: Success (correctly handled error).\n");
    } else {
        fprintf(stderr, "Test Case 3: Failed.\n");
    }

    // Test Case 4: No permission directory
    printf("\n--- Test Case 4: No Permission Directory ---\n");
    snprintf(archive_path, PATH_MAX, "%s/archive4.zip", base_tmp_dir);
    const char* names4[] = {"file1.txt"};
    const char* contents4[] = {"content"};
    create_test_zip(archive_path, names4, contents4, 1, 0);
    printf("Unzipping %s to /root/unpack4\n", archive_path);
    if (unzip_archive(archive_path, "/root/unpack4") != 0) {
        printf("Test Case 4: Success (correctly handled permission error).\n");
    } else {
        fprintf(stderr, "Test Case 4: Failed or was skipped.\n");
    }

    // Test Case 5: Empty archive
    printf("\n--- Test Case 5: Empty Archive ---\n");
    snprintf(archive_path, PATH_MAX, "%s/empty.zip", base_tmp_dir);
    snprintf(unpack_dir, PATH_MAX, "%s/unpack5", base_tmp_dir);
    create_test_zip(archive_path, NULL, NULL, 0, 0);
    printf("Unzipping %s to %s\n", archive_path, unpack_dir);
    if (unzip_archive(archive_path, unpack_dir) == 0) {
        printf("Test Case 5: Success.\n");
    } else {
        fprintf(stderr, "Test Case 5: Failed.\n");
    }

    return 0;
}