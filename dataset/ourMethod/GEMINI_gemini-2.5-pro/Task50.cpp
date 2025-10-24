#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <memory>
#include <filesystem>
#include <cstdio>
#include <sys/stat.h>

#define CURL_STATICLIB
#include <curl/curl.h>

#ifdef _WIN32
#include <io.h>
#define fileno _fileno
#define fstat _fstat
#define stat _stat
#endif

// RAII wrapper for a global CURL initialization
struct CurlGlobalInitializer {
    CurlGlobalInitializer() {
        if (curl_global_init(CURL_GLOBAL_ALL) != 0) {
            throw std::runtime_error("Failed to initialize libcurl");
        }
    }
    ~CurlGlobalInitializer() {
        curl_global_cleanup();
    }
};

// RAII wrapper for a CURL easy handle
struct CurlHandle {
    CURL* handle;
    CurlHandle() : handle(curl_easy_init()) {
        if (!handle) {
            throw std::runtime_error("curl_easy_init() failed");
        }
    }
    ~CurlHandle() {
        if (handle) {
            curl_easy_cleanup(handle);
        }
    }
    // Forbid copying
    CurlHandle(const CurlHandle&) = delete;
    CurlHandle& operator=(const CurlHandle&) = delete;
    // Allow moving
    CurlHandle(CurlHandle&& other) noexcept : handle(other.handle) {
        other.handle = nullptr;
    }
    CurlHandle& operator=(CurlHandle&& other) noexcept {
        if (this != &other) {
            if (handle) {
                curl_easy_cleanup(handle);
            }
            handle = other.handle;
            other.handle = nullptr;
        }
        return *this;
    }
};

// Callback for writing response data from the server
static size_t write_callback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t realsize = size * nmemb;
    try {
        userp->append(static_cast<char*>(contents), realsize);
    } catch (const std::bad_alloc&) {
        // Not much we can do here, returning 0 will cause libcurl to abort.
        return 0;
    }
    return realsize;
}

std::string uploadFile(const std::string& url, const std::string& filePath) {
    if (url.empty() || filePath.empty()) {
        return "Error: URL or file path is empty.";
    }

    std::error_code ec;
    if (!std::filesystem::is_regular_file(filePath, ec) || ec) {
        return "Error: File path is not a regular file, does not exist, or a permissions error occurred.";
    }

    FILE* fp = fopen(filePath.c_str(), "rb");
    if (!fp) {
        return "Error: Could not open file for reading: " + filePath;
    }
    // Using a unique_ptr for RAII on the FILE handle
    std::unique_ptr<FILE, decltype(&fclose)> file_guard(fp, &fclose);

    struct stat file_info;
    if (fstat(fileno(fp), &file_info) != 0) {
        return "Error: fstat failed on the opened file.";
    }

    try {
        CurlHandle curl;
        std::string response_string;

        curl_easy_setopt(curl.handle, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(curl.handle, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl.handle, CURLOPT_READDATA, fp);
        curl_easy_setopt(curl.handle, CURLOPT_INFILESIZE_LARGE, (curl_off_t)file_info.st_size);

        curl_easy_setopt(curl.handle, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl.handle, CURLOPT_WRITEDATA, &response_string);

        // Security: Enable default SSL/TLS certificate verification
        curl_easy_setopt(curl.handle, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl.handle, CURLOPT_SSL_VERIFYHOST, 2L);
        // On Windows, libcurl might need a CA bundle path.
        // curl_easy_setopt(curl.handle, CURLOPT_CAINFO, "path/to/ca-bundle.crt");

        CURLcode res = curl_easy_perform(curl.handle);

        if (res != CURLE_OK) {
            return std::string("Error: curl_easy_perform() failed: ") + curl_easy_strerror(res);
        }

        long http_code = 0;
        curl_easy_getinfo(curl.handle, CURLINFO_RESPONSE_CODE, &http_code);

        if (http_code >= 200 && http_code < 300) {
            return "Success (" + std::to_string(http_code) + "). Server response: " + response_string;
        } else {
            return "Error: Server returned HTTP status " + std::to_string(http_code) + ". Response: " + response_string;
        }
    } catch (const std::exception& e) {
        return std::string("Error: An exception occurred: ") + e.what();
    }
}

void run_cpp_test_cases() {
    const std::string test_file_name = "test_upload.txt";
    const std::string test_dir_name = "test_dir_cpp";
    {
        std::ofstream test_file(test_file_name);
        test_file << "This is a C++ test file for upload." << std::endl;
    }
    std::filesystem::create_directory(test_dir_name);

    std::string url = "http://localhost:8080/upload-cpp.txt";
    
    std::cout << "--- C++ Test Case 1: Valid file upload ---" << std::endl;
    std::cout << uploadFile(url, test_file_name) << std::endl << std::endl;

    std::cout << "--- C++ Test Case 2: Non-existent file ---" << std::endl;
    std::cout << uploadFile(url, "nonexistent.txt") << std::endl << std::endl;

    std::cout << "--- C++ Test Case 3: Uploading a directory ---" << std::endl;
    std::cout << uploadFile(url, test_dir_name) << std::endl << std::endl;

    std::cout << "--- C++ Test Case 4: Empty file path ---" << std::endl;
    std::cout << uploadFile(url, "") << std::endl << std::endl;
    
    std::cout << "--- C++ Test Case 5: Empty URL ---" << std::endl;
    std::cout << uploadFile("", test_file_name) << std::endl << std::endl;

    std::error_code ec;
    std::filesystem::remove(test_file_name, ec);
    std::filesystem::remove(test_dir_name, ec);
}

int main() {
    // Note: To run this code, you need a server listening on http://localhost:8080
    // that accepts PUT requests. A simple Python server is provided in the thought process.
    try {
        CurlGlobalInitializer curl_initializer;
        run_cpp_test_cases();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}