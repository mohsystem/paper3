#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <curl/curl.h>

// Note: This code requires libcurl.
// To compile: gcc -o ftp_downloader_c Task77.c -lcurl

// Callback function to write received data to a file
static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
    size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
    return written;
}

// Security: Check for path traversal characters in the local filename.
bool is_safe_filename(const char* filename) {
    if (strstr(filename, "/") != NULL || strstr(filename, "\\") != NULL || strstr(filename, "..") != NULL) {
        return false;
    }
    return true;
}

/**
 * Connects to an FTP server using FTPS and downloads a file.
 *
 * @param host        The FTP server hostname or IP address.
 * @param username    The username for authentication.
 * @param password    The password for authentication.
 * @param remote_file The path to the file on the server.
 * @param local_file  The name of the file to save locally. Path traversal is prevented.
 * @return true on success, false on failure.
 */
bool downloadFileSecureFTP(const char* host, const char* username, const char* password,
                           const char* remote_file, const char* local_file) {
    
    if (!is_safe_filename(local_file)) {
        fprintf(stderr, "Error: Local filename '%s' contains invalid characters.\n", local_file);
        return false;
    }

    CURL *curl;
    CURLcode res = CURLE_OK;
    FILE *outfile = NULL;
    char url[512];
    char userpwd[256];

    // Construct the FTPS URL
    snprintf(url, sizeof(url), "ftps://%s/%s", host, remote_file);
    // Construct credentials
    snprintf(userpwd, sizeof(userpwd), "%s:%s", username, password);

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to initialize curl.\n");
        curl_global_cleanup();
        return false;
    }

    outfile = fopen(local_file, "wb");
    if (!outfile) {
        fprintf(stderr, "Error opening local file for writing.\n");
        res = CURLE_WRITE_ERROR;
        goto cleanup;
    }

    // Set FTPS URL
    curl_easy_setopt(curl, CURLOPT_URL, url);
    // Set credentials
    curl_easy_setopt(curl, CURLOPT_USERPWD, userpwd);
    // Enforce FTPS (explicit TLS)
    curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
    // For security, verify the peer's certificate.
    // For self-signed certs, you might need CURLOPT_SSL_VERIFYPEER, 0L but this is not recommended.
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    // Set the write callback function and data
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, outfile);
    
    printf("Starting download from %s to %s\n", url, local_file);
    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    } else {
        printf("Download successful.\n");
    }

cleanup:
    if (outfile) {
        fclose(outfile);
    }
    if (curl) {
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    
    return res == CURLE_OK;
}

int main(int argc, char *argv[]) {
    // SECURITY WARNING: Passing credentials as command-line arguments is insecure.
    // They can be viewed by other users on the system via the process list and
    // may be stored in shell history. In a production environment, use a more
    // secure method like environment variables, a secure vault, or interactive prompts.
    if (argc != 6) {
        fprintf(stderr, "Usage: %s <host> <username> <password> <remote_file_path> <local_file_name>\n", argv[0]);
        fprintf(stderr, "\n--- Test Case Examples (replace with actual server details) ---\n");
        fprintf(stderr, "1. Download 'report.csv' from 'ftp.example.com':\n");
        fprintf(stderr, "   %s ftp.example.com user1 pass1 reports/report.csv report_local.csv\n", argv[0]);
        fprintf(stderr, "2. Download 'archive.zip' from an IP address:\n");
        fprintf(stderr, "   %s 192.168.1.100 admin secret backups/archive.zip backup.zip\n", argv[0]);
        fprintf(stderr, "3. Download 'image.jpg' from a public test server (credentials may vary):\n");
        fprintf(stderr, "   %s test.rebex.net demo password pub/example/image.jpg example.jpg\n", argv[0]);
        fprintf(stderr, "4. Attempt to download a non-existent file:\n");
        fprintf(stderr, "   %s ftp.example.com user1 pass1 path/to/nonexistent.file missing.txt\n", argv[0]);
        fprintf(stderr, "5. Use invalid credentials:\n");
        fprintf(stderr, "   %s ftp.example.com wronguser wrongpass reports/report.csv report.csv\n", argv[0]);
        return 1;
    }

    downloadFileSecureFTP(argv[1], argv[2], argv[3], argv[4], argv[5]);

    return 0;
}