#include <iostream>
#include <string>
#include <cstdio>
#include <curl/curl.h>

// Note: This code requires libcurl.
// To compile: g++ -o your_executable your_source.cpp -lcurl

// Helper function to get the base name of a path for security
std::string getBaseName(const std::string& path) {
    size_t pos = path.find_last_of("/\\");
    if (pos != std::string::npos) {
        return path.substr(pos + 1);
    }
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
 * @return true if the file was downloaded successfully, false otherwise.
 */
bool downloadFile(const std::string& host, const std::string& username, const std::string& password, const std::string& remoteFile) {
    // WARNING: Standard FTP sends credentials and data in plaintext.
    // For production systems, always use FTPS (FTP over SSL/TLS) or SFTP.
    CURL *curl;
    CURLcode res = CURLE_OK;
    FILE *outfile = nullptr;

    // Security: Prevent path traversal attacks.
    // Use only the basename of the remote file for the local filename.
    std::string localFileName = getBaseName(remoteFile);
    if (localFileName.empty()) {
        std::cerr << "Error: Could not determine a local filename from remote path: " << remoteFile << std::endl;
        return false;
    }
    
    std::string url = "ftp://" + host + "/" + remoteFile;
    std::string userpwd = username + ":" + password;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl) {
        outfile = fopen(localFileName.c_str(), "wb");
        if (!outfile) {
            std::cerr << "Error: Could not open local file for writing: " << localFileName << std::endl;
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return false;
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_USERPWD, userpwd.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, outfile);
        curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L); // Fail on HTTP/FTP errors >= 400

        std::cout << "Downloading file " << url << " to " << localFileName << "..." << std::endl;
        res = curl_easy_perform(curl);
        
        fclose(outfile);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            // If download failed, remove the potentially empty/partial local file
            remove(localFileName.c_str());
        } else {
            std::cout << "File downloaded successfully." << std::endl;
        }

        curl_easy_cleanup(curl);
    } else {
        res = CURLE_FAILED_INIT;
    }
    
    curl_global_cleanup();
    return (res == CURLE_OK);
}

int main(int argc, char *argv[]) {
    // WARNING: Passing credentials as command-line arguments is insecure.
    // They can be viewed by other users on the system via the process list.
    // This is for demonstration purposes only.
    if (argc == 5) {
        downloadFile(argv[1], argv[2], argv[3], argv[4]);
    } else {
        std::cout << "Running test cases. Please replace placeholder values if needed." << std::endl;
        std::cout << "Usage: ./your_executable <host> <username> <password> <remoteFilePath>" << std::endl;

        // Test cases use a public test FTP server.
        // Note: The remote files do not exist, so download will fail,
        // but connection and login should succeed, demonstrating the process.

        // Test Case 1: Example with valid-looking data
        std::cout << "\n--- Test Case 1 ---" << std::endl;
        downloadFile("ftp.dlptest.com", "dlpuser", "rNrKYTX9g7z3RgJR", "some-file.txt");

        // Test Case 2: Example with a file in a subdirectory
        std::cout << "\n--- Test Case 2 ---" << std::endl;
        downloadFile("ftp.dlptest.com", "dlpuser", "rNrKYTX9g7z3RgJR", "folder/some-other-file.zip");

        // Test Case 3: Invalid Host (will fail to connect)
        std::cout << "\n--- Test Case 3 ---" << std::endl;
        downloadFile("invalid.ftp.host", "user", "pass", "file.txt");

        // Test Case 4: Invalid Credentials (will fail to login)
        std::cout << "\n--- Test Case 4 ---" << std::endl;
        downloadFile("ftp.dlptest.com", "wronguser", "wrongpass", "file.txt");

        // Test Case 5: File Not Found (will fail to download)
        std::cout << "\n--- Test Case 5 ---" << std::endl;
        downloadFile("ftp.dlptest.com", "dlpuser", "rNrKYTX9g7z3RgJR", "non-existent-file.dat");
    }
    return 0;
}