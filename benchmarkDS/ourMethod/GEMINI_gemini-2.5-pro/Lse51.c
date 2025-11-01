// Requires libarchive.
// Compile with: gcc your_file.c -larchive -o your_executable
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <errno.h>
#include <libgen.h>

#include <archive.h>
#include <archive_entry.h>

int copy_data(struct archive *ar, int fd) {
    const void *buff;
    size_t size;
    la_int64_t offset;
    int r;

    for (;;) {
        r = archive_read_data_block(ar, &buff, &size, &offset);
        if (r == ARCHIVE_EOF)
            return ARCHIVE_OK;
        if (r < ARCHIVE_OK)
            return r;

        if (write(fd, buff, size) != (ssize_t)size) {
            fprintf(stderr, "Error writing to file: %s\n", strerror(errno));
            return ARCHIVE_FATAL;
        }
    }
}

int create_parent_dirs(char *path) {
    char *p = strdup(path);
    if (p == NULL) {
        perror("strdup");
        return -1;
    }
    
    char *dir = dirname(p);
    struct stat st;
    if (stat(dir, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            free(p);
            return 0; // Directory already exists
        }
    }

    // Recursively create parent
    if (create_parent_dirs(dir) != 0) {
        free(p);
        return -1;
    }

    if (mkdir(dir, 0750) != 0 && errno != EEXIST) {
        perror("mkdir");
        free(p);
        return -1;
    }

    free(p);
    return 0;
}


