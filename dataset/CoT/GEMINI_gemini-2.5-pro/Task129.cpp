/*
 * NOTE: This code requires the libcurl library.
 * To compile: g++ your_source_file.cpp -o your_executable -lcurl
 * On Windows, you may need to configure include and library paths for libcurl.
 * You may also need to create the save directory manually before running.
 */
#include <iostream>
#include <string>
#include <fstream>
#include <curl/curl.h>

// Callback function for libcurl to write received data into a file stream
static size_t write_callback(void *ptr, size_t size, size_t nmemb, void *stream) {
    std::ofstream *outFile = static_cast<std::ofstream*>(stream);
    if (outFile && outFile->is_open()) {
        outFile->write(static_cast<const char*>(ptr), size * nmemb);
        return size * nmemb;
    }
    return 0; // Signal an error if stream is not valid
}

/**
 * @brief Retrieves a file from a server and saves it locally.
 *
 * @param baseUrl The base URL of the server directory.
 * @param fileName The name of the file to retrieve (must be a simple name).
 * @param saveDir The local directory to save the file in.
 * @return true if successful, false otherwise.
 */
bool retrieveFile(const std::string& baseUrl, const std::string& fileName, const std::string& saveDir) {
    // 1. Security: Input validation to prevent Path Traversal
    if (fileName.empty() ||
        fileName.find('/') != std::string::npos ||
        fileName.find('\\') != std::string::npos ||
        fileName.find("..") != std::string::npos) {
        std::cerr << "Error: Invalid file name. Must be a simple name without path separators." << std::endl;
        return false;
    }

    CURL *curl_handle = curl_easy_init();
    if (!curl_handle) {
        std::cerr << "Error: Failed to initialize libcurl." << std::endl;
        return false;
    }

    // 2. Security: Construct URL from a fixed base to prevent SSRF
    std::string url = baseUrl;
    if (url.back() != '/') {
        url += '/';
    }
    url += fileName;

    std::string savePath = saveDir;
    if (savePath.back() != '/' && savePath.back() != '\\') {
        savePath += '/'; // Simple platform-agnostic separator
    }
    savePath += fileName;
    
    std::ofstream outFile(savePath, std::ios::binary);
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open local file for writing: " << savePath << std::endl;
        curl_easy_cleanup(curl_handle);
        return false;
    }

    // Set libcurl options
    curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &outFile);
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirects
    // 3. Security: Set timeout to prevent DoS
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 10L);

    bool success = false;
    CURLcode res = curl_easy_perform(curl_handle);

    outFile.close(); // Close the file before checking results

    if (res == CURLE_OK) {
        long http_code = 0;
        curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &http_code);
        if (http_code == 200) {
            std::cout << "File downloaded successfully to: " << savePath << std::endl;
            success = true;
        } else {
            std::cerr << "Server responded with code: " << http_code << " for URL: " << url << std::endl;
        }
    } else {
        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
    }

    curl_easy_cleanup(curl_handle);

    // If download failed, remove the potentially incomplete/empty file
    if (!success) {
        remove(savePath.c_str());
    }

    return success;
}

int main() {
    curl_global_init(CURL_GLOBAL_ALL);

    // Base URL of a public repository for testing purposes
    const std::string baseUrl = "https://raw.githubusercontent.com/gittar-und-fuchtel/md5-python-java-c/master/";
    // Local directory to save downloaded files. Ensure this directory exists.
    const std::string saveDir = "downloads_cpp";

    std::cout << "--- Running C++ Test Cases ---" << std::endl;
    std::cout << "Note: Please create the directory '" << saveDir << "' manually if it doesn't exist." << std::endl;
    
    // Test Case 1: Valid file
    std::cout << "\n[Test 1: Valid File]" << std::endl;
    retrieveFile(baseUrl, "README.md", saveDir);

    // Test Case 2: File not found
    std::cout << "\n[Test 2: Non-existent File]" << std::endl;
    retrieveFile(baseUrl, "non_existent_file.txt", saveDir);

    // Test Case 3: Path Traversal attempt (should be blocked)
    std::cout << "\n[Test 3: Path Traversal Attempt]" << std::endl;
    retrieveFile(baseUrl, "../some_other_file.txt", saveDir);

    // Test Case 4: Absolute path attempt (should be blocked)
    std::cout << "\n[Test 4: Absolute Path Attempt]" << std::endl;
    retrieveFile(baseUrl, "/etc/passwd", saveDir);

    // Test Case 5: Empty filename (should be blocked)
    std::cout << "\n[Test 5: Empty Filename]" << std::endl;
    retrieveFile(baseUrl, "", saveDir);

    curl_global_cleanup();
    return 0;
}