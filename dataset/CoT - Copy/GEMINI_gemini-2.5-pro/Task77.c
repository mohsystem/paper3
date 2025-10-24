#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

// Note: This code requires libcurl.
// To compile: gcc -o your_executable your_source.c -lcurl

// Helper function to get the base name of a path for security
const char* getBaseName(const char* path) {
    const char *base_nix = strrchr(path, '/');
    const char *base_win = strrchr(path, '\\');
    if (base_nix && base_win) {
        return (base_nix > base_win) ? base_nix + 1 : base_win + 1;
    }
    if (base_nix) return base_nix + 1;
    if (base_win) return base_win + 1;
    return path;
}

// Callback function for libcurl to write received data to a file
static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
    size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
    return written;
}

/**
 * Connects to an FTP server and downloads a file using libcurl.
 *
 * @param host The FTP server hostname or IP address.
 * @param username The username for authentication.
 * @param password The password for authentication.
 * @param remoteFile The full path of the file to download from the server.
 * @return 1 on success, 0 on failure.
 */
int downloadFile(const char* host, const char* username, const char* password, const char* remoteFile) {
    // WARNING: Standard FTP sends credentials and data in plaintext.
    // For production systems, always use FTPS (FTP over SSL/TLS) or SFTP.
    CURL *curl;
    CURLcode res = CURLE_OK;
    FILE *outfile = NULL;
    int success = 0;

    // Security: Prevent path traversal attacks.
    const char* localFileName = getBaseName(remoteFile);
    if (localFileName == NULL || *localFileName == '\0') {
        fprintf(stderr, "Error: Could not determine a local filename from remote path: %s\n", remoteFile);
        return 0;
    }

    char url[1024];
    char userpwd[256];

    // Safely construct the URL and user:pass strings to prevent buffer overflows
    snprintf(url, sizeof(url), "ftp://%s/%s", host, remoteFile);
    snprintf(userpwd, sizeof(userpwd), "%s:%s", username, password);

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl) {
        outfile = fopen(localFileName, "wb");
        if (!outfile) {
            fprintf(stderr, "Error: Could not open local file for writing: %s\n", localFileName);
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return 0;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_USERPWD, userpwd);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, outfile);
        curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L); // Fail on FTP errors >= 400

        printf("Downloading file %s to %s...\n", url, localFileName);
        res = curl_easy_perform(curl);
        
        fclose(outfile);

        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            // If download failed, remove the potentially empty/partial local file
            remove(localFileName);
        } else {
            printf("File downloaded successfully.\n");
            success = 1;
        }

        curl_easy_cleanup(curl);
    } else {
        fprintf(stderr, "curl_easy_init() failed.\n");
    }
    
    curl_global_cleanup();
    return success;
}

int main(int argc, char *argv[]) {
    // WARNING: Passing credentials as command-line arguments is insecure.
    // They can be viewed by other users on the system via the process list.
    // This is for demonstration purposes only.
    if (argc == 5) {
        downloadFile(argv[1], argv[2], argv[3], argv[4]);
    } else {
        printf("Running test cases. Please replace placeholder values if needed.\n");
        printf("Usage: ./your_executable <host> <username> <password> <remoteFilePath>\n");

        // Test cases use a public test FTP server.
        // Note: The remote files do not exist, so download will fail,
        // but connection and login should succeed, demonstrating the process.
        
        // Test Case 1: Example with valid-looking data
        printf("\n--- Test Case 1 ---\n");
        downloadFile("ftp.dlptest.com", "dlpuser", "rNrKYTX9g7z3RgJR", "some-file.txt");

        // Test Case 2: Example with a file in a subdirectory
        printf("\n--- Test Case 2 ---\n");
        downloadFile("ftp.dlptest.com", "dlpuser", "rNrKYTX9g7z3RgJR", "folder/some-other-file.zip");

        // Test Case 3: Invalid Host (will fail to connect)
        printf("\n--- Test Case 3 ---\n");
        downloadFile("invalid.ftp.host", "user", "pass", "file.txt");

        // Test Case 4: Invalid Credentials (will fail to login)
        printf("\n--- Test Case 4 ---\n");
        downloadFile("ftp.dlptest.com", "wronguser", "wrongpass", "file.txt");

        // Test Case 5: File Not Found (will fail to download)
        printf("\n--- Test Case 5 ---\n");
        downloadFile("ftp.dlptest.com", "dlpuser", "rNrKYTX9g7z3RgJR", "non-existent-file.dat");
    }
    return 0;
}