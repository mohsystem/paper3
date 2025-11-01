
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>

/* Using minizip library (part of zlib contrib) for zip extraction */
/* Compile with: gcc -o program program.c -lz -lminizip */
#include <minizip/unzip.h>

/* Security limits to prevent zip bomb attacks */
#define MAX_FILE_SIZE (100LL * 1024 * 1024)  /* 100MB per file */
#define MAX_TOTAL_SIZE (500LL * 1024 * 1024) /* 500MB total */
#define MAX_FILE_COUNT 10000
#define BASE_DIR "/tmp/unpack"
#define BUFFER_SIZE 8192

/* Create directory recursively with secure permissions (0700) */
static int create_directory_recursive(const char* path) {
    char tmp[PATH_MAX];
    char* p = NULL;
    size_t len;
    
    if (path == NULL || path[0] == '\\0') {
        return -1;
    }
    
    len = strlen(path);
    if (len >= PATH_MAX) {
        return -1;
    }
    
    /* Copy path safely */
    strncpy(tmp, path, PATH_MAX - 1);
    tmp[PATH_MAX - 1] = '\\0';
    
    /* Remove trailing slash */
    if (tmp[len - 1] == '/') {
        tmp[len - 1] = '\\0';
    }
    
    /* Create each directory in the path */
    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\\0';
            if (mkdir(tmp, 0700) != 0 && errno != EEXIST) {
                return -1;
            }
            *p = '/';
        }
    }
    
    /* Create the final directory */
    if (mkdir(tmp, 0700) != 0 && errno != EEXIST) {
        return -1;
    }
    
    return 0;
}

/* Validate path to prevent directory traversal attacks */
static int validate_path(const char* base_dir, const char* entry_name, 
                        char* output_path, size_t output_size) {
    if (base_dir == NULL || entry_name == NULL || output_path == NULL) {
        return -1;
    }
    
    /* Check for path traversal patterns */
    if (strstr(entry_name, "..") != NULL) {
        fprintf(stderr, "Path traversal attempt detected: %s\\n", entry_name);
        return -1;
    }
    
    /* Check for absolute paths */
    if (entry_name[0] == '/') {
        fprintf(stderr, "Absolute path not allowed: %s\\n", entry_name);
        return -1;
    }
    
    /* Construct output path safely */
    int result = snprintf(output_path, output_size, "%s/%s", base_dir, entry_name);
    if (result < 0 || (size_t)result >= output_size) {
        fprintf(stderr, "Path too long\\n");
        return -1;
    }
    
    return 0;
}

int unzip_archive(const char* zip_file_path) {
    unzFile uf = NULL;
    unz_global_info gi;
    long long total_size = 0;
    int file_count = 0;
    int ret = 0;
    
    /* Validate input */
    if (zip_file_path == NULL || zip_file_path[0] == '\\0') {
        fprintf(stderr, "Invalid zip file path\\n");
        return -1;
    }
    
    /* Create base directory with secure permissions */
    if (create_directory_recursive(BASE_DIR) != 0) {
        fprintf(stderr, "Failed to create base directory\\n");
        return -1;
    }
    
    /* Open zip file */
    uf = unzOpen(zip_file_path);
    if (uf == NULL) {
        fprintf(stderr, "Cannot open zip file: %s\\n", zip_file_path);
        return -1;
    }
    
    /* Get global info about the zip file */
    if (unzGetGlobalInfo(uf, &gi) != UNZ_OK) {
        fprintf(stderr, "Failed to get zip file info\\n");
        unzClose(uf);
        return -1;
    }
    
    /* Process each file in the archive */
    for (uLong i = 0; i < gi.number_entry; i++) {
        char filename[PATH_MAX];
        char output_path[PATH_MAX];
        unz_file_info file_info;
        size_t filename_len;
        
        file_count++;
        if (file_count > MAX_FILE_COUNT) {
            fprintf(stderr, "Too many files - potential zip bomb\\n");
            ret = -1;
            break;
        }
        
        /* Get current file info */
        memset(filename, 0, sizeof(filename));
        if (unzGetCurrentFileInfo(uf, &file_info, filename, sizeof(filename) - 1, 
                                  NULL, 0, NULL, 0) != UNZ_OK) {
            fprintf(stderr, "Failed to get file info\\n");
            ret = -1;
            break;
        }
        
        /* Validate and construct output path */
        if (validate_path(BASE_DIR, filename, output_path, sizeof(output_path)) != 0) {
            ret = -1;
            break;
        }
        
        /* Check if entry is a directory */
        filename_len = strlen(filename);
        if (filename_len > 0 && filename[filename_len - 1] == '/') {
            /* Create directory */
            if (create_directory_recursive(output_path) != 0) {
                fprintf(stderr, "Failed to create directory: %s\\n", output_path);
                ret = -1;
                break;
            }
        } else {
            /* Extract file */
            char* last_slash;
            int fd = -1;
            unsigned char buffer[BUFFER_SIZE];
            long long entry_size = 0;
            int bytes_read;
            
            /* Create parent directories */
            last_slash = strrchr(output_path, '/');
            if (last_slash != NULL) {
                *last_slash = '\\0';
                create_directory_recursive(output_path);
                *last_slash = '/';
            }
            
            /* Open file in zip */
            if (unzOpenCurrentFile(uf) != UNZ_OK) {
                fprintf(stderr, "Cannot open file in zip: %s\\n", filename);
                ret = -1;
                break;
            }
            
            /* Open output file with restricted permissions (0600) */
            fd = open(output_path, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0600);
            if (fd < 0) {
                fprintf(stderr, "Cannot create output file: %s\\n", output_path);
                unzCloseCurrentFile(uf);
                ret = -1;
                break;
            }
            
            /* Extract file content with size validation */
            while ((bytes_read = unzReadCurrentFile(uf, buffer, BUFFER_SIZE)) > 0) {
                ssize_t written;
                
                entry_size += bytes_read;
                total_size += bytes_read;
                
                /* Check individual file size to prevent zip bombs */
                if (entry_size > MAX_FILE_SIZE) {
                    fprintf(stderr, "File too large - potential zip bomb: %s\\n", filename);
                    close(fd);
                    unzCloseCurrentFile(uf);
                    ret = -1;
                    break;
                }
                
                /* Check total extracted size to prevent zip bombs */
                if (total_size > MAX_TOTAL_SIZE) {
                    fprintf(stderr, "Total size exceeds limit - potential zip bomb\\n");
                    close(fd);
                    unzCloseCurrentFile(uf);
                    ret = -1;
                    break;
                }
                
                /* Write to output file */
                written = write(fd, buffer, bytes_read);
                if (written != bytes_read) {
                    fprintf(stderr, "Write failed for file: %s\\n", output_path);
                    close(fd);
                    unzCloseCurrentFile(uf);
                    ret = -1;
                    break;
                }
            }
            
            /* Sync and close file */
            if (ret == 0 && fd >= 0) {
                fsync(fd);
            }
            
            if (fd >= 0) {
                close(fd);
            }
            
            unzCloseCurrentFile(uf);
            
            if (ret != 0 || bytes_read < 0) {
                ret = -1;
                break;
            }
        }
        
        /* Move to next file in archive */
        if (i + 1 < gi.number_entry) {
            if (unzGoToNextFile(uf) != UNZ_OK) {
                fprintf(stderr, "Failed to move to next file\\n");
                ret = -1;
                break;
            }
        }
    }
    
    unzClose(uf);
    return ret;
}

int main(void) {
    /* Test case 1: Valid zip file extraction */
    printf("Test 1: Extracting valid archive\\n");
    if (unzip_archive("archive.zip") == 0) {
        printf("Test 1: SUCCESS\\n");
    } else {
        printf("Test 1: FAILED\\n");
    }
    
    /* Test case 2: Non-existent file */
    printf("\\nTest 2: Non-existent file\\n");
    if (unzip_archive("nonexistent.zip") != 0) {
        printf("Test 2: SUCCESS - Correctly failed\\n");
    } else {
        printf("Test 2: FAILED - Should have failed\\n");
    }
    
    /* Test case 3: NULL input */
    printf("\\nTest 3: NULL input\\n");
    if (unzip_archive(NULL) != 0) {
        printf("Test 3: SUCCESS - Correctly rejected\\n");
    } else {
        printf("Test 3: FAILED - Should have rejected\\n");
    }
    
    /* Test case 4: Empty string */
    printf("\\nTest 4: Empty string input\\n");
    if (unzip_archive("") != 0) {
        printf("Test 4: SUCCESS - Correctly rejected\\n");
    } else {
        printf("Test 4: FAILED - Should have rejected\\n");
    }
    
    /* Test case 5: Special characters in filename */
    printf("\\nTest 5: Archive with special name\\n");
    if (unzip_archive("test-archive_v1.0.zip") == 0) {
        printf("Test 5: SUCCESS\\n");
    } else {
        printf("Test 5: FAILED\\n");
    }
    
    return 0;
}
