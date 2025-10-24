/*
 * NOTE: This code requires the libcurl library.
 * You can install it on Debian/Ubuntu with: sudo apt-get install libcurl4-openssl-dev
 * On RedHat/CentOS: sudo yum install libcurl-devel
 *
 * To compile and run:
 * g++ -std=c++11 -o ftp_downloader_cpp Task77.cpp -lcurl
 * ./ftp_downloader_cpp <hostname> <user> <pass> <remote_file> <local_file>
 */
#include <iostream>
#include <string>
#include <fstream>
#include <memory>
#include <curl/curl.h>

// Callback function for libcurl to write received data into a file
static size_t write_data_callback(void *ptr, size_t size, size_t nmemb, void *stream) {
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
 * @return true if download was successful, false otherwise.
 */
bool downloadFileFromFtp(const std::string& host, const std::string& user, const std::string& password, const std::string& remoteFile, const std::string& localFile) {
    CURL *curl;
    CURLcode res;
    FILE *outfile;

    std::string url = "ftp://" + host + "/" + remoteFile;
    std::string userpwd = user + ":" + password;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        outfile = fopen(localFile.c_str(), "wb");
        if (outfile == nullptr) {
            std::cerr << "Error opening local file for writing: " << localFile << std::endl;
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return false;
        }

        // Set the URL
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        // Set the user and password
        curl_easy_setopt(curl, CURLOPT_USERPWD, userpwd.c_str());
        // Set the write callback function
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data_callback);
        // Set the data pointer for the callback (the file handle)
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, outfile);
        // Enable verbose for debugging
        // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        std::cout << "Downloading " << url << " to " << localFile << std::endl;
        res = curl_easy_perform(curl);
        
        fclose(outfile);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            // Remove partially downloaded file on error
            remove(localFile.c_str());
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return false;
        } else {
            std::cout << "Download successful!" << std::endl;
        }

        curl_easy_cleanup(curl);
    } else {
        std::cerr << "Could not initialize curl." << std::endl;
        curl_global_cleanup();
        return false;
    }

    curl_global_cleanup();
    return true;
}

class Task77 {
public:
    static void main(int argc, char *argv[]) {
        /*
        * Main function to run the FTP download from command line arguments.
        *
        * 5 Test Cases (using public FTP servers):
        *
        * Test Case 1: Download readme.txt from test.rebex.net
        * ./ftp_downloader_cpp test.rebex.net demo password readme.txt rebex_readme.txt
        *
        * Test Case 2: Download a test file from ftp.dlptest.com
        * ./ftp_downloader_cpp ftp.dlptest.com dlpuser rNrKYTX9g7z3RgJR test-file-100kb.txt dlp_test_100kb.txt
        *
        * Test Case 3: Download a different file from test.rebex.net
        * ./ftp_downloader_cpp test.rebex.net demo password pub/example/readme.txt rebex_pub_readme.txt
        *
        * Test Case 4: (Dummy) Download a backup file from a private server
        * ./ftp_downloader_cpp my-private-ftp.net admin secretpass backups/2023.zip backup.zip
        *
        * Test Case 5: (Dummy) Download a log file from a local network server
        * ./ftp_downloader_cpp 192.168.1.100 localuser 12345 logs/app.log app_log_today.log
        *
        */
        if (argc < 6) {
            std::cerr << "Usage: " << argv[0] << " <hostname> <username> <password> <remote_filepath> <local_filename>" << std::endl;
            exit(1);
        }

        std::string hostname = argv[1];
        std::string username = argv[2];
        std::string password = argv[3];
        std::string remoteFile = argv[4];
        std::string localFile = argv[5];

        downloadFileFromFtp(hostname, username, password, remoteFile, localFile);
    }
};

int main(int argc, char *argv[]) {
    Task77::main(argc, argv);
    return 0;
}