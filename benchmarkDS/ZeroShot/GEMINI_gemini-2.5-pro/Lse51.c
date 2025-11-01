// This code requires the 'libarchive' library.
// To compile: gcc your_file_name.c -o Lse51 -larchive
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>

#include <archive.h>
#include <archive_entry.h>

const char* TEST_DIR = "/tmp/unpack_c_test";

/**
 * Securely extracts a tar archive.
 * Resource cleanup is handled manually with a goto pattern, common in C.
 */
static int secure_untar(const char *filename, const char *extract_path) {
    struct archive *a;
    struct archive_entry *entry;
    int r;
    int result = 0; // 0 for success, -1 for failure
    char extract_path_abs[PATH_MAX];
    char dest_path_abs[PATH_MAX];

    // Get absolute path of the destination directory
    if (realpath(extract_path, extract_path_abs) == NULL) {
        perror("realpath on extract_path failed");
        return -1;
    }
    size_t extract_path_len = strlen(extract_path_abs);

    a = archive_read_new();
    archive_read_support_filter_all(a);
    archive_read_support_format_all(a);

    r = archive_read_open_filename(a, filename, 10240);
    if (r != ARCHIVE_OK) {
        fprintf(stderr, "Error opening archive: %s\n", archive_error_string(a));
        result = -1;
        goto cleanup_read;
    }

    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        const char *entry_pathname = archive_entry_pathname(entry);
        char full_dest_path[PATH_MAX];
        snprintf(full_dest_path, sizeof(full_dest_path), "%s/%s", extract_path, entry_pathname);

        // Security Check: Resolve the path and ensure it's within the destination
        if (realpath(full_dest_path, dest_path_abs) == NULL) {
            // realpath will fail if the path doesn't exist yet, which is expected.
            // We can manually check for ".." components or absolute paths.
            if (strstr(entry_pathname, "/../") != NULL || strncmp(entry_pathname, "../", 3) == 0 || entry_pathname[0] == '/') {
                fprintf(stderr, "Security Error: Malicious entry detected (Path Traversal): %s\n", entry_pathname);
                result = -1;
                goto cleanup_all;
            }
        } else {
             if (strncmp(extract_path_abs, dest_path_abs, extract_path_len) != 0) {
                fprintf(stderr, "Security Error: Malicious entry detected (Path Traversal): %s\n", entry_pathname);
                result = -1;
                goto cleanup_all;
             }
        }
        
        archive_entry_set_pathname(entry, full_dest_path);
        r = archive_read_extract(a, entry, ARCHIVE_EXTRACT_TIME);
        if (r != ARCHIVE_OK) {
            fprintf(stderr, "Error extracting %s: %s\n", full_dest_path, archive_error_string(a));
            result = -1;
            goto cleanup_all;
        }
    }

cleanup_all:
    archive_read_close(a);
cleanup_read:
    archive_read_free(a);
    return result;
}

// --- Test Case Setup ---

void create_test_tar(const char* archive_name, const char* names[], const char* contents[], int count) {
    struct archive *a = archive_write_new();
    archive_write_set_format_pax_restricted(a);
    archive_write_open_filename(a, archive_name);

    for (int i = 0; i < count; ++i) {
        struct archive_entry *entry = archive_entry_new();
        archive_entry_set_pathname(entry, names[i]);
        archive_entry_set_size(entry, strlen(contents[i]));
        archive_entry_set_filetype(entry, AE_IFREG);
        archive_entry_set_perm(entry, 0644);
        archive_write_header(a, entry);
        archive_write_data(a, contents[i], strlen(contents[i]));
        archive_entry_free(entry);
    }
    archive_write_close(a);
    archive_write_free(a);
}

void setup_test_environment() {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", TEST_DIR);
    system(cmd);
    mkdir(TEST_DIR, 0755);

    const char* good_names[] = {"file1.txt", "dir1/file2.txt"};
    const char* good_contents[] = {"content1", "content2"};
    create_test_tar("good.tar", good_names, good_contents, 2);

    const char* bad_trav_names[] = {"../../tmp/evil.txt"};
    const char* bad_trav_contents[] = {"malicious"};
    create_test_tar("bad_traversal.tar", bad_trav_names, bad_trav_contents, 1);
    
    const char* bad_abs_names[] = {"/tmp/evil_absolute.txt"};
    const char* bad_abs_contents[] = {"malicious"};
    create_test_tar("bad_absolute.tar", bad_abs_names, bad_abs_contents, 1);
    
    FILE *f = fopen("corrupt.tar", "w");
    if(f) {
        fputs("this is not a tar file", f);
        fclose(f);
    }
}

void cleanup_test_environment() {
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "rm -rf %s", TEST_DIR);
    system(cmd);
    remove("good.tar");
    remove("bad_traversal.tar");
    remove("bad_absolute.tar");
    remove("corrupt.tar");
}

int main() {
    setup_test_environment();
    printf("--- Running C Test Cases ---\n");
    int status = 0;

    // Test Case 1: Valid tar file
    printf("\n[Test 1] Extracting a valid TAR file...\n");
    int result1 = secure_untar("good.tar", TEST_DIR);
    printf("Result: %s\n", (result1 == 0) ? "SUCCESS" : "FAILURE");
    if (result1 != 0) status = 1;

    // Test Case 2: Path traversal attack
    printf("\n[Test 2] Extracting a TAR with path traversal...\n");
    int result2 = secure_untar("bad_traversal.tar", TEST_DIR);
    printf("Result: %s\n", (result2 != 0) ? "SUCCESS (Attack Blocked)" : "FAILURE (Vulnerable)");
    if (result2 == 0) status = 1;

    // Test Case 3: Absolute path attack
    printf("\n[Test 3] Extracting a TAR with an absolute path...\n");
    int result3 = secure_untar("bad_absolute.tar", TEST_DIR);
    printf("Result: %s\n", (result3 != 0) ? "SUCCESS (Attack Blocked)" : "FAILURE (Vulnerable)");
    if (result3 == 0) status = 1;

    // Test Case 4: Non-existent file
    printf("\n[Test 4] Attempting to extract a non-existent TAR file...\n");
    int result4 = secure_untar("non_existent.tar", TEST_DIR);
    printf("Result: %s\n", (result4 != 0) ? "SUCCESS (Handled gracefully)" : "FAILURE");
    if (result4 == 0) status = 1;

    // Test Case 5: Corrupt tar file
    printf("\n[Test 5] Extracting a corrupt TAR file...\n");
    int result5 = secure_untar("corrupt.tar", TEST_DIR);
    printf("Result: %s\n", (result5 != 0) ? "SUCCESS (Handled gracefully)" : "FAILURE");
    if (result5 == 0) status = 1;

    cleanup_test_environment();
    printf("\n--- C Tests Complete, Cleanup Done ---\n");

    return status;
}