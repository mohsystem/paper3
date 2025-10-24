// To compile: g++ -std=c++17 Task129.cpp -o Task129 -lcurl -lstdc++fs
#include <iostream>
#include <string>
#include <regex>
#include <filesystem>
#include <fstream>
#include <memory>
#include <curl/curl.h>

namespace fs = std::filesystem;

const std::string DOWNLOAD_DIR = "downloads_cpp";

// RAII wrapper for CURL handle
struct CurlHandle {
    CURL* handle;
    CurlHandle() : handle(curl_easy_init()) {}
    ~CurlHandle() { if (handle) curl_easy_cleanup(handle); }
    operator CURL*() { return handle; }
};

// Callback function for libcurl to write received data into a file
size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    return fwrite(ptr, size, nmemb, stream);
}

bool is_valid_filename(const std::string& filename) {
    if (filename.empty()) {
        return false;
    }
    // Regex to validate filename. Allows alphanumeric, underscore, dot, hyphen.
    static const std::regex pattern("^[a-zA-Z0-9._-]+$");
    return std::regex_match(filename, pattern);
}

bool downloadFile(const std::string& baseUrl, const std::string& filename) {
    if (!is_valid_filename(filename)) {
        std::cerr << "Error: Invalid filename provided: " << filename << std::endl;
        return false;
    }

    fs::path downloadDirPath(DOWNLOAD_DIR);
    try {
        fs::create_directories(downloadDirPath);
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error: Could not create download directory: " << e.what() << std::endl;
        return false;
    }

    fs::path destinationPath = (downloadDirPath / filename).lexically_normal();
    if (destinationPath.string().find(downloadDirPath.string()) != 0) {
        std::cerr << "Error: Directory traversal attempt detected." << std::endl;
        return false;
    }

    CurlHandle curl;
    if (!curl.handle) {
        std::cerr << "Error: Failed to initialize libcurl." << std::endl;
        return false;
    }

    char* encoded_filename = curl_easy_escape(curl, filename.c_str(), filename.length());
    if (!encoded_filename) {
        std::cerr << "Error: Failed to URL-encode filename." << std::endl;
        return false;
    }
    std::string fullUrl = baseUrl + encoded_filename;
    curl_free(encoded_filename);

    fs::path tempPath = downloadDirPath / (filename + ".tmp");
    
    // Using C-style FILE* because it's what libcurl's callback expects
    FILE* fp = fopen(tempPath.c_str(), "wb");
    if (!fp) {
        std::cerr << "Error: Failed to create temporary file." << std::endl;
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, fullUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L); // Fail on 4xx/5xx responses

    CURLcode res = curl_easy_perform(curl);
    fclose(fp); // Close the file handle regardless of outcome

    if (res != CURLE_OK) {
        std::cerr << "Error: Failed to download file: " << curl_easy_strerror(res) << std::endl;
        fs::remove(tempPath); // Clean up temp file
        return false;
    }

    try {
        fs::rename(tempPath, destinationPath);
        std::cout << "Successfully downloaded: " << filename << " to " << destinationPath << std::endl;
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error: Failed to rename temporary file: " << e.what() << std::endl;
        fs::remove(tempPath); // Clean up temp file
        return false;
    }

    return true;
}

int main() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    std::string baseUrl = "https://raw.githubusercontent.com/git/git/master/";

    std::cout << "--- Test Case 1: Valid File ---" << std::endl;
    downloadFile(baseUrl, "README.md");

    std::cout << "\n--- Test Case 2: Non-existent File ---" << std::endl;
    downloadFile(baseUrl, "NON_EXISTENT_FILE.txt");

    std::cout << "\n--- Test Case 3: Path Traversal Attempt ---" << std::endl;
    downloadFile(baseUrl, "../../etc/passwd");
    
    std::cout << "\n--- Test Case 4: Invalid Characters ---" << std::endl;
    downloadFile(baseUrl, "file?name=value");

    std::cout << "\n--- Test Case 5: Empty Filename ---" << std::endl;
    downloadFile(baseUrl, "");

    curl_global_cleanup();
    return 0;
}