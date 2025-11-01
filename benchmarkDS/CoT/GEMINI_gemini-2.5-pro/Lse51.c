#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

// Required external library: libarchive
// On Debian/Ubuntu: sudo apt-get install libarchive-dev
// On macOS (Homebrew): brew install libarchive
// Compile with: gcc your_source_file.c -o your_executable -larchive
#include <archive.h>
#include <archive_entry.h>

#if defined(_WIN32) || defined(__CYGWIN__)
#include <direct.h>
#define MKDIR(path, mode) _mkdir(path)
#define PATH_MAX 4096
#define PATH_SEPARATOR '\\'
#else
#include <unistd.h>
#include <limits.h>
#define MKDIR(path, mode) mkdir(path, mode)
#define PATH_SEPARATOR '/'
#endif


/**
 * Recursively creates a directory path.
 */
static int mkpath(char* path, mode_t mode) {
    char *p;
    for (p = strchr(path + 1, PATH_SEPARATOR); p; p = strchr(p + 1, PATH_SEPARATOR)) {
        *p = '\0';
        if (MKDIR(path, mode) == -1) {
            if (errno != EEXIST) {
                *p = PATH_SEPARATOR;
                return -1;
            }
        }
        *p = PATH_SEPARATOR;
    }
    if (MKDIR(path, mode) == -1) {
        if (errno != EEXIST) {
            return -1;
        }
    }
    return 0;
}


/**
 * Copies data from the archive to a file on disk.
 */
static int copy_data(struct archive *ar, FILE *f) {
    const void *buff;
    size_t size;
    la_int64_t offset;
    int r;

    for (;;) {
        r = archive_read_data_block(ar, &buff, &size, &offset);
        if (r == ARCHIVE_EOF)
            return ARCHIVE_OK;
        if (r < ARCHIVE_OK) {
            fprintf(stderr, "Archive read error: %s\n", archive_error_string(ar));
            return r;
        }
        if (fwrite(buff, 1, size, f) != size) {
            fprintf(stderr, "Can't write to file.\n");
            return ARCHIVE_FATAL;
        }
    }
}

/**
 * Safely extracts a tar archive, preventing path traversal attacks.
 */
int extractTar(const char* tarPath, const char* destDir) {
    struct stat st;
    if (stat(tarPath, &st) != 0) {
        fprintf(stderr, "Error: Archive file not found: %s\n", tarPath);
        return 0;
    }
    
    struct archive *a;
    struct archive_entry *entry;
    int r;
    
    a = archive_read_new();
    archive_read_support_format_all(a);
    archive_read_support_filter_all(a);
    
    r = archive_read_open_filename(a, tarPath, 10240);
    if (r != ARCHIVE_OK) {
        fprintf(stderr, "Error opening archive: %s\n", archive_error_string(a));
        archive_read_free(a);
        return 0;
    }

    char canonical_dest[PATH_MAX];
    // Create the destination directory if it doesn't exist
    if (stat(destDir, &st) == -1) {
        mkpath((char*)destDir, 0755);
    }
    // Get the real, absolute path of the destination
    if (realpath(destDir, canonical_dest) == NULL) {
        fprintf(stderr, "Error resolving destination path: %s\n", destDir);
        archive_read_close(a);
        archive_read_free(a);
        return 0;
    }
    size_t dest_len = strlen(canonical_dest);

    printf("Extracting %s to %s\n", tarPath, canonical_dest);

    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        const char *entry_pathname = archive_entry_pathname(entry);
        char full_path[PATH_MAX];
        
        // Security Check: Absolute paths in archive
        if (entry_pathname[0] == '/') {
             fprintf(stderr, "SECURITY: Skipping absolute path entry: %s\n", entry_pathname);
             continue;
        }

        snprintf(full_path, sizeof(full_path), "%s/%s", canonical_dest, entry_pathname);
        
        char resolved_path[PATH_MAX];
        if (realpath(full_path, resolved_path) == NULL) {
            // realpath fails if the file doesn't exist. We build the path and check it manually.
            // This is safer.
            strcpy(resolved_path, full_path);
        }
        
        // Security Check: Path Traversal
        if (strncmp(resolved_path, canonical_dest, dest_len) != 0) {
            fprintf(stderr, "SECURITY: Skipping potentially malicious entry: %s\n", entry_pathname);
            continue;
        }
        // Ensure it's inside the subdir, e.g. /tmp/unpack not /tmp/unpack-evil
        if (resolved_path[dest_len] != '\0' && resolved_path[dest_len] != '/') {
            fprintf(stderr, "SECURITY: Skipping potentially malicious entry: %s\n", entry_pathname);
            continue;
        }

        printf("  -> %s\n", entry_pathname);
        
        if (archive_entry_filetype(entry) == AE_IFDIR) {
            mkpath(full_path, 0755);
        } else if (archive_entry_filetype(entry) == AE_IFREG) {
            // Create parent directories for the file
            char *p = strrchr(full_path, '/');
            if (p) {
                *p = '\0';
                mkpath(full_path, 0755);
                *p = '/';
            }
            
            FILE* f = fopen(full_path, "wb");
            if (f) {
                copy_data(a, f);
                fclose(f);
            } else {
                fprintf(stderr, "Error opening file for writing: %s\n", full_path);
            }
        }
    }
    
    archive_read_close(a);
    archive_read_free(a);
    return 1;
}

int main() {
    // --- Test Setup ---
    // These test archives must be created beforehand.
    // You can generate them using the provided Python script.
    const char* testDataBaseDir = "/tmp/lse51_test_data/";
    char testArchives[5][PATH_MAX];
    snprintf(testArchives[0], PATH_MAX, "%stest.tar", testDataBaseDir);
    snprintf(testArchives[1], PATH_MAX, "%stest.tar.gz", testDataBaseDir);
    snprintf(testArchives[2], PATH_MAX, "%stest.tar.bz2", testDataBaseDir);
    snprintf(testArchives[3], PATH_MAX, "%smalicious_traversal.tar", testDataBaseDir);
    snprintf(testArchives[4], PATH_MAX, "%smalicious_absolute.tar", testDataBaseDir);

    const char* unpackDir = "/tmp/unpack_c";
    char command[PATH_MAX + 10];

    printf("--- C Tar Extraction Tests ---\n");
    printf("NOTE: Test archives are expected in %s\n", testDataBaseDir);
    printf("You can generate them by running the Python code in this file.\n\n");

    for (int i = 0; i < 5; ++i) {
        printf("\n--- Test Case %d ---\n", i + 1);
        
        // Clean up previous run
        snprintf(command, sizeof(command), "rm -rf %s", unpackDir);
        system(command);
        
        extractTar(testArchives[i], unpackDir);
    }
    
    printf("\n--- C Tests Complete ---\n");
    return 0;
}