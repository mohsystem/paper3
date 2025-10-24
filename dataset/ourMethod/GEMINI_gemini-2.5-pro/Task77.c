#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

/**
 * @brief Callback function for libcurl to write received data to a file.
 *
 * This function is called by libcurl as soon as there is data received
 * that needs to be saved.
 *
 * @param ptr Pointer to the received data.
 * @param size Size in bytes of each data element.
 * @param nmemb Number of data elements.
 * @param stream User-provided pointer, in this case a FILE* handle.
 * @return The number of bytes written. If this differs from the number of
 *         bytes passed in (size * nmemb), libcurl will abort the transfer.
 */
static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
    FILE *out = (FILE *)stream;
    if (!out) {
        fprintf(stderr, "Error: File stream is invalid in write_data.\n");
        return 0;
    }
    size_t written = fwrite(ptr, size, nmemb, out);
    if (written < nmemb) {
        fprintf(stderr, "Error: fwrite failed to write complete data chunk.\n");
    }
    // Return the number of bytes actually written.
    // If this is not equal to size*nmemb, curl will abort.
    return written * size;
}

/**
 * @brief Connects to an FTP server and downloads a file securely.
 *
 * @param hostname The FTP server hostname.
 * @param username The login username.
 * @param password The login password.
 * @param remote_file The full path to the file to download on the server.
 * @param local_file The local filename to save the downloaded file as.
 * @return 0 on success, -1 on failure.
 */
int download_ftp_file(const char *hostname, const char *username, const char *password, const char *remote_file, const char *local_file) {
    CURL *curl = NULL;
    FILE *local_fp = NULL;
    CURLcode res = CURLE_OK;
    char *url = NULL;
    int result = -1; // Default to error

    // Security: Validate local_file to prevent path traversal.
    if (strchr(local_file, '/') != NULL || strchr(local_file, '\\') != NULL) {
        fprintf(stderr, "Error: Invalid local filename '%s'. Path separators are not allowed.\n", local_file);
        return -1;
    }

    // Allocate buffer for the URL: "ftps://<hostname>/<remote_file>"
    size_t url_len = strlen("ftps://") + strlen(hostname) + 1 + strlen(remote_file) + 1;
    url = (char*)malloc(url_len);
    if (!url) {
        fprintf(stderr, "Error: Could not allocate memory for URL.\n");
        return -1;
    }
    // Use snprintf for safe string formatting.
    snprintf(url, url_len, "ftps://%s/%s", hostname, remote_file);

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Error: curl_easy_init() failed.\n");
        goto cleanup;
    }

    // Open the local file for writing in binary mode.
    local_fp = fopen(local_file, "wb");
    if (!local_fp) {
        perror("Error opening local file");
        goto cleanup;
    }

    // Set FTP options.
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_USERNAME, username);
    curl_easy_setopt(curl, CURLOPT_PASSWORD, password);

    // Enforce secure TLS connection.
    curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
    
    // Security: Verify the peer's SSL certificate.
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);

    // Security: Verify the server's name against the certificate.
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

    // Security: Enforce a minimum of TLS 1.2.
    curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);

    // Set the write callback function and data.
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, local_fp);
    
    // Tell curl to fail on server errors (e.g., file not found).
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

    // Perform the file transfer.
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        goto cleanup;
    }

    printf("File '%s' downloaded successfully to '%s'.\n", remote_file, local_file);
    result = 0; // Success

cleanup:
    if (local_fp) {
        fclose(local_fp);
    }
    if (curl) {
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    free(url);

    // If an error occurred, delete the potentially partial local file.
    if (result != 0) {
        remove(local_file);
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        // Run test cases if no arguments are provided.
        printf("Running built-in test cases...\n\n");
        
        // Public FTPS server for testing: test.rebex.net
        // Credentials: user=demo, pass=password
        const char *hostname = "test.rebex.net";
        const char *username = "demo";
        const char *password = "password";

        // Test Case 1: Download a small text file.
        printf("--- Test Case 1: Download readme.txt ---\n");
        download_ftp_file(hostname, username, password, "readme.txt", "readme_downloaded.txt");
        printf("\n");
        
        // Test Case 2: Download a binary file (image).
        printf("--- Test Case 2: Download a PNG image ---\n");
        download_ftp_file(hostname, username, password, "pub/example/Key-Pair.png", "image_downloaded.png");
        printf("\n");

        // Test Case 3: Attempt to download a non-existent file.
        printf("--- Test Case 3: File not found ---\n");
        download_ftp_file(hostname, username, password, "nonexistentfile.txt", "should_not_exist.txt");
        printf("\n");
        
        // Test Case 4: Invalid local filename (path traversal attempt).
        printf("--- Test Case 4: Invalid local filename ---\n");
        download_ftp_file(hostname, username, password, "readme.txt", "../invalid_path.txt");
        printf("\n");

        // Test Case 5: Incorrect password.
        printf("--- Test Case 5: Incorrect password ---\n");
        download_ftp_file(hostname, username, "wrongpassword", "readme.txt", "should_also_not_exist.txt");
        printf("\n");
        
        return 0;
    }

    if (argc != 6) {
        fprintf(stderr, "Usage: %s <hostname> <username> <password> <remote_file> <local_file>\n", argv[0]);
        fprintf(stderr, "\nWarning: Providing a password on the command line is insecure and should be avoided!\n");
        return 1;
    }
    
    // Acknowledge the security risk of command line credentials.
    fprintf(stdout, "Warning: Providing a password on the command line is insecure!\n\n");

    const char *hostname = argv[1];
    const char *username = argv[2];
    const char *password = argv[3];
    const char *remote_file = argv[4];
    const char *local_file = argv[5];

    if (download_ftp_file(hostname, username, password, remote_file, local_file) != 0) {
        fprintf(stderr, "FTP download failed.\n");
        return 1;
    }

    return 0;
}