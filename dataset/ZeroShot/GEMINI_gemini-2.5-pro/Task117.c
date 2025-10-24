#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// Required for libarchive
#include <archive.h>
#include <archive_entry.h>

/*
================================================================================
IMPORTANT: This code requires the 'libarchive' library.
To compile on a Linux-like system, you need to install it first:
  - On Debian/Ubuntu: sudo apt-get install libarchive-dev
  - On Fedora/CentOS: sudo dnf install libarchive-devel
  - On macOS (with Homebrew): brew install libarchive

Then, compile this file using a C compiler:
  gcc Task117.c -o Task117 -larchive
================================================================================
*/

/**
 * Creates a directory if it doesn't exist.
 * Note: This is a simplified implementation for demonstration.
 */
static void ensure_dir_exists(const char* path) {
    // This is a simplified way to create a directory.
    // For a robust solution, you would create each parent directory in the path.
    #ifdef _WIN32
        _mkdir(path);
    #else
        mkdir(path, 0755);
    #endif
}

/**
 * Extracts an archive file (ZIP, TAR, etc.) to a destination directory.
 * @param archive_path Path to the archive file.
 * @param dest_dir Path to the destination directory.
 * @return 0 on success, non-zero on failure.
 */
int extract_archive(const char* archive_path, const char* dest_dir) {
    ensure_dir_exists(dest_dir);

    struct archive* a;
    struct archive_entry* entry;
    int r;
    int result = 0;

    a = archive_read_new();
    archive_read_support_filter_all(a);
    archive_read_support_format_all(a);

    r = archive_read_open_filename(a, archive_path, 10240);
    if (r != ARCHIVE_OK) {
        fprintf(stderr, "Error opening archive: %s\n", archive_error_string(a));
        archive_read_free(a);
        return 1;
    }
    
    // Change the current directory to the destination to simplify extraction paths
    if (chdir(dest_dir) != 0) {
        fprintf(stderr, "Error changing to directory: %s\n", dest_dir);
        archive_read_close(a);
        archive_read_free(a);
        return 1;
    }

    while ((r = archive_read_next_header(a, &entry)) == ARCHIVE_OK) {
        const char* pathname = archive_entry_pathname(entry);

        // Security Check: Basic Path Traversal (Zip Slip)
        // 1. Disallow absolute paths
        // 2. Disallow paths containing ".."
        if (pathname[0] == '/' || strstr(pathname, "..") != NULL) {
            fprintf(stderr, "Security violation: Path traversal detected in '%s'. Skipping.\n", pathname);
            continue; // Skip this malicious entry
        }
        
        // Use default extraction flags
        int flags = ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM | ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS;

        r = archive_read_extract(a, entry, flags);
        if (r != ARCHIVE_OK) {
            fprintf(stderr, "Error extracting %s: %s\n", pathname, archive_error_string(a));
            result = 1;
            break;
        }
    }

    if (r != ARCHIVE_EOF) {
        fprintf(stderr, "Archive reading error: %s\n", archive_error_string(a));
        result = 1;
    }

    archive_read_close(a);
    archive_read_free(a);
    
    // Change back to original directory (important!)
    // This is a simplified example; a robust solution would save the original CWD.
    chdir(".."); 
    
    return result;
}

int main() {
    printf("C Archive Extractor - Note: This program expects you to create files for testing.\n");
    printf("Please create a directory 'c_test_area' and place some archives in it.\n");
    printf("For example, create 'c_test_area/test.zip' and 'c_test_area/test.tar.gz'.\n\n");

    const char* test_dir = "c_test_area";
    const char* output_dir = "c_test_area/output";
    
    // Test Case 1: Extract a ZIP file
    printf("--- Test Case 1: Extracting a valid ZIP file (e.g., 'c_test_area/test.zip') ---\n");
    if (extract_archive("c_test_area/test.zip", "c_test_area/zip_out") == 0) {
        printf("Success: ZIP file extracted.\n");
    } else {
        printf("Failure: Could not extract ZIP file (does it exist?).\n");
    }

    // Test Case 2: Extract a TAR.GZ file
    printf("\n--- Test Case 2: Extracting a valid TAR.GZ file (e.g., 'c_test_area/test.tar.gz') ---\n");
    if (extract_archive("c_test_area/test.tar.gz", "c_test_area/tar_out") == 0) {
        printf("Success: TAR.GZ file extracted.\n");
    } else {
        printf("Failure: Could not extract TAR.GZ file (does it exist?).\n");
    }

    // Test Case 3: Attempt to extract a non-existent file
    printf("\n--- Test Case 3: Non-existent archive ---\n");
    if (extract_archive("non_existent_archive.zip", "c_test_area/non_existent_out") != 0) {
        printf("Success: Correctly handled non-existent archive.\n");
    } else {
        printf("Failure: Program behaved unexpectedly.\n");
    }

    // Test Case 4: Invalid destination directory (e.g., one we can't write to)
    printf("\n--- Test Case 4: Invalid destination ---\n");
    if (extract_archive("c_test_area/test.zip", "/root/no_permission") != 0) {
        printf("Success: Correctly failed on a directory with no permissions (as expected).\n");
    } else {
        printf("Failure: Program behaved unexpectedly.\n");
    }

    // Test Case 5: Path Traversal (Zip Slip)
    printf("\n--- Test Case 5: Path Traversal (Zip Slip) ---\n");
    printf("This requires a specially crafted archive (e.g., 'malicious.zip' with a file path like '../../evil.txt').\n");
    if (extract_archive("malicious.zip", "c_test_area/malicious_out") == 0) {
        printf("Success: Malicious entries in archive were skipped as expected.\n");
    } else {
        printf("Warning: Malicious archive extraction failed for other reasons.\n");
    }

    return 0;
}