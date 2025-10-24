#include <iostream>
#include <string>
#include <vector>
#include <cstdio>
#include <curl/curl.h>

// Note: This code requires libcurl.
// Compile with: g++ task.cpp -o task -lcurl

/**
 * @brief Extracts a safe basename from a file path to prevent path traversal.
 * @param path The full remote path.
 * @return A safe filename, or an empty string if the path is invalid.
 */
std::string get_safe_basename(const std::string& path) {
    if (path.empty()) {
        return "";
    }
    
    size_t last_slash = path.find_last_of("/\\");
    std::string basename = (last_slash == std::string::npos) ? path : path.substr(last_slash + 1);

    // Rule #11: Further validation to ensure it's a simple filename.
    if (basename.empty() || basename.find('/') != std::string::npos || basename.find('\\') != std::string::npos || basename == "." || basename == "..") {
        return "";
    }
    return basename;
}


/**
 * @brief Callback function for libcurl to write received data to a file.
 */
static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
    size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
    return written;
}

/**
 * @brief Connects to an FTP server using FTPS and downloads a file.
 * @param hostname The FTP server hostname.
 * @param username The username for authentication.
 * @param password The password for authentication.
 * @param remote_file_path The full path of the file to download.
 * @return true on success, false on failure.
 */
bool download_file(const std::string& hostname, const std::string& username, const std::string& password, const std::string& remote_file_path) {
    std::string local_filename = get_safe_basename(remote_file_path);
    if (local_filename.empty()) {
        std::cerr << "Error: Invalid remote file path." << std::endl;
        return false;
    }

    CURL *curl;
    CURLcode res = CURLE_OK;
    FILE *local_file = nullptr;
    bool success = false;

    // Rule #15: Download to a temporary file
    std::string temp_filename = local_filename + ".tmp";

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (!curl) {
        std::cerr << "Error: Failed to initialize libcurl." << std::endl;
        curl_global_cleanup();
        return false;
    }

    local_file = fopen(temp_filename.c_str(), "wb");
    if (!local_file) {
        std::cerr << "Error: Cannot open local file for writing: " << temp_filename << std::endl;
        goto cleanup;
    }
    
    std::string url = "ftps://" + hostname + "/" + remote_file_path;
    std::string userpwd = username + ":" + password;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_USERPWD, userpwd.c_str());
    curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
    
    // Rules #1, #2: The following two lines disable certificate verification,
    // which is INSECURE and for testing only. In production, remove these
    // and configure CURLOPT_CAINFO with your CA certificate bundle.
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, local_file);

    std::cout << "Downloading " << remote_file_path << " to " << local_filename << "..." << std::endl;
    res = curl_easy_perform(curl);
    
    fclose(local_file);
    local_file = nullptr;

    if (res != CURLE_OK) {
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        remove(temp_filename.c_str());
        goto cleanup;
    }

    // Rule #15: Atomically rename the temporary file to the final name
    if (rename(temp_filename.c_str(), local_filename.c_str()) != 0) {
        perror("Error renaming temporary file");
        remove(temp_filename.c_str());
        goto cleanup;
    }

    std::cout << "File downloaded successfully." << std::endl;
    success = true;

cleanup:
    if (local_file) fclose(local_file);
    if (curl) curl_easy_cleanup(curl);
    curl_global_cleanup();
    return success;
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <hostname> <username> <password> <remoteFilePath>" << std::endl;
        std::cerr << "WARNING: Providing a password on the command line is insecure." << std::endl;
        return 1;
    }

    std::cout << "--- Running Main Logic ---" << std::endl;
    download_file(argv[1], argv[2], argv[3], argv[4]);

    std::cout << "\n--- Mock Test Cases (will fail without a running FTP server) ---" << std::endl;

    // Test Case 1: Valid inputs (placeholders)
    std::cout << "\nTest Case 1: Valid inputs" << std::endl;
    download_file("ftp.example.com", "user", "pass", "/pub/file.txt");

    // Test Case 2: Invalid hostname
    std::cout << "\nTest Case 2: Invalid hostname" << std::endl;
    download_file("invalid.hostname.nonexistent", "user", "pass", "/pub/file.txt");

    // Test Case 3: Invalid credentials
    std::cout << "\nTest Case 3: Invalid credentials" << std::endl;
    download_file("ftp.example.com", "wronguser", "wrongpass", "/pub/file.txt");

    // Test Case 4: File not found
    std::cout << "\nTest Case 4: File not found" << std::endl;
    download_file("ftp.example.com", "user", "pass", "/nonexistent/file.txt");

    // Test Case 5: Path traversal attempt
    std::cout << "\nTest Case 5: Path traversal attempt" << std::endl;
    download_file("ftp.example.com", "user", "pass", "../../../etc/passwd");
    
    return 0;
}