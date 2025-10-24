/*
 * NOTE: This code requires the libcurl library.
 * To compile: gcc your_source_file.c -o your_executable -lcurl
 * On Windows, you may need to configure include and library paths for libcurl.
 * You may also need to create the save directory manually before running.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>

// Callback function for libcurl to write received data to a FILE pointer
static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
    size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
    return written;
}

/**
 * @brief Retrieves a file from a server and saves it locally.
 *
 * @param baseUrl The base URL of the server directory.
 * @param fileName The name of the file to retrieve (must be a simple name).
 * @param saveDir The local directory to save the file in.
 * @return 1 if successful, 0 otherwise.
 */
int retrieveFile(const char* baseUrl, const char* fileName, const char* saveDir) {
    // 1. Security: Input validation to prevent Path Traversal
    if (fileName == NULL || *fileName == '\0' ||
        strchr(fileName, '/') != NULL ||
        strchr(fileName, '\\') != NULL ||
        strstr(fileName, "..") != NULL) {
        fprintf(stderr, "Error: Invalid file name. Must be a simple name without path separators.\n");
        return 0; // 0 for failure
    }

    CURL *curl_handle = curl_easy_init();
    if (!curl_handle) {
        fprintf(stderr, "Error: Failed to initialize libcurl.\n");
        return 0;
    }

    char url[1024];
    char savePath[1024];

    // 2. Security: Safely construct URL from a fixed base to prevent SSRF
    snprintf(url, sizeof(url), "%s%s%s", baseUrl,
             (baseUrl[strlen(baseUrl) - 1] == '/' ? "" : "/"), fileName);
    
    // Safely construct local save path
    snprintf(savePath, sizeof(savePath), "%s%s%s", saveDir,
             (saveDir[strlen(saveDir) - 1] == '/' ? "" : "/"), fileName);

    FILE *outFile = fopen(savePath, "wb");
    if (!outFile) {
        fprintf(stderr, "Error: Could not open local file for writing: %s\n", savePath);
        curl_easy_cleanup(curl_handle);
        return 0;
    }

    // Set libcurl options
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, outFile);
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L);
    // 3. Security: Set timeout to prevent DoS
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 10L);

    int success = 0;
    CURLcode res = curl_easy_perform(curl_handle);

    fclose(outFile);

    if (res == CURLE_OK) {
        long http_code = 0;
        curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &http_code);
        if (http_code == 200) {
            printf("File downloaded successfully to: %s\n", savePath);
            success = 1;
        } else {
            fprintf(stderr, "Server responded with code: %ld for URL: %s\n", http_code, url);
        }
    } else {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl_handle);

    // If download failed, remove the potentially incomplete/empty file
    if (!success) {
        remove(savePath);
    }
    
    return success;
}

int main(void) {
    curl_global_init(CURL_GLOBAL_ALL);

    // Base URL of a public repository for testing purposes
    const char* baseUrl = "https://raw.githubusercontent.com/gittar-und-fuchtel/md5-python-java-c/master/";
    // Local directory to save downloaded files. Ensure this directory exists.
    const char* saveDir = "downloads_c";

    printf("--- Running C Test Cases ---\n");
    printf("Note: Please create the directory '%s' manually if it doesn't exist.\n", saveDir);

    // Test Case 1: Valid file
    printf("\n[Test 1: Valid File]\n");
    retrieveFile(baseUrl, "README.md", saveDir);

    // Test Case 2: File not found
    printf("\n[Test 2: Non-existent File]\n");
    retrieveFile(baseUrl, "non_existent_file.txt", saveDir);

    // Test Case 3: Path Traversal attempt (should be blocked)
    printf("\n[Test 3: Path Traversal Attempt]\n");
    retrieveFile(baseUrl, "../some_other_file.txt", saveDir);

    // Test Case 4: Absolute path attempt (should be blocked)
    printf("\n[Test 4: Absolute Path Attempt]\n");
    retrieveFile(baseUrl, "/etc/passwd", saveDir);

    // Test Case 5: Empty filename (should be blocked)
    printf("\n[Test 5: Empty Filename]\n");
    retrieveFile(baseUrl, "", saveDir);

    curl_global_cleanup();
    return 0;
}