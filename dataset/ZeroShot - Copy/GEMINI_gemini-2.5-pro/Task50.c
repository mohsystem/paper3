#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <ctype.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define MKDIR(path) mkdir(path, 0755)
#endif

// --- Configuration ---
#define MAX_FILE_SIZE_BYTES (10 * 1024 * 1024) // 10 MB
#define UPLOAD_DIRECTORY "c_uploads"
#define MAX_FILENAME_LEN 256
#define UUID_STR_LEN 37

const char* ALLOWED_EXTENSIONS[] = {".txt", ".jpg", ".png", ".pdf", ".docx", NULL};

/**
 * Returns a dynamically allocated string with the result message.
 * The caller is responsible for freeing this memory.
 */
char* create_result_string(const char* format, const char* s1, const char* s2) {
    // Generously allocate memory for the result string
    size_t len1 = s1 ? strlen(s1) : 0;
    size_t len2 = s2 ? strlen(s2) : 0;
    char* buffer = (char*)malloc(strlen(format) + len1 + len2 + 1);
    if (!buffer) return NULL;
    if (s1 && s2) {
        sprintf(buffer, format, s1, s2);
    } else if (s1) {
        sprintf(buffer, format, s1);
    } else {
        sprintf(buffer, "%s", format);
    }
    return buffer;
}


/**
 * Securely "uploads" a file by saving it to a designated directory after security checks.
 *
 * @param originalFileName The name of the file provided by the user.
 * @param fileContent The byte content of the file.
 * @param contentSize The size of the file content in bytes.
 * @return A dynamically allocated string with a confirmation or error message. The caller MUST free this string.
 */
char* uploadFile(const char* originalFileName, const char* fileContent, size_t contentSize) {
    // 1. Check for null or empty inputs
    if (originalFileName == NULL || originalFileName[0] == '\0' || fileContent == NULL) {
        return strdup("Error: Invalid file name or content.");
    }
    if (contentSize == 0) {
        return strdup("Error: File content cannot be empty.");
    }
    
    // 2. Check file size
    if (contentSize > MAX_FILE_SIZE_BYTES) {
        char msg[100];
        sprintf(msg, "Error: File size exceeds the limit of %d MB.", (int)(MAX_FILE_SIZE_BYTES / (1024 * 1024)));
        return strdup(msg);
    }

    // 3. Sanitize filename to prevent path traversal
    const char *basename = strrchr(originalFileName, '/');
    const char *basename_bs = strrchr(originalFileName, '\\');
    if (basename_bs > basename) basename = basename_bs; // Handle both / and \
    if (basename == NULL) {
        basename = originalFileName; // No slashes
    } else {
        basename++; // Move past the slash
    }
    
    if (strlen(basename) == 0 || strcmp(basename, ".") == 0 || strcmp(basename, "..") == 0) {
        return strdup("Error: Invalid filename.");
    }


    // 4. Validate file extension against a whitelist
    const char *extension = strrchr(basename, '.');
    if (extension == NULL || extension == basename) {
        return strdup("Error: File must have an extension.");
    }
    
    char lower_ext[MAX_FILENAME_LEN];
    strncpy(lower_ext, extension, MAX_FILENAME_LEN - 1);
    lower_ext[MAX_FILENAME_LEN - 1] = '\0';
    for (int i = 0; lower_ext[i]; i++) {
        lower_ext[i] = tolower(lower_ext[i]);
    }

    bool isAllowed = false;
    for (int i = 0; ALLOWED_EXTENSIONS[i] != NULL; ++i) {
        if (strcmp(lower_ext, ALLOWED_EXTENSIONS[i]) == 0) {
            isAllowed = true;
            break;
        }
    }
    if (!isAllowed) {
        return strdup("Error: File type is not allowed.");
    }

    // 5. Generate a unique filename
    char unique_filename[MAX_FILENAME_LEN + UUID_STR_LEN];
    time_t t = time(NULL);
    sprintf(unique_filename, "%ld-%s", t, basename);

    // 6. Create upload directory if it doesn't exist
    MKDIR(UPLOAD_DIRECTORY);

    // 7. Define the destination path securely
    char destination_path[512];
    int path_len = snprintf(destination_path, sizeof(destination_path), "%s/%s", UPLOAD_DIRECTORY, unique_filename);
    if (path_len >= sizeof(destination_path)) {
        return strdup("Error: Destination path is too long.");
    }

    // 8. Write the file content
    FILE *outfile = fopen(destination_path, "wb");
    if (outfile == NULL) {
        return strdup("Error: Could not open file for writing.");
    }
    
    size_t written = fwrite(fileContent, 1, contentSize, outfile);
    fclose(outfile);
    
    if (written != contentSize) {
        return strdup("Error: Failed to write complete file.");
    }

    return create_result_string("Success: File '%s' uploaded as '%s'.", basename, unique_filename);
}

void run_test(int test_num, const char* test_name, const char* filename, const char* content, size_t size) {
    char* result = uploadFile(filename, content, size);
    printf("Test %d (%s): %s\n", test_num, test_name, result);
    free(result); // Free the message returned by uploadFile
}

int main() {
    printf("--- Running C File Upload Tests ---\n");
    MKDIR(UPLOAD_DIRECTORY);

    // Test Case 1: Successful upload of a valid text file
    const char* content1 = "This is a simple text file.";
    run_test(1, "Valid .txt", "mydocument.txt", content1, strlen(content1));
    
    // Test Case 2: Successful upload of a valid image file (simulated content)
    const char content2[] = {0x89, 'P', 'N', 'G', '\r', '\n', 0x1a, '\n'};
    run_test(2, "Valid .png", "photo.png", content2, sizeof(content2));

    // Test Case 3: Rejected upload due to disallowed file type
    const char* content3 = "#!/bin/bash\necho 'pwned'";
    run_test(3, "Invalid extension .sh", "malicious_script.sh", content3, strlen(content3));

    // Test Case 4: Rejected upload due to path traversal attempt in filename
    const char* content4 = "some data";
    run_test(4, "Path Traversal", "../../etc/passwd", content4, strlen(content4));

    // Test Case 5: Rejected upload due to file size limit
    char* large_content = (char*)malloc(MAX_FILE_SIZE_BYTES + 1);
    if (large_content) {
        memset(large_content, 0, MAX_FILE_SIZE_BYTES + 1);
        run_test(5, "File too large", "largefile.docx", large_content, MAX_FILE_SIZE_BYTES + 1);
        free(large_content);
    } else {
        printf("Test 5 (File too large): Could not allocate memory for test.\n");
    }

    printf("-----------------------------------\n");
    return 0;
}