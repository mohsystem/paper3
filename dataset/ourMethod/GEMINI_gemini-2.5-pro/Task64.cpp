#include <iostream>
#include <string>
#include <stdexcept>
#include <memory>
#include <curl/curl.h>
// The nlohmann/json library is required.
// Download the single-header file 'json.hpp' from:
// https://github.com/nlohmann/json/releases
// and place it in the same directory or in your include path.
#include "json.hpp"

// This code requires libcurl.
//
// INSTALLATION:
// On Debian/Ubuntu: sudo apt-get install libcurl4-openssl-dev
// On Fedora/CentOS: sudo dnf install libcurl-devel
// On macOS (Homebrew): brew install curl
//
// COMPILE:
// g++ -std=c++17 -o task64 task64.cpp -lcurl

// Use nlohmann's json for convenience
using json = nlohmann::json;

// Callback function for libcurl to write received data into a std::string
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    try {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
    } catch (const std::bad_alloc& e) {
        std::cerr << "Memory allocation failed in WriteCallback: " << e.what() << std::endl;
        return 0; // Returning 0 will signal an error to libcurl
    }
    return size * nmemb;
}

// Custom deleter for CURL handle to be used with std::unique_ptr
struct CurlHandleDeleter {
    void operator()(CURL* handle) const {
        if (handle) {
            curl_easy_cleanup(handle);
        }
    }
};
using CurlUniquePtr = std::unique_ptr<CURL, CurlHandleDeleter>;

/**
 * @brief Fetches content from a URL and parses it as JSON.
 * @param url The HTTPS URL to fetch.
 * @return A nlohmann::json object.
 * @throws std::invalid_argument for invalid URLs.
 * @throws std::runtime_error for network or parsing failures.
 */
json fetchAndParseJson(const std::string& url) {
    if (url.empty()) {
        throw std::invalid_argument("URL cannot be empty.");
    }
    if (url.rfind("https://", 0) != 0) {
        throw std::invalid_argument("Invalid URL scheme. Only HTTPS is supported.");
    }

    CURL* curl_handle = curl_easy_init();
    if (!curl_handle) {
        throw std::runtime_error("Failed to initialize libcurl.");
    }
    CurlUniquePtr curl(curl_handle);

    std::string readBuffer;
    
    // Set libcurl options
    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 0L); // Do not follow redirects
    curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT, 10L); // 10-second timeout for the whole operation
    curl_easy_setopt(curl.get(), CURLOPT_CONNECTTIMEOUT, 5L); // 5-second timeout for connection
    curl_easy_setopt(curl.get(), CURLOPT_USERAGENT, "cpp-fetchjson-agent/1.0");
    curl_easy_setopt(curl.get(), CURLOPT_PROTOCOLS, CURLPROTO_HTTPS); // Enforce HTTPS
    
    CURLcode res = curl_easy_perform(curl.get());

    if (res != CURLE_OK) {
        throw std::runtime_error("curl_easy_perform() failed: " + std::string(curl_easy_strerror(res)));
    }
    
    long http_code = 0;
    curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &http_code);
    if (http_code != 200) {
        throw std::runtime_error("HTTP request failed with status code: " + std::to_string(http_code));
    }
    
    try {
        return json::parse(readBuffer);
    } catch (const json::parse_error& e) {
        throw std::runtime_error("Failed to parse JSON: " + std::string(e.what()));
    }
}

int main(int argc, char* argv[]) {
    curl_global_init(CURL_GLOBAL_SSL);

    if (argc > 1) {
        std::cout << "--- Testing with provided URL: " << argv[1] << " ---" << std::endl;
        try {
            json result = fetchAndParseJson(argv[1]);
            std::cout << "Success! Fetched JSON object:" << std::endl;
            std::cout << result.dump(4) << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
        std::cout << "\n--- Running built-in test cases ---\n" << std::endl;
    }

    const char* testUrls[] = {
        "https://jsonplaceholder.typicode.com/posts/1",
        "https://www.google.com",
        "https://jsonplaceholder.typicode.com/posts/99999999",
        "http://jsonplaceholder.typicode.com/posts/1",
        "https://domain.invalid"
    };
    
    for (int i = 0; i < 5; ++i) {
        std::cout << "--- Test Case " << i + 1 << ": " << testUrls[i] << " ---" << std::endl;
        try {
            json result = fetchAndParseJson(testUrls[i]);
            std::cout << "Success! Fetched JSON object:" << std::endl;
            std::cout << result.dump(4) << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
        std::cout << std::endl;
    }
    
    curl_global_cleanup();
    return 0;
}