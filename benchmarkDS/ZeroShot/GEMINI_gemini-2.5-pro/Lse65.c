/*
 * IMPORTANT: This code requires the libcurl library.
 *
 * To compile on Linux/macOS:
 * gcc -o uploader_c Lse65.c -lcurl
 *
 * To compile on Windows (with MinGW and pre-compiled curl):
 * gcc -o uploader_c.exe Lse65.c -lcurl -lws2_32 -lwldap32
 *
 * You must have libcurl installed on your system.
 * The test cases will only work if a server (like the one in the Python or Java example)
 * is running and listening on the specified URL.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#define MKDIR(path) CreateDirectory(path, NULL)
#define RMDIR(path) RemoveDirectory(path) // Only works on empty dirs
#else
#include <unistd.h>
#define MKDIR(path) mkdir(path, 0755)
#define RMDIR(path) rmdir(path)
#endif

/**
 * @brief Checks if a file exists and is a regular file.
 *
 * @param filename The path to the file.
 * @return 1 if it exists and is a regular file, 0 otherwise.
 */
int file_exists(const char *filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0 && S_ISREG(buffer.st_mode));
}

/**
 * @brief Uploads a PDF file to a server using a multipart/form-data POST request.
 *
 * @param server_url The URL of the server endpoint (e.g., "http://localhost:8080/upload").
 * @param pdf_file_path The path to the PDF file to upload.
 * @return 1 on success (HTTP 2xx response), 0 on failure.
 */
int uploadPdfFile(const char* server_url, const char* pdf_file_path) {
    if (!file_exists(pdf_file_path)) {
        fprintf(stderr, "Error: File does not exist or is not a regular file: %s\n", pdf_file_path);
        return 0;
    }

    CURL *curl;
    CURLcode res;
    int success = 0;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if (curl) {
        curl_mime *mime = curl_mime_init(curl);
        curl_mimepart *part = curl_mime_addpart(mime);

        // Set the file part
        curl_mime_name(part, "file");
        curl_mime_filedata(part, pdf_file_path);
        curl_mime_type(part, "application/pdf");

        // Set curl options
        curl_easy_setopt(curl, CURLOPT_URL, server_url);
        curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirects
        curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);    // Fail on HTTP >= 400

        // Perform the request
        res = curl_easy_perform(curl);

        if (res == CURLE_OK) {
            long http_code = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
            // Consider 2xx as success
            if (http_code >= 200 && http_code < 300) {
                success = 1;
            } else {
                fprintf(stderr, "Upload failed with HTTP status code: %ld\n", http_code);
            }
        } else {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }

        // Cleanup
        curl_mime_free(mime);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return success;
}

void run_test(int test_num, const char* url, const char* temp_dir) {
    char test_file[256];
    snprintf(test_file, sizeof(test_file), "%s/test_c_%d.pdf", temp_dir, test_num);
    
    FILE *fp = fopen(test_file, "w");
    if (!fp) {
        fprintf(stderr, "Failed to create test file: %s\n", test_file);
        return;
    }
    fprintf(fp, "Dummy C PDF content for test %d", test_num);
    fclose(fp);

    printf("\n--- Test Case %d ---\n", test_num);
    printf("Uploading file: %s\n", test_file);

    if (uploadPdfFile(url, test_file)) {
        printf("Result: Upload successful.\n");
    } else {
        printf("Result: Upload failed.\n");
    }

    remove(test_file); // Clean up the individual test file
}

int main(void) {
    // NOTE: A server must be running at this URL for the tests to succeed.
    // You can run the Java or Python example file to start a compatible server.
    const char* UPLOAD_URL = "http://localhost:8080/upload";
    const char* TEMP_DIR = "c_test_files";
    
    printf("C PDF Uploader\n");
    printf("Make sure a server is running at %s\n", UPLOAD_URL);

    // Create a temporary directory for test files
    MKDIR(TEMP_DIR);
    
    // --- Run 5 Test Cases ---
    for (int i = 1; i <= 5; ++i) {
        run_test(i, UPLOAD_URL, TEMP_DIR);
    }

    // --- Cleanup ---
    printf("\nCleaning up temporary test directory...\n");
    RMDIR(TEMP_DIR);
    printf("Cleanup complete.\n");

    return 0;
}