int secure_extract_tar(const char* tar_path, const char* dest_dir) {
    char canon_dest_dir[PATH_MAX];
    if (realpath(dest_dir, canon_dest_dir) == NULL) {
        if (errno == ENOENT) {
            if (mkdir(dest_dir, 0750) != 0) {
                perror("Could not create destination directory");
                return -1;
            }
            if (realpath(dest_dir, canon_dest_dir) == NULL) {
                perror("Could not get canonical path for destination");
                return -1;
            }
        } else {
            perror("Could not get canonical path for destination");
            return -1;
        }
    }

    size_t canon_dest_dir_len = strlen(canon_dest_dir);
    struct archive *a = NULL;
    struct archive_entry *entry = NULL;
    int r = 0;
    int exit_code = 0;

    a = archive_read_new();
    if (a == NULL) {
        fprintf(stderr, "Failed to allocate archive struct\n");
        return -1;
    }
    
    archive_read_support_filter_all(a);
    archive_read_support_format_all(a);
    r = archive_read_open_filename(a, tar_path, 10240);
    if (r != ARCHIVE_OK) {
        fprintf(stderr, "Error opening archive: %s\n", archive_error_string(a));
        exit_code = -1;
        goto cleanup;
    }

    while ((r = archive_read_next_header(a, &entry)) == ARCHIVE_OK) {
        const char *pathname = archive_entry_pathname(entry);
        if (pathname == NULL) continue;
        
        // Security checks on the entry's path
        if (pathname[0] == '/' || strstr(pathname, "../") != NULL || strstr(pathname, "/..") != NULL) {
             fprintf(stderr, "Error: Attempted path traversal in tar file: %s\n", pathname);
             exit_code = -1;
             goto cleanup;
        }

        char full_dest_path[PATH_MAX];
        int len = snprintf(full_dest_path, sizeof(full_dest_path), "%s/%s", canon_dest_dir, pathname);
        if (len < 0 || (size_t)len >= sizeof(full_dest_path)) {
            fprintf(stderr, "Path is too long.\n");
            exit_code = -1;
            goto cleanup;
        }
        
        // Final security check: ensure the constructed path starts with our safe base path
        if (strncmp(full_dest_path, canon_dest_dir, canon_dest_dir_len) != 0) {
            fprintf(stderr, "Error: Final path is outside of destination directory: %s\n", full_dest_path);
            exit_code = -1;
            goto cleanup;
        }

        mode_t file_type = archive_entry_filetype(entry);
        if (S_ISDIR(file_type)) {
            if (mkdir(full_dest_path, 0750) != 0 && errno != EEXIST) {
                fprintf(stderr, "Error creating directory %s: %s\n", full_dest_path, strerror(errno));
                exit_code = -1;
                goto cleanup;
            }
        } else if (S_ISREG(file_type)) {
            char *path_copy = strdup(full_dest_path);
            if(path_copy == NULL) {
                perror("strdup");
                exit_code = -1;
                goto cleanup;
            }
            if (create_parent_dirs(path_copy) != 0) {
                free(path_copy);
                exit_code = -1;
                goto cleanup;
            }
            free(path_copy);

            int fd = open(full_dest_path, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
            if (fd < 0) {
                fprintf(stderr, "Error opening file %s for writing: %s\n", full_dest_path, strerror(errno));
                exit_code = -1;
                goto cleanup;
            }
            if (copy_data(a, fd) < ARCHIVE_OK) {
                fprintf(stderr, "Error writing file data for %s\n", full_dest_path);
                close(fd);
                exit_code = -1;
                goto cleanup;
            }
            close(fd);
        } else {
             printf("Skipping non-regular file/directory: %s\n", pathname);
        }
    }
    
    if (r != ARCHIVE_EOF) {
        fprintf(stderr, "Error reading archive: %s\n", archive_error_string(a));
        exit_code = -1;
    }

cleanup:
    if (a != NULL) archive_read_free(a);
    return exit_code;
}

void create_test_archives(const char* base_dir) {
    char command[1024];
    
    // Test 1: simple .tar.gz
    snprintf(command, sizeof(command), "mkdir -p %s/creation/dir1 && echo 'content1' > %s/creation/file1.txt && echo 'content2' > %s/creation/dir1/file2.txt", base_dir, base_dir, base_dir);
    system(command);
    snprintf(command, sizeof(command), "tar -czf %s/test1.tar.gz -C %s/creation .", base_dir, base_dir);
    system(command);
    snprintf(command, sizeof(command), "rm -rf %s/creation", base_dir);
    system(command);

    // Test 2: simple .tar.bz2
    snprintf(command, sizeof(command), "mkdir -p %s/creation && echo 'content3' > %s/creation/file3.txt", base_dir, base_dir);
    system(command);
    snprintf(command, sizeof(command), "tar -cjf %s/test2.tar.bz2 -C %s/creation .", base_dir, base_dir);
    system(command);
    snprintf(command, sizeof(command), "rm -rf %s/creation", base_dir);
    system(command);
    
    // Malicious tars
    snprintf(command, sizeof(command), "mkdir -p %s/creation && touch %s/creation/pwned.txt", base_dir, base_dir);
    system(command);
    snprintf(command, sizeof(command), "tar -czf %s/test3_traversal.tar.gz --transform 's|pwned.txt|../../../pwned.txt|' -C %s/creation pwned.txt", base_dir, base_dir);
    system(command);
    snprintf(command, sizeof(command), "tar -czf %s/test4_absolute.tar.gz --transform 's|pwned.txt|/tmp/absolute_pwned.txt|' -C %s/creation pwned.txt", base_dir, base_dir);
    system(command);
    snprintf(command, sizeof(command), "ln -s file1.txt %s/creation/link_to_file1 && tar -czf %s/test5_symlink.tar.gz -C %s/creation link_to_file1", base_dir, base_dir, base_dir);
    system(command);
    snprintf(command, sizeof(command), "rm -rf %s/creation", base_dir);
    system(command);
}

int main() {
    char archives_dir_template[] = "/tmp/c_tar_tests_XXXXXX";
    char* archives_dir = mkdtemp(archives_dir_template);
    if (!archives_dir) {
        perror("mkdtemp archives"); return 1;
    }
    printf("Setting up test archives in: %s\n", archives_dir);
    create_test_archives(archives_dir);
    
    char unpack_dir_template[] = "/tmp/c_unpack_XXXXXX";
    char file_path[PATH_MAX];
    struct stat st;

    // Test Case 1
    char* unpack_dir1 = mkdtemp(strdup(unpack_dir_template));
    printf("\n--- Test Case 1: Simple .tar.gz ---\n");
    snprintf(file_path, sizeof(file_path), "%s/test1.tar.gz", archives_dir);
    int success1 = secure_extract_tar(file_path, unpack_dir1) == 0;
    printf("Extraction success: %s\n", success1 ? "true" : "false");
    snprintf(file_path, sizeof(file_path), "%s/file1.txt", unpack_dir1);
    printf("Check file1.txt exists: %s\n", stat(file_path, &st) == 0 ? "true" : "false");
    snprintf(file_path, sizeof(file_path), "%s/dir1/file2.txt", unpack_dir1);
    printf("Check dir1/file2.txt exists: %s\n", stat(file_path, &st) == 0 ? "true" : "false");
    
    // Test Case 2
    char* unpack_dir2 = mkdtemp(strdup(unpack_dir_template));
    printf("\n--- Test Case 2: Simple .tar.bz2 ---\n");
    snprintf(file_path, sizeof(file_path), "%s/test2.tar.bz2", archives_dir);
    int success2 = secure_extract_tar(file_path, unpack_dir2) == 0;
    printf("Extraction success: %s\n", success2 ? "true" : "false");
    snprintf(file_path, sizeof(file_path), "%s/file3.txt", unpack_dir2);
    printf("Check file3.txt exists: %s\n", stat(file_path, &st) == 0 ? "true" : "false");

    // Test Case 3
    char* unpack_dir3 = mkdtemp(strdup(unpack_dir_template));
    printf("\n--- Test Case 3: Path Traversal (should fail) ---\n");
    snprintf(file_path, sizeof(file_path), "%s/test3_traversal.tar.gz", archives_dir);
    int success3 = secure_extract_tar(file_path, unpack_dir3) == 0;
    printf("Extraction success: %s\n", success3 ? "true" : "false");
    printf("Check for traversal file exists: %s\n", stat("/tmp/pwned.txt", &st) == 0 ? "true" : "false");
    
    // Test Case 4
    char* unpack_dir4 = mkdtemp(strdup(unpack_dir_template));
    printf("\n--- Test Case 4: Absolute Path (should fail) ---\n");
    snprintf(file_path, sizeof(file_path), "%s/test4_absolute.tar.gz", archives_dir);
    int success4 = secure_extract_tar(file_path, unpack_dir4) == 0;
    printf("Extraction success: %s\n", success4 ? "true" : "false");
    printf("Check for absolute file exists: %s\n", stat("/tmp/absolute_pwned.txt", &st) == 0 ? "true" : "false");
    
    // Test Case 5
    char* unpack_dir5 = mkdtemp(strdup(unpack_dir_template));
    printf("\n--- Test Case 5: Symlink (should be skipped) ---\n");
    snprintf(file_path, sizeof(file_path), "%s/test5_symlink.tar.gz", archives_dir);
    int success5 = secure_extract_tar(file_path, unpack_dir5) == 0;
    printf("Extraction success: %s\n", success5 ? "true" : "false");
    snprintf(file_path, sizeof(file_path), "%s/link_to_file1", unpack_dir5);
    printf("Check symlink exists: %s\n", lstat(file_path, &st) == 0 ? "true" : "false");

    // Cleanup
    snprintf(file_path, sizeof(file_path), "rm -rf %s %s %s %s %s %s /tmp/absolute_pwned.txt /tmp/pwned.txt",
        archives_dir, unpack_dir1, unpack_dir2, unpack_dir3, unpack_dir4, unpack_dir5);
    system(file_path);
    free(unpack_dir1); free(unpack_dir2); free(unpack_dir3); free(unpack_dir4); free(unpack_dir5);
    printf("\nCleanup complete.\n");

    return 0;
}