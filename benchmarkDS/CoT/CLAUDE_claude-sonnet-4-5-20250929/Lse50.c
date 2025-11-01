
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

// Note: C requires external libraries like libzip for zip handling
// This is a conceptual implementation showing security measures

#ifdef __unix__
    #include <zip.h>
    #include <unistd.h>
    #include <limits.h>
#endif

int createDirectory(const char* path) {
#ifdef _WIN32
    return _mkdir(path);
#else
    return mkdir(path, 0755);
#endif
}

int secureExtractZip(const char* zipFilePath, const char* destDirectory) {
#ifdef __unix__
    int err = 0;
    zip_t* archive = zip_open(zipFilePath, ZIP_RDONLY, &err);
    
    if (!archive) {
        fprintf(stderr, "Failed to open zip file: %s\\n", zipFilePath);
        return 0;
    }
    
    // Create destination directory
    createDirectory(destDirectory);
    
    char destRealPath[PATH_MAX];
    if (!realpath(destDirectory, destRealPath)) {
        fprintf(stderr, "Failed to resolve destination path\\n");
        zip_close(archive);
        return 0;
    }
    
    zip_int64_t numEntries = zip_get_num_entries(archive, 0);
    
    for (zip_int64_t i = 0; i < numEntries; i++) {
        const char* name = zip_get_name(archive, i, 0);
        if (!name) continue;
        
        // Security: Check for path traversal attempts
        if (strstr(name, "..") != NULL) {
            fprintf(stderr, "Path traversal detected: %s\\n", name);
            zip_close(archive);
            return 0;
        }
        
        char fullPath[PATH_MAX];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", destDirectory, name);
        
        // Security: Validate resolved path
        char resolvedPath[PATH_MAX];
        if (realpath(fullPath, resolvedPath) && 
            strncmp(resolvedPath, destRealPath, strlen(destRealPath)) != 0) {
            fprintf(stderr, "Attempted extraction outside target directory\\n");
            zip_close(archive);
            return 0;
        }
        
        struct zip_stat st;
        zip_stat_init(&st);
        zip_stat_index(archive, i, 0, &st);
        
        if (name[strlen(name) - 1] == '/') {
            createDirectory(fullPath);
        } else {
            zip_file_t* file = zip_fopen_index(archive, i, 0);
            if (!file) continue;
            
            FILE* outFile = fopen(fullPath, "wb");
            if (!outFile) {
                zip_fclose(file);
                continue;
            }
            
            char buffer[4096];
            zip_int64_t bytesRead;
            
            while ((bytesRead = zip_fread(file, buffer, sizeof(buffer))) > 0) {
                fwrite(buffer, 1, bytesRead, outFile);
            }
            
            fclose(outFile);
            zip_fclose(file);
        }
    }
    
    zip_close(archive);
    return 1;
#else
    fprintf(stderr, "Zip extraction requires libzip on Unix systems\\n");
    return 0;
#endif
}

int main() {
    // Test case 1: Normal extraction
    if (secureExtractZip("archive.zip", "/tmp/unpack")) {
        printf("Test 1: Successfully extracted archive.zip\\n");
    } else {
        printf("Test 1 failed\\n");
    }
    
    // Test case 2: With different archive name
    if (secureExtractZip("test.zip", "/tmp/unpack")) {
        printf("Test 2: Successfully extracted test.zip\\n");
    } else {
        printf("Test 2 failed\\n");
    }
    
    // Test case 3: Non-existent file
    if (secureExtractZip("nonexistent.zip", "/tmp/unpack")) {
        printf("Test 3: Completed\\n");
    } else {
        printf("Test 3 failed (expected)\\n");
    }
    
    // Test case 4: Different destination
    if (secureExtractZip("archive.zip", "/tmp/unpack2")) {
        printf("Test 4: Successfully extracted to different location\\n");
    } else {
        printf("Test 4 failed\\n");
    }
    
    // Test case 5: Create nested directory structure
    if (secureExtractZip("archive.zip", "/tmp/unpack/nested/path")) {
        printf("Test 5: Successfully extracted to nested path\\n");
    } else {
        printf("Test 5 failed\\n");
    }
    
    return 0;
}
