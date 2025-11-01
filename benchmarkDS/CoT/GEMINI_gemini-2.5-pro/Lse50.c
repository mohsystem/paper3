#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

// Requires libarchive. Link with -larchive
// On Debian/Ubuntu: sudo apt-get install libarchive-dev
// On macOS (Homebrew): brew install libarchive
#include <archive.h>
#include <archive_entry.h>

/**
 * Extracts a TAR archive to a specified directory using libarchive.
 * Relies on libarchive's built-in security features.
 *
 * @param tar_file_path Path to the .tar file.
 * @param dest_dir_path Path to the destination directory.
 * @return 0 on success, non-zero on failure.
 */
int extract_tar(const char* tar_file_path, const char* dest_dir_path) {
    struct archive *a;
    struct archive *ext;
    struct archive_entry *entry;
    int flags;
    int r;
    
    // Create destination directory if it doesn't exist.
    // NOTE: In a real-world scenario, a more robust directory creation
    // function that handles nested paths would be better.
    mkdir(dest_dir_path, 0755);

    // Set flags for secure extraction
    flags = ARCHIVE_EXTRACT_TIME;
    flags |= ARCHIVE_EXTRACT_PERM;
    flags |= ARCHIVE_EXTRACT_ACL;
    flags |= ARCHIVE_EXTRACT_FFLAGS;
    // Security: Prevent .., absolute paths, and symlink traversals
    flags |= ARCHIVE_EXTRACT_SECURE_NODOTDOT;
    flags |= ARCHIVE_EXTRACT_SECURE_SYMLINKS;
    flags |= ARCHIVE_EXTRACT_SECURE_NOABSOLUTEPATHS;

    a = archive_read_new();
    archive_read_support_format_all(a);
    archive_read_support_filter_all(a);

    ext = archive_write_disk_new();
    archive_write_disk_set_options(ext, flags);
    archive_write_disk_set_standard_lookup(ext);

    if ((r = archive_read_open_filename(a, tar_file_path, 10240))) {
        fprintf(stderr, "Error opening archive: %s\n", archive_error_string(a));
        archive_read_free(a);
        archive_write_free(ext);
        return 1;
    }

    int result = 0;
    while (1) {
        r = archive_read_next_header(a, &entry);
        if (r == ARCHIVE_EOF) {
            break;
        }
        if (r < ARCHIVE_OK) {
            fprintf(stderr, "Error reading header: %s\n", archive_error_string(a));
        }
        if (r < ARCHIVE_WARN) {
            result = 1;
            break;
        }

        // Prepend destination path to the entry's path
        const char *current_file = archive_entry_pathname(entry);
        char dest_path[1024];
        snprintf(dest_path, sizeof(dest_path), "%s/%s", dest_dir_path, current_file);
        archive_entry_set_pathname(entry, dest_path);

        r = archive_write_header(ext, entry);
        if (r < ARCHIVE_OK) {
            fprintf(stderr, "Error writing header: %s\n", archive_error_string(ext));
        } else if (archive_entry_size(entry) > 0) {
            const void *buff;
            size_t size;
        #if ARCHIVE_VERSION_NUMBER >= 3000000
            int64_t offset;
        #else
            off_t offset;
        #endif
            while (1) {
                r = archive_read_data_block(a, &buff, &size, &offset);
                if (r == ARCHIVE_EOF) break;
                if (r < ARCHIVE_OK) {
                     fprintf(stderr, "Error reading data block: %s\n", archive_error_string(a));
                     result = 1;
                     break;
                }
                r = archive_write_data_block(ext, buff, size, offset);
                if (r < ARCHIVE_OK) {
                    fprintf(stderr, "Error writing data block: %s\n", archive_error_string(ext));
                    result = 1;
                    break;
                }
            }
            if (result != 0) break;
        }
        r = archive_write_finish_entry(ext);
        if (r < ARCHIVE_OK) {
            fprintf(stderr, "Error finishing entry: %s\n", archive_error_string(ext));
        }
        if (r < ARCHIVE_WARN) {
            result = 1;
            break;
        }
    }

    archive_read_close(a);
    archive_read_free(a);
    archive_write_close(ext);
    archive_write_free(ext);

    return result;
}

// Helper to run a system command
void run_command(const char* cmd) {
    printf("Executing: %s\n", cmd);
    int res = system(cmd);
    if (res != 0) {
        fprintf(stderr, "Command failed: %s\n", cmd);
    }
}

int main() {
    // --- Test Setup ---
    run_command("mkdir -p test_data/subdir");
    run_command("echo 'hello' > test_data/file1.txt");
    run_command("echo 'world' > test_data/subdir/file2.txt");
    run_command("echo 'malicious' > test_data/evil.txt");

    // --- Test Cases ---
    const char* test_names[] = {
        "test_case_1_normal",
        "test_case_2_empty_tar",
        "test_case_3_nested_dirs",
        "test_case_4_malicious_path_rel",
        "test_case_5_malicious_path_abs"
    };
    const char* create_cmds[] = {
        "tar -cf test_case_1_normal.tar -C test_data file1.txt subdir/file2.txt",
        "tar -cf test_case_2_empty_tar.tar --files-from /dev/null",
        "tar -cf test_case_3_nested_dirs.tar --transform 's,^,a/b/c/,' -C test_data file1.txt",
        "tar -cf test_case_4_malicious_path_rel.tar --transform 's,^,../../,' -C test_data evil.txt",
        "tar -cf test_case_5_malicious_path_abs.tar -C test_data --transform 's,^,/tmp/,' evil.txt"
    };
    int num_tests = sizeof(test_names) / sizeof(test_names[0]);

    for (int i = 0; i < num_tests; ++i) {
        char tar_file[256];
        char unpack_dir[256];
        char cleanup_cmd[512];

        snprintf(tar_file, sizeof(tar_file), "%s.tar", test_names[i]);
        snprintf(unpack_dir, sizeof(unpack_dir), "/tmp/unpack_c_%s", test_names[i]);

        printf("\n--- Running %s ---\n", test_names[i]);
        
        // 1. Create TAR file
        run_command(create_cmds[i]);

        // 2. Run extraction
        printf("Extracting %s to %s\n", tar_file, unpack_dir);
        if (extract_tar(tar_file, unpack_dir) == 0) {
            printf("Extraction finished.\n");
        } else {
            fprintf(stderr, "Extraction failed (might be an expected failure for a malicious archive).\n");
        }

        // 3. Cleanup
        snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -f %s && rm -rf %s", tar_file, unpack_dir);
        run_command(cleanup_cmd);
        printf("Cleaned up.\n");
    }
    
    // --- Global Cleanup ---
    run_command("rm -rf test_data");

    return 0;
}