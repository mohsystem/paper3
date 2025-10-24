/*
 * NOTE: This code requires the libcurl library.
 * You can install it on Debian/Ubuntu with: sudo apt-get install libcurl4-openssl-dev
 * On RedHat/CentOS: sudo yum install libcurl-devel
 *
 * To compile and run:
 * gcc -o ftp_downloader_c Task77.c -lcurl
 * ./ftp_downloader_c <hostname> <user> <pass> <remote_file> <local_file>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

// Callback function for libcurl to write received data into a file
static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
    size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
    return written;
}

/**
 * Connects to an FTP server and downloads a file using libcurl.
 *
 * @param host        The FTP server hostname.
 * @param user        The username for login.
 * @param password    The password for login.
 * @param remoteFile  The full path of the file to download from the server.
 * @param localFile   The name of the file to save locally.
 * @return 1 on success, 0 on failure.
 */
int downloadFileFromFtp(const char* host, const char* user, const char* password, const char* remoteFile, const char* localFile) {
    CURL *curl;
    CURLcode res;
    FILE *outfile;
    int success = 0;

    // Allocate memory for URL and user:pass strings
    // "ftp://" + host + "/" + remoteFile + \0
    char url[2048];
    snprintf(url, sizeof(url), "ftp://%s/%s", host, remoteFile);
    
    // user + ":" + password + \0
    char userpwd[512];
    snprintf(userpwd, sizeof(userpwd), "%s:%s", user, password);

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        outfile = fopen(localFile, "wb");
        if (outfile == NULL) {
            fprintf(stderr, "Error opening local file for writing: %s\n", localFile);
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return 0;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_USERPWD, userpwd);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, outfile);

        printf("Downloading %s to %s\n", url, localFile);
        res = curl_easy_perform(curl);

        fclose(outfile);

        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            remove(localFile); // Clean up partially downloaded file
        } else {
            printf("Download successful!\n");
            success = 1;
        }

        curl_easy_cleanup(curl);
    } else {
        fprintf(stderr, "Could not initialize curl.\n");
    }

    curl_global_cleanup();
    return success;
}

int main(int argc, char *argv[]) {
    /*
     * Main function to run the FTP download from command line arguments.
     *
     * 5 Test Cases (using public FTP servers):
     *
     * Test Case 1: Download readme.txt from test.rebex.net
     * ./ftp_downloader_c test.rebex.net demo password readme.txt rebex_readme.txt
     *
     * Test Case 2: Download a test file from ftp.dlptest.com
     * ./ftp_downloader_c ftp.dlptest.com dlpuser rNrKYTX9g7z3RgJR test-file-100kb.txt dlp_test_100kb.txt
     *
     * Test Case 3: Download a different file from test.rebex.net
     * ./ftp_downloader_c test.rebex.net demo password pub/example/readme.txt rebex_pub_readme.txt
     *
     * Test Case 4: (Dummy) Download a backup file from a private server
     * ./ftp_downloader_c my-private-ftp.net admin secretpass backups/2023.zip backup.zip
     *
     * Test Case 5: (Dummy) Download a log file from a local network server
     * ./ftp_downloader_c 192.168.1.100 localuser 12345 logs/app.log app_log_today.log
     *
     */
    if (argc < 6) {
        fprintf(stderr, "Usage: %s <hostname> <username> <password> <remote_filepath> <local_filename>\n", argv[0]);
        return 1;
    }

    const char *hostname = argv[1];
    const char *username = argv[2];
    const char *password = argv[3];
    const char *remoteFile = argv[4];
    const char *localFile = argv[5];

    if (downloadFileFromFtp(hostname, username, password, remoteFile, localFile)) {
        return 0; // Success
    } else {
        return 1; // Failure
    }
}