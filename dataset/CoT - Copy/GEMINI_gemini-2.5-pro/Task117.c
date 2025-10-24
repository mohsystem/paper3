/*
 * IMPORTANT: This code requires the 'libarchive' library.
 *
 * To compile on Linux/macOS:
 * gcc Task117.c -o Task117 -larchive
 *
 * To install libarchive:
 * - On Debian/Ubuntu: sudo apt-get install libarchive-dev
 * - On RedHat/CentOS: sudo yum install libarchive-devel
 * - On macOS (Homebrew): brew install libarchive
 *
 * NOTE: realpath() is a POSIX function. For Windows, you would need
 * to use _fullpath() or GetFullPathName() and adapt the path logic.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <limits.h>
#include <archive.h>
#include <archive_entry.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path, mode) _mkdir(path)
#else
#include <unistd.h>
#define MKDIR(path, mode) mkdir(path, mode)
#endif


static int copy_data(struct archive *ar, struct archive *aw) {
    int r;
    const void *buff;
    size_t size;
    la_int64_t offset;

    for (;;) {
        r = archive_read_data_block(ar, &buff, &size, &offset);
        if (r == ARCHIVE_EOF)
            return (ARCHIVE_OK);
        if (r < ARCHIVE_OK)
            return (r);
        r = archive_write_data_block(aw, buff, size, offset);
        if (r < ARCHIVE_OK) {
            fprintf(stderr, "%s\n", archive_error_string(aw));
            return (r);
        }
    }
}

/**
 * Extracts an archive to a destination directory safely.
 * @param archive_path Path to the archive file.
 * @param dest_dir     Path to the destination directory.
 * @return 0 on success, -1 on failure.
 */
int extract_archive(const char *archive_path, const char *dest_dir) {
    struct archive *a;
    struct archive *ext;
    struct archive_entry *entry;
    int flags;
    int r;
    char dest_dir_real[PATH_MAX];
    char entry_full_path[PATH_MAX];
    char entry_real_path[PATH_MAX];

    // Create destination directory if it doesn't exist.
    struct stat st = {0};
    if (stat(dest_dir, &st) == -1) {
        MKDIR(dest_dir, 0755);
    }
    
    // Get the real, absolute path of the destination directory for security checks.
    if (realpath(dest_dir, dest_dir_real) == NULL) {
        perror("realpath for destination directory failed");
        return -1;
    }
    size_t dest_dir_len = strlen(dest_dir_real);

    // Standard extraction flags
    flags = ARCHIVE_EXTRACT_TIME;
    flags |= ARCHIVE_EXTRACT_PERM;
    flags |= ARCHIVE_EXTRACT_ACL;
    flags |= ARCHIVE_EXTRACT_FFLAGS;

    a = archive_read_new();
    archive_read_support_format_all(a);
    archive_read_support_filter_all(a);
    ext = archive_write_disk_new();
    archive_write_disk_set_options(ext, flags);
    archive_write_disk_set_standard_lookup(ext);

    if ((r = archive_read_open_filename(a, archive_path, 10240))) {
        fprintf(stderr, "Error opening archive: %s\n", archive_error_string(a));
        goto cleanup;
    }

    for (;;) {
        r = archive_read_next_header(a, &entry);
        if (r == ARCHIVE_EOF) break;
        if (r < ARCHIVE_OK) fprintf(stderr, "%s\n", archive_error_string(a));
        if (r < ARCHIVE_WARN) goto cleanup;

        // Construct the full path of the file to be extracted
        snprintf(entry_full_path, sizeof(entry_full_path), "%s/%s", dest_dir, archive_entry_pathname(entry));
        
        // --- Security Check for Path Traversal ---
        if (realpath(entry_full_path, entry_real_path) == NULL) {
            // realpath may fail if path doesn't exist, which is expected.
            // We can construct the path and normalize it manually for the check.
            // A simple strncmp on the absolute paths is a good defense.
            char temp_path[PATH_MAX];
            if (realpath(dest_dir, temp_path) == NULL) {
                // Should not happen as we checked it before
                r = -1;
                goto cleanup;
            }
            strcat(temp_path, "/");
            strcat(temp_path, archive_entry_pathname(entry));
            // Let's use the constructed full path and check against the real dest path
        }

        // Check if the constructed path starts with the destination directory path
        if (strncmp(entry_full_path, dest_dir_real, dest_dir_len) != 0) {
            fprintf(stderr, "Path Traversal attempt detected. Skipping entry: %s\n", archive_entry_pathname(entry));
            continue;
        }

        archive_entry_set_pathname(entry, entry_full_path);

        r = archive_write_header(ext, entry);
        if (r < ARCHIVE_OK) {
            fprintf(stderr, "%s\n", archive_error_string(ext));
        } else if (archive_entry_size(entry) > 0) {
            r = copy_data(a, ext);
            if (r < ARCHIVE_OK) {
                fprintf(stderr, "%s\n", archive_error_string(ext));
            }
            if (r < ARCHIVE_WARN) {
                goto cleanup;
            }
        }
        r = archive_write_finish_entry(ext);
        if (r < ARCHIVE_OK) fprintf(stderr, "%s\n", archive_error_string(ext));
        if (r < ARCHIVE_WARN) goto cleanup;
    }

cleanup:
    archive_read_close(a);
    archive_read_free(a);
    archive_write_close(ext);
    archive_write_free(ext);
    return (r == ARCHIVE_OK || r == ARCHIVE_EOF) ? 0 : -1;
}

