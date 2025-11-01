// This code requires the libzip library.
// To compile: gcc Lse49.c -o Lse49 -lzip
// To run: ./Lse49

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <zip.h>
#include <limits.h> // For PATH_MAX

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <unistd.h>
#define MKDIR(path) mkdir(path, 0755)
#endif

// Helper function to create directories recursively
void create_dirs(const char* path) {
    char* p = strdup(path);
    if (!p) return;
    char* sep = strrchr(p, '/');
#ifdef _WIN32
    char* sep_bs = strrchr(p, '\\');
    if (sep_bs > sep) sep = sep_bs;
#endif
    if (sep != NULL) {
        *sep = '\0';
        create_dirs(p);
        *sep = '/';
    }
    MKDIR(path);
    free(p);
}


/**
 * Unzips an archive to a destination directory, preventing path traversal attacks.
 * @param zip_file_path Path to the .zip file.
 * @param dest_dir_path Path to the destination directory.
 */
void unzip_archive(const char* zip_file_path, const char* dest_dir_path) {
    // Ensure destination directory exists and get its canonical path
    MKDIR(dest_dir_path);
    
    char dest_dir_canonical[PATH_MAX];
    #ifdef _WIN32
    if (_fullpath(dest_dir_canonical, dest_dir_path, PATH_MAX) == NULL) {
    #else
    if (realpath(dest_dir_path, dest_dir_canonical) == NULL) {
    #endif
        perror("Could not resolve destination path");
        return;
    }
    size_t dest_dir_len = strlen(dest_dir_canonical);

    int err = 0;
    zip_t* za = zip_open(zip_file_path, 0, &err);
    if (!za) {
        fprintf(stderr, "Failed to open zip archive. Code: %d\n", err);
        return;
    }

    zip_int64_t num_entries = zip_get_num_entries(za, 0);
    for (zip_int64_t i = 0; i < num_entries; i++) {
        const char* name = zip_get_name(za, i, 0);
        if (name == NULL) {
            fprintf(stderr, "Warning: Could not get name for entry %lld\n", i);
            continue;
        }

        char dest_file_path[PATH_MAX];
        snprintf(dest_file_path, sizeof(dest_file_path), "%s/%s", dest_dir_path, name);
        
        // Security Check for Path Traversal (Zip Slip)
        char dest_file_canonical[PATH_MAX];
        #ifdef _WIN32
        if (_fullpath(dest_file_canonical, dest_file_path, PATH_MAX) == NULL) {
        #else
        if (realpath(dest_file_path, dest_file_canonical) == NULL) {
        #endif
            // realpath fails if path doesn't exist, but we still need to check.
            // A simple string check is a good first step.
            if (strstr(name, "..") != NULL) {
                 fprintf(stderr, "Skipping potentially malicious path: %s\n", name);
                 continue;
            }
             // For non-existent files, we build the path and normalize it manually for the check
            snprintf(dest_file_canonical, sizeof(dest_file_canonical), "%s/%s", dest_dir_canonical, name);
        }

        if (strncmp(dest_dir_canonical, dest_file_canonical, dest_dir_len) != 0) {
            fprintf(stderr, "Skipping malicious path: %s\n", name);
            continue;
        }

        if (name[strlen(name) - 1] == '/') {
            MKDIR(dest_file_canonical);
        } else {
            create_dirs(dest_file_canonical); // Create parent dirs
            // remove filename from path
            char *last_slash = strrchr(dest_file_canonical, '/');
            if(last_slash) *last_slash = '\0';
            MKDIR(dest_file_canonical);
            if(last_slash) *last_slash = '/';


            zip_stat_t zs;
            zip_stat_index(za, i, 0, &zs);

            zip_file_t* zf = zip_fopen_index(za, i, 0);
            if (!zf) {
                fprintf(stderr, "Warning: Could not open file in zip: %s\n", name);
                continue;
            }

            char* buffer = (char*)malloc(zs.size);
            if (!buffer) {
                fprintf(stderr, "Error allocating memory.\n");
                zip_fclose(zf);
                continue;
            }

            zip_fread(zf, buffer, zs.size);
            zip_fclose(zf);

            FILE* of = fopen(dest_file_canonical, "wb");
            if (of) {
                fwrite(buffer, 1, zs.size, of);
                fclose(of);
            } else {
                perror("Could not open output file");
            }
            free(buffer);
        }
    }
    zip_close(za);
}


// Helper function to create a test zip file using libzip
void create_test_zip(const char* zip_file_name, int test_case) {
    int error = 0;
    zip_t* za = zip_open(zip_file_name, ZIP_CREATE | ZIP_TRUNCATE, &error);
    if (!za) {
        fprintf(stderr, "Failed to create zip archive. Code: %d\n", error);
        return;
    }

    const char* content1 = "content of file1";
    zip_source_t* source1 = zip_source_buffer(za, content1, strlen(content1), 0);
    zip_file_add(za, "file1.txt", source1, ZIP_FL_OVERWRITE);

    const char* content2 = "content of file2";
    zip_source_t* source2 = zip_source_buffer(za, content2, strlen(content2), 0);
    zip_file_add(za, "dir1/file2.txt", source2, ZIP_FL_OVERWRITE);
    
    if (test_case > 1) {
        const char* content3 = "malicious content";
        zip_source_t* source3 = zip_source_buffer(za, content3, strlen(content3), 0);
        zip_file_add(za, "../../evil.txt", source3, ZIP_FL_OVERWRITE);
    }

    zip_close(za);
}

// Helper to check if a file exists
int file_exists(const char *path) {
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

// Helper to clean up
void cleanup(const char* zip_file_name, const char* unpack_dir) {
    remove(zip_file_name);
    // A simple cleanup; for robust recursive delete, a dedicated function would be needed.
    remove("/tmp/unpack/dir1/file2.txt");
    rmdir("/tmp/unpack/dir1");
    remove("/tmp/unpack/file1.txt");
    rmdir(unpack_dir);
    remove("/tmp/evil.txt");
}


int main() {
    const char* zip_file_name = "archive.zip";
    const char* unpack_dir = "/tmp/unpack";

    for (int i = 1; i <= 5; ++i) {
        printf("--- Running Test Case %d ---\n", i);
        
        // Setup
        create_test_zip(zip_file_name, i);
        printf("Created %s\n", zip_file_name);

        // Execute
        unzip_archive(zip_file_name, unpack_dir);
        printf("Successfully ran unzip on %s\n", unpack_dir);

        // Verify
        if (file_exists("/tmp/unpack/file1.txt")) printf("Verified: file1.txt exists.\n");
        if (file_exists("/tmp/unpack/dir1/file2.txt")) printf("Verified: dir1/file2.txt exists.\n");
        if (file_exists("/tmp/evil.txt")) {
            fprintf(stderr, "SECURITY VULNERABILITY: Traversal file created at /tmp/evil.txt\n");
        } else {
            printf("Security check passed: Traversal file was not created.\n");
        }

        // Teardown
        printf("Cleaning up...\n");
        cleanup(zip_file_name, unpack_dir);
        printf("---------------------------\n\n");
    }
    
    return 0;
}