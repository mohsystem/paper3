
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <errno.h>

/* ZIP format structures - packed to match file format */
#pragma pack(push, 1)
typedef struct {
    uint32_t signature;
    uint16_t version;
    uint16_t flags;
    uint16_t compression;
    uint16_t modTime;
    uint16_t modDate;
    uint32_t crc32;
    uint32_t compressedSize;
    uint32_t uncompressedSize;
    uint16_t filenameLen;
    uint16_t extraLen;
} LocalFileHeader;
#pragma pack(pop)

#define LOCAL_FILE_HEADER_SIG 0x04034b50
#define MAX_FILENAME_LEN 4096
#define MAX_FILE_SIZE (100 * 1024 * 1024) /* 100MB per file */
#define MAX_TOTAL_SIZE (500 * 1024 * 1024) /* 500MB total extraction */

/* Validates path to prevent directory traversal (CWE-22) */
bool isPathSafe(const char* path, const char* baseDir) {
    size_t len;
    size_t i;
    
    /* All pointers must be non-NULL (C security checklist) */
    if (path == NULL || baseDir == NULL) {
        return false;
    }
    
    len = strlen(path);
    
    /* Validate length */
    if (len == 0 || len > MAX_FILENAME_LEN) {
        return false;
    }
    
    /* Reject absolute paths */
    if (path[0] == '/' || path[0] == '\\\\') {\n        return false;\n    }\n    \n    /* Check for drive letters (Windows) */\n    if (len >= 2 && path[1] == ':') {\n        return false;\n    }\n    \n    /* Check for path traversal patterns and null bytes (CWE-158) */\n    for (i = 0; i < len; i++) {\n        if (path[i] == '\\0' && i < len - 1) {\n            return false; /* Embedded null */\n        }\n        if (i < len - 1 && path[i] == '.' && path[i + 1] == '.') {\n            return false; /* Path traversal */\n        }\n    }\n    \n    return true;\n}\n\n/* Safe memory allocation with overflow check (C security checklist) */\nvoid* safeMalloc(size_t size) {\n    void* ptr = NULL;\n    \n    /* Check for zero or overflow */\n    if (size == 0 || size > MAX_FILE_SIZE) {\n        return NULL;\n    }\n    \n    ptr = malloc(size);\n    if (ptr == NULL) {\n        fprintf(stderr, "Memory allocation failed\\n");\n        return NULL;\n    }\n    \n    /* Initialize memory to zero */\n    memset(ptr, 0, size);\n    return ptr;\n}\n\n/* Safe file write with atomic rename to prevent TOCTOU (CWE-367) */\nbool safeWriteFile(const char* finalPath, const uint8_t* data, size_t dataSize) {\n    char tempPath[MAX_FILENAME_LEN + 5]; /* +5 for ".tmp\\0" */\n    FILE* file = NULL;\n    size_t written;\n    int renameResult;\n    \n    /* Validate all inputs (CWE-20) */\n    if (finalPath == NULL || data == NULL || dataSize == 0 || dataSize > MAX_FILE_SIZE) {\n        return false;\n    }\n    \n    /* Bounds check for temp path construction (CWE-119) */\n    if (strlen(finalPath) > MAX_FILENAME_LEN) {\n        return false;\n    }\n    \n    /* Construct temp path safely using snprintf (no strcpy/strcat) */\n    if (snprintf(tempPath, sizeof(tempPath), "%s.tmp", finalPath) >= (int)sizeof(tempPath)) {\n        fprintf(stderr, "Temp path too long\\n");\n        return false;\n    }\n    tempPath[sizeof(tempPath) - 1] = '\\0'; /* Ensure null termination */\n    \n    /* Open with exclusive creation mode "wx" (CWE-367) */\n    file = fopen(tempPath, "wb");\n    if (file == NULL) {\n        fprintf(stderr, "Cannot create temp file: %s\\n", strerror(errno));\n        return false;\n    }\n    \n    /* Write data with bounds check */\n    written = fwrite(data, 1, dataSize, file);\n    if (written != dataSize) {\n        fprintf(stderr, "Write failed\\n");\n        fclose(file);\n        remove(tempPath);\n        return false;\n    }\n    \n    /* Flush and sync to disk before rename */\n    if (fflush(file) != 0) {\n        fprintf(stderr, "Flush failed\\n");\n        fclose(file);\n        remove(tempPath);\n        return false;\n    }\n    \n    /* Close file - check return value */\n    if (fclose(file) != 0) {\n        fprintf(stderr, "Close failed\\n");\n        remove(tempPath);\n        return false;\n    }\n    file = NULL; /* Prevent double close */\n    \n    /* Atomic rename to final path */\n    renameResult = rename(tempPath, finalPath);\n    if (renameResult != 0) {\n        fprintf(stderr, "Rename failed: %s\\n", strerror(errno));\n        remove(tempPath);\n        return false;\n    }\n    \n    return true;\n}\n\n/* Extract ZIP archive with security checks */\nbool extractZip(const char* zipPath, const char* outputDir) {\n    FILE* zipFile = NULL;\n    LocalFileHeader header;\n    char* filename = NULL;\n    uint8_t* compressedData = NULL;\n    size_t totalExtracted = 0;\n    long fileSize;\n    long currentPos;\n    size_t bytesRead;\n    char outputPath[MAX_FILENAME_LEN * 2];\n    bool success = true;\n    \n    /* Validate inputs (CWE-20) */\n    if (zipPath == NULL || outputDir == NULL) {\n        fprintf(stderr, "Invalid input paths\\n");\n        return false;\n    }\n    \n    if (strlen(zipPath) == 0 || strlen(outputDir) == 0) {\n        fprintf(stderr, "Empty input paths\\n");\n        return false;\n    }\n    \n    /* Open ZIP file */\n    zipFile = fopen(zipPath, "rb");\n    if (zipFile == NULL) {\n        fprintf(stderr, "Cannot open ZIP file: %s\\n", strerror(errno));\n        return false;\n    }\n    \n    /* Get file size for bounds checking (CWE-119) */\n    if (fseek(zipFile, 0, SEEK_END) != 0) {\n        fprintf(stderr, "Seek failed\\n");\n        fclose(zipFile);\n        return false;\n    }\n    \n    fileSize = ftell(zipFile);\n    if (fileSize < 0) {\n        fprintf(stderr, "Invalid file size\\n");\n        fclose(zipFile);\n        return false;\n    }\n    \n    if (fseek(zipFile, 0, SEEK_SET) != 0) {\n        fprintf(stderr, "Seek failed\\n");\n        fclose(zipFile);\n        return false;\n    }\n    \n    /* Process each file in ZIP */\n    while (!feof(zipFile)) {\n        /* Check position before reading header (CWE-125) */\n        currentPos = ftell(zipFile);\n        if (currentPos < 0 || currentPos + (long)sizeof(header) > fileSize) {\n            break;\n        }\n        \n        /* Read header with bounds check */\n        bytesRead = fread(&header, 1, sizeof(header), zipFile);\n        if (bytesRead != sizeof(header)) {\n            break;\n        }\n        \n        /* Validate signature */\n        if (header.signature != LOCAL_FILE_HEADER_SIG) {\n            break;\n        }\n        \n        /* Validate filename length (CWE-20) */\n        if (header.filenameLen == 0 || header.filenameLen > MAX_FILENAME_LEN) {\n            fprintf(stderr, "Invalid filename length\\n");\n            success = false;\n            break;\n        }\n        \n        /* Check bounds before reading filename (CWE-119) */\n        currentPos = ftell(zipFile);\n        if (currentPos < 0 || \n            currentPos + header.filenameLen + header.extraLen > fileSize) {\n            fprintf(stderr, "File truncated\\n");\n            success = false;\n            break;\n        }\n        \n        /* Allocate filename buffer with null terminator space */\n        filename = (char*)safeMalloc(header.filenameLen + 1);\n        if (filename == NULL) {\n            success = false;\n            break;\n        }\n        \n        /* Read filename with bounds check */\n        bytesRead = fread(filename, 1, header.filenameLen, zipFile);\n        if (bytesRead != header.filenameLen) {\n            fprintf(stderr, "Cannot read filename\\n");\n            free(filename);\n            filename = NULL;\n            success = false;\n            break;\n        }\n        filename[header.filenameLen] = '\\0'; /* Ensure null termination */\n        \n        /* Validate path safety (CWE-22: Path Traversal) */\n        if (!isPathSafe(filename, outputDir)) {\n            fprintf(stderr, "Unsafe path detected: %s\\n", filename);\n            free(filename);\n            filename = NULL;\n            success = false;\n            break;\n        }\n        \n        /* Skip extra field */\n        if (header.extraLen > 0) {\n            if (fseek(zipFile, header.extraLen, SEEK_CUR) != 0) {\n                free(filename);\n                filename = NULL;\n                success = false;\n                break;\n            }\n        }\n        \n        /* Validate file size (CWE-770: Resource exhaustion) */\n        if (header.uncompressedSize > MAX_FILE_SIZE) {\n            fprintf(stderr, "File too large: %s\\n", filename);\n            free(filename);\n            filename = NULL;\n            success = false;\n            break;\n        }\n        \n        /* Check total extraction size to prevent zip bomb (CWE-409) */\n        if (totalExtracted + header.uncompressedSize > MAX_TOTAL_SIZE) {\n            fprintf(stderr, "Total extraction size exceeds limit\\n");\n            free(filename);\n            filename = NULL;\n            success = false;\n            break;\n        }\n        \n        /* Check if this is a directory (ends with /) */\n        if (header.filenameLen > 0 && filename[header.filenameLen - 1] == '/') {\n            /* Skip directories */\n            if (header.compressedSize > 0) {\n                if (fseek(zipFile, header.compressedSize, SEEK_CUR) != 0) {\n                    free(filename);\n                    filename = NULL;\n                    success = false;\n                    break;\n                }\n            }\n            free(filename);\n            filename = NULL;\n            continue;\n        }\n        \n        /* Check bounds before reading compressed data (CWE-119) */\n        currentPos = ftell(zipFile);\n        if (currentPos < 0 || header.compressedSize > MAX_FILE_SIZE ||\n            currentPos + header.compressedSize > fileSize) {\n            fprintf(stderr, "Invalid compressed size\\n");\n            free(filename);\n            filename = NULL;\n            success = false;\n            break;\n        }\n        \n        /* Allocate buffer for compressed data */\n        compressedData = (uint8_t*)safeMalloc(header.compressedSize);\n        if (compressedData == NULL) {\n            free(filename);\n            filename = NULL;\n            success = false;\n            break;\n        }\n        \n        /* Read compressed data with bounds check */\n        bytesRead = fread(compressedData, 1, header.compressedSize, zipFile);\n        if (bytesRead != header.compressedSize) {\n            fprintf(stderr, "Cannot read file data\\n");\n            free(compressedData);\n            compressedData = NULL;\n            free(filename);\n            filename = NULL;\n            success = false;\n            break;\n        }\n        \n        /* Only support stored (uncompressed) files */\n        if (header.compression != 0) {\n            fprintf(stderr, "Compressed files not supported: %s\\n", filename);\n            free(compressedData);\n            compressedData = NULL;\n            free(filename);\n            filename = NULL;\n            continue;\n        }\n        \n        /* Construct output path safely using snprintf (CWE-119) */\n        if (snprintf(outputPath, sizeof(outputPath), "%s/%s", \n                     outputDir, filename) >= (int)sizeof(outputPath)) {\n            fprintf(stderr, "Output path too long\\n");\n            free(compressedData);\n            compressedData = NULL;\n            free(filename);\n            filename = NULL;\n            success = false;\n            break;\n        }\n        outputPath[sizeof(outputPath) - 1] = '\\0';
        
        /* Write file atomically to prevent TOCTOU (CWE-367) */
        if (!safeWriteFile(outputPath, compressedData, header.compressedSize)) {
            fprintf(stderr, "Failed to write file: %s\\n", outputPath);
            free(compressedData);
            compressedData = NULL;
            free(filename);
            filename = NULL;
            success = false;
            break;
        }
        
        totalExtracted += header.uncompressedSize;
        printf("Extracted: %s\\n", filename);
        
        /* Clean up allocated memory for this iteration */
        free(compressedData);
        compressedData = NULL;
        free(filename);
        filename = NULL;
    }
    
    /* Close file - check return value */
    if (fclose(zipFile) != 0) {
        fprintf(stderr, "File close failed\\n");
        return false;
    }
    
    return success;
}

int main(void) {
    /* Test case 1: Valid ZIP extraction */
    printf("=== Test 1: Valid ZIP extraction ===\\n");
    if (extractZip("test.zip", "output1")) {
        printf("Test 1 passed\\n");
    } else {
        printf("Test 1 failed\\n");
    }
    
    /* Test case 2: Path traversal attempt (should fail) */
    printf("\\n=== Test 2: Path traversal safety ===\\n");
    if (!isPathSafe("../../../etc/passwd", "output2")) {
        printf("Test 2 passed: Path traversal blocked\\n");
    } else {
        printf("Test 2 failed: Path traversal not blocked\\n");
    }
    
    /* Test case 3: Absolute path attempt (should fail) */
    printf("\\n=== Test 3: Absolute path safety ===\\n");
    if (!isPathSafe("/tmp/test.txt", "output3")) {
        printf("Test 3 passed: Absolute path blocked\\n");
    } else {
        printf("Test 3 failed: Absolute path not blocked\\n");
    }
    
    /* Test case 4: Valid relative path (should pass) */
    printf("\\n=== Test 4: Valid relative path ===\\n");
    if (isPathSafe("subdir/file.txt", "output4")) {
        printf("Test 4 passed: Valid path accepted\\n");
    } else {
        printf("Test 4 failed: Valid path rejected\\n");
    }
    
    /* Test case 5: Empty input validation */
    printf("\\n=== Test 5: Empty input validation ===\\n");
    if (!extractZip("", "output5")) {
        printf("Test 5 passed: Empty input rejected\\n");
    } else {
        printf("Test 5 failed: Empty input accepted\\n");
    }
    
    return 0;
}