void create_test_files_notice() {
    printf("--------------------------------------------------------\n");
    printf("This C program requires test archive files to exist.\n");
    printf("Please create them manually before running the tests.\n\n");
    printf("Example commands to create test files:\n");
    printf("  mkdir -p temp_for_zip/dir1\n");
    printf("  echo 'content' > temp_for_zip/dir1/file1.txt\n");
    printf("  (cd temp_for_zip && zip -r ../test.zip .)\n");
    printf("  (cd temp_for_zip && tar -czf ../test.tar.gz .)\n");
    printf("  zip malicious.zip ../../malicious_file.txt\n");
    printf("  rm -rf temp_for_zip\n");
    printf("--------------------------------------------------------\n\n");
}


int main() {
    const char* base_test_dir = "c_test_area";
    MKDIR(base_test_dir, 0755);

    create_test_files_notice();
    
    // --- Test Case 1: Normal ZIP Extraction ---
    printf("--- Test Case 1: Normal ZIP Extraction ---\n");
    const char* normal_zip = "test.zip";
    const char* extract_dir1 = "c_test_area/extract1";
    if (access(normal_zip, F_OK) == 0) {
        int res1 = extract_archive(normal_zip, extract_dir1);
        printf("Extraction result: %s\n", res1 == 0 ? "Success" : "Failure");
    } else {
        printf("Skipping test, file not found: %s\n", normal_zip);
    }
    printf("\n");

    // --- Test Case 2: Normal TAR Extraction ---
    printf("--- Test Case 2: Normal TAR Extraction ---\n");
    const char* normal_tar = "test.tar.gz";
    const char* extract_dir2 = "c_test_area/extract2";
    if (access(normal_tar, F_OK) == 0) {
        int res2 = extract_archive(normal_tar, extract_dir2);
        printf("Extraction result: %s\n", res2 == 0 ? "Success" : "Failure");
    } else {
        printf("Skipping test, file not found: %s\n", normal_tar);
    }
    printf("\n");

    // --- Test Case 3: Malicious ZIP with Path Traversal ---
    printf("--- Test Case 3: Malicious ZIP Extraction ---\n");
    const char* malicious_zip = "malicious.zip";
    const char* extract_dir3 = "c_test_area/extract3";
    if (access(malicious_zip, F_OK) == 0) {
        int res3 = extract_archive(malicious_zip, extract_dir3);
        printf("Extraction operation finished with result: %s\n", res3 == 0 ? "Success" : "Failure");
        printf("Check manually if 'malicious_file.txt' was created outside the test area.\n");
    } else {
        printf("Skipping test, file not found: %s\n", malicious_zip);
    }
    printf("\n");

    // --- Test Case 4: Extracting to a non-existent directory ---
    printf("--- Test Case 4: Extract to Non-Existent Directory ---\n");
    const char* extract_dir4 = "c_test_area/new_dir_extract";
    if (access(normal_zip, F_OK) == 0) {
        int res4 = extract_archive(normal_zip, extract_dir4);
        printf("Extraction result: %s\n", res4 == 0 ? "Success" : "Failure");
    } else {
        printf("Skipping test, file not found: %s\n", normal_zip);
    }
    printf("\n");

    // --- Test Case 5: Archive path does not exist ---
    printf("--- Test Case 5: Archive Path Not Found ---\n");
    const char* non_existent_zip = "notfound.zip";
    const char* extract_dir5 = "c_test_area/extract5";
    int res5 = extract_archive(non_existent_zip, extract_dir5);
    printf("Extraction result: %s\n", res5 == 0 ? "Success" : "Failure");
    printf("\n");

    printf("Tests complete. Manual cleanup of '%s' and test archives may be needed.\n", base_test_dir);

    return 0;
}