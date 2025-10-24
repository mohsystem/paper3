#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <curl/curl.h>

#define MAX_FILENAME_LEN 255
#define MAX_URL_LEN 2048
#define MAX_TEMP_FILENAME_LEN (MAX_FILENAME_LEN + 5) // For ".part" and null terminator

/**
 * @brief Validates a filename against a strict whitelist of characters.
 *
 * It allows only alphanumeric characters, '.', '_', and '-'. It also checks
 * for a reasonable length and disallows any path traversal sequences.
 *
 * @param filename The filename to validate.
 * @return true if the filename is valid, false otherwise.
 */
bool is_valid_filename(const char *filename) {
    if (!filename || filename[0] == '\0') {
        return false;
    }

    size_t len = strlen(filename);
    if (len == 0 || len > MAX_FILENAME_LEN) {
        return false;
    }

    // Check for directory traversal sequences.
    if (strstr(filename, "..")) {
        return false;
    }

    for (size_t i = 0; i < len; ++i) {
        // Whitelist of allowed characters. Disallow path separators.
        if (!isalnum((unsigned char)filename[i]) &&
            filename[i] != '.' &&
            filename[i] != '_' &&
            filename[i] != '-') {
            return false;
        }
    }

    return true;
}

/**
 * @brief libcurl callback function for writing received data to a file.
 *
 * This function is called by libcurl as data is received. It writes the data
 * to the provided file stream.
 *
 * @param ptr Pointer to the received data.
 * @param size Size of each data element.
 * @param nmemb Number of data elements.
 * @param stream A FILE* pointer to the output file.
 * @return The number of bytes successfully written. If this does not match
 *         the number of bytes passed in, libcurl will abort the transfer.
 */
static size_t write_data_callback(void *ptr, size_t size, size_t nmemb, void *stream) {
    FILE *outfile = (FILE *)stream;
    size_t written = fwrite(ptr, size, nmemb, outfile);
    if (written < nmemb) {
        fprintf(stderr, "fwrite error: Failed to write full block to file.\n");
        return 0; // Signal an error to libcurl
    }
    return written;
}

/**
 * @brief Retrieves a file from a server using HTTPS.
 *
 * This function constructs a URL, performs an HTTPS GET request using libcurl,
 * and saves the file locally. It validates the filename, uses a temporary
 * file for the download, and performs an atomic rename upon success.
 *
 * @param base_url The base URL of the server (e.g., "https://example.com/files/").
 * @param filename The name of the file to retrieve.
 * @return true on success, false on failure.
 */
bool retrieve_file(const char *base_url, const char *filename) {
    if (!base_url || !filename) {
        fprintf(stderr, "Error: Base URL or filename is NULL.\n");
        return false;
    }

    // 1. Input Validation
    if (!is_valid_filename(filename)) {
        fprintf(stderr, "Error: Invalid filename provided: '%s'. Aborting.\n", filename);
        return false;
    }

    // 2. URL Construction
    char full_url[MAX_URL_LEN];
    int url_len = snprintf(full_url, sizeof(full_url), "%s%s", base_url, filename);
    if (url_len < 0 || (size_t)url_len >= sizeof(full_url)) {
        fprintf(stderr, "Error: Failed to construct URL or URL is too long.\n");
        return false;
    }

    CURL *curl_handle = NULL;
    FILE *outfile = NULL;
    char temp_filename[MAX_TEMP_FILENAME_LEN];
    bool success = false;

    // 3. Create a temporary filename to avoid partial files on failure
    int temp_len = snprintf(temp_filename, sizeof(temp_filename), "%s.part", filename);
    if (temp_len < 0 || (size_t)temp_len >= sizeof(temp_filename)) {
        fprintf(stderr, "Error: Failed to create temporary filename.\n");
        return false;
    }

    // Open temporary file for writing in binary mode.
    outfile = fopen(temp_filename, "wb");
    if (!outfile) {
        perror("Error opening temporary file for writing");
        return false;
    }

    curl_handle = curl_easy_init();
    if (!curl_handle) {
        fprintf(stderr, "Error: curl_easy_init() failed.\n");
        goto cleanup;
    }

    // 4. Set libcurl options for a secure download
    curl_easy_setopt(curl_handle, CURLOPT_URL, full_url);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data_callback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, outfile);

    // Security settings
    curl_easy_setopt(curl_handle, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS); // Allow HTTPS only
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 1L); // Verify server certificate
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 2L); // Verify hostname in certificate
    curl_easy_setopt(curl_handle, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2); // Enforce TLS 1.2+
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 0L); // Do not follow redirects

    // Operational settings
    curl_easy_setopt(curl_handle, CURLOPT_FAILONERROR, 1L); // Fail on HTTP status codes >= 400
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 30L); // 30-second timeout

    // 5. Perform the file transfer
    CURLcode res = curl_easy_perform(curl_handle);

    // 6. Finalize file operations
    fclose(outfile);
    outfile = NULL; // Mark as closed

    if (res == CURLE_OK) {
        // 7. On success, atomically rename the temporary file to the final name
        if (rename(temp_filename, filename) != 0) {
            perror("Error renaming temporary file");
            remove(temp_filename); // Attempt to clean up
        } else {
            printf("Successfully downloaded and saved '%s'.\n", filename);
            success = true;
        }
    } else {
        // 8. On failure, print error and remove the temporary file
        fprintf(stderr, "Download failed: %s\n", curl_easy_strerror(res));
        remove(temp_filename);
    }

cleanup:
    if (outfile) {
        fclose(outfile); // Ensure file is closed on error paths
        remove(temp_filename); // Clean up temp file on error
    }
    if (curl_handle) {
        curl_easy_cleanup(curl_handle);
    }
    return success;
}

int main(void) {
    // Global libcurl initialization.
    if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
        fprintf(stderr, "Fatal: curl_global_init() failed.\n");
        return EXIT_FAILURE;
    }

    const char *base_url = "https://raw.githubusercontent.com/curl/curl/master/";

    // --- Test Cases ---
    const char *test_cases[] = {
        // 1. Valid and existing file
        "README.md",
        // 2. Another valid file
        "COPYING",
        // 3. Valid filename format, but file does not exist (will cause 404)
        "non-existent-file-123.txt",
        // 4. Invalid filename with path traversal
        "../../etc/passwd",
        // 5. Invalid filename with disallowed characters (slash)
        "docs/INSTALL.md"
    };
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);

    for (int i = 0; i < num_tests; ++i) {
        printf("\n--- Test Case %d: Fetching '%s' ---\n", i + 1, test_cases[i]);
        bool result = retrieve_file(base_url, test_cases[i]);
        printf("Result: %s\n", result ? "SUCCESS" : "FAILURE");
    }

    // Global libcurl cleanup.
    curl_global_cleanup();

    return EXIT_SUCCESS;
}