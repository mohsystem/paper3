// Compile with: gcc this_file.c -o Lse49 -lzip
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <libgen.h> // for dirname
#include <limits.h> // for PATH_MAX

#if defined(_WIN32) || defined(_WIN64)
#include <direct.h>
#define MKDIR(path, mode) _mkdir(path)
#else
#define MKDIR(path, mode) mkdir(path, mode)
#endif

#include <zip.h>

/**
 * The prompt requests to "Open the archive.zip file as a tarfile".
 * This is technically incorrect as .zip and .tar are different archive formats.
 * This function correctly processes a .zip file using the libzip library.
 * It requires libzip to be installed (e.g., 'sudo apt-get install libzip-dev' on Debian/Ubuntu).
 */

// Recursively creates directories.
static int make_dir_recursive(const char *path) {
    char *sub_path = strdup(path);
    if (sub_path == NULL) {
        perror("strdup");
        return -1;
    }
    size_t len = strlen(sub_path);
    if (sub_path[len - 1] == '/') {
        sub_path[len - 1] = 0;
    }

    for (char *p = sub_path + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            if (MKDIR(sub_path, S_IRWXU) != 0 && errno != EEXIST) {
                perror("mkdir");
                free(sub_path);
                return -1;
            }
            *p = '/';
        }
    }
    if (MKDIR(sub_path, S_IRWXU) != 0 && errno != EEXIST) {
        perror("mkdir");
        free(sub_path);
        return -1;
    }
    free(sub_path);
    return 0;
}

int unzip_archive(const char* zip_path, const char* dest_dir) {
    int err = 0;
    zip_t *za = zip_open(zip_path, 0, &err);
    if (za == NULL) {
        zip_error_t ziperror;
        zip_error_init_with_code(&ziperror, err);
        fprintf(stderr, "Error opening zip file %s: %s\n", zip_path, zip_error_strerror(&ziperror));
        zip_error_fini(&ziperror);
        return -1;
    }

    if (MKDIR(dest_dir, S_IRWXU) != 0 && errno != EEXIST) {
        fprintf(stderr, "Failed to create destination directory %s\n", dest_dir);
        zip_close(za);
        return -1;
    }

    zip_int64_t num_entries = zip_get_num_entries(za, 0);
    for (zip_int64_t i = 0; i < num_entries; i++) {
        const char *name = zip_get_name(za, i, 0);
        if (name == NULL) {
            fprintf(stderr, "Error reading entry name at index %lld\n", i);
            continue;
        }

        // SECURITY CHECK: Prevent Path Traversal (Zip Slip) by disallowing '..' components and absolute paths.
        if (strstr(name, "..") != NULL) {
            fprintf(stderr, "Security Error: Path contains '..' component: %s. Skipping.\n", name);
            continue;
        }
        if (name[0] == '/') {
            fprintf(stderr, "Security Error: Path is absolute: %s. Skipping.\n", name);
            continue;
        }

        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s", dest_dir, name);

        if (name[strlen(name) - 1] == '/') {
            make_dir_recursive(full_path);
        } else {
            zip_stat_t zs;
            if (zip_stat_index(za, i, 0, &zs) != 0) {
                fprintf(stderr, "Failed to stat entry %s\n", name);
                continue;
            }
            zip_file_t *zf = zip_fopen_index(za, i, 0);
            if (!zf) {
                fprintf(stderr, "Error opening file in zip: %s\n", name);
                continue;
            }

            char *path_copy = strdup(full_path);
            if (path_copy) {
                make_dir_recursive(dirname(path_copy));
                free(path_copy);
            }

            FILE *fp = fopen(full_path, "wb");
            if (!fp) {
                fprintf(stderr, "Error creating output file: %s\n", full_path);
                zip_fclose(zf);
                continue;
            }

            char buffer[4096];
            zip_int64_t n;
            while ((n = zip_fread(zf, buffer, sizeof(buffer))) > 0) {
                fwrite(buffer, 1, n, fp);
            }
            fclose(fp);
            zip_fclose(zf);
        }
    }

    zip_close(za);
    return 0;
}

void Lse49_runTests() {
    printf("Running C Unzip Test Cases...\n");
    printf("NOTE: This requires manually created .zip files for testing.\n");

    const char* base_test_dir = "/tmp/c_unzip_tests_lse49";
    MKDIR(base_test_dir, S_IRWXU);
    printf("Test files should be placed in: %s\n", base_test_dir);
    printf("\nSee C++ test case output for instructions on creating test files.\n");

    // Test Case 1
    printf("\n--- Test Case 1: Simple zip ---\n");
    char zip1[PATH_MAX], unpack1[PATH_MAX];
    snprintf(zip1, sizeof(zip1), "%s/archive1.zip", base_test_dir);
    snprintf(unpack1, sizeof(unpack1), "%s/unpack1", base_test_dir);
    if (unzip_archive(zip1, unpack1) == 0) {
        printf("Unzip may be successful. Verify: %s/file1.txt\n", unpack1);
    }

    // Test Case 2
    printf("\n--- Test Case 2: Zip with subdirectory ---\n");
    char zip2[PATH_MAX], unpack2[PATH_MAX];
    snprintf(zip2, sizeof(zip2), "%s/archive2.zip", base_test_dir);
    snprintf(unpack2, sizeof(unpack2), "%s/unpack2", base_test_dir);
    if (unzip_archive(zip2, unpack2) == 0) {
        printf("Unzip may be successful. Verify: %s/dir1/file2.txt\n", unpack2);
    }
    
    // Test Case 3
    printf("\n--- Test Case 3: Zip with empty directory ---\n");
    char zip3[PATH_MAX], unpack3[PATH_MAX];
    snprintf(zip3, sizeof(zip3), "%s/archive3.zip", base_test_dir);
    snprintf(unpack3, sizeof(unpack3), "%s/unpack3", base_test_dir);
    if (unzip_archive(zip3, unpack3) == 0) {
        printf("Unzip may be successful. Verify directory: %s/emptydir\n", unpack3);
    }
    
    // Test Case 4
    printf("\n--- Test Case 4: Unzip to non-existent directory ---\n");
    char zip4[PATH_MAX], unpack4[PATH_MAX];
    snprintf(zip4, sizeof(zip4), "%s/archive4.zip", base_test_dir);
    snprintf(unpack4, sizeof(unpack4), "%s/unpack4", base_test_dir);
    if (unzip_archive(zip4, unpack4) == 0) {
        printf("Unzip may be successful. Verify: %s/test.txt\n", unpack4);
    }
    
    // Test Case 5
    printf("\n--- Test Case 5: Unzip empty zip ---\n");
    char zip5[PATH_MAX], unpack5[PATH_MAX];
    snprintf(zip5, sizeof(zip5), "%s/archive5.zip", base_test_dir);
    snprintf(unpack5, sizeof(unpack5), "%s/unpack5", base_test_dir);
    if (unzip_archive(zip5, unpack5) == 0) {
        printf("Unzip may be successful. Verify directory '%s' is empty.\n", unpack5);
    }

    printf("\nC tests complete. Please manually clean up %s\n", base_test_dir);
}

int main() {
    // The prompt requests a class name Lse49. In C, a function prefix is used.
    Lse49_runTests();
    return 0;
}