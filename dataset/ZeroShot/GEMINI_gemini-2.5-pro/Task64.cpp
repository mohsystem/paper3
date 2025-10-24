/*
 * IMPORTANT: This code requires third-party libraries for HTTP requests and JSON parsing.
 * - libcurl: For handling HTTP/HTTPS requests.
 * - nlohmann/json: A header-only library for JSON parsing.
 *
 * To compile this code, you need to have libcurl installed and the nlohmann/json header
 * available.
 *
 * Example compilation command on Linux:
 * g++ your_file_name.cpp -o your_executable -lcurl
 *
 * You must download json.hpp from https://github.com/nlohmann/json and place it
 * in your project or a standard include path.
 */

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <curl/curl.h>
#include "json.hpp" // Assumes nlohmann/json header is available

// Use the nlohmann json library
using json = nlohmann::json;

const long CONNECT_TIMEOUT = 5L; // 5 seconds
const long REQUEST_TIMEOUT = 10L; // 10 seconds
const size_t MAX_RESPONSE_SIZE = 1024 * 1024; // 1 MB

// Callback function for libcurl to write received data into a std::string
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    std::string* mem = static_cast<std::string*>(userp);

    // 3. Security: Check if the new data will exceed the max size
    if (mem->size() + realsize > MAX_RESPONSE_SIZE) {
        std::cerr << "Error: Response size exceeds the limit of " << MAX_RESPONSE_SIZE << " bytes." << std::endl;
        return 0; // Returning 0 signals an error to libcurl, stopping the transfer
    }

    try {
        mem->append(static_cast<char*>(contents), realsize);
    } catch (const std::bad_alloc& e) {
        std::cerr << "Error: Not enough memory (bad_alloc)." << std::endl;
        return 0;
    }
    return realsize;
}

json fetchAndParseJson(const std::string& urlString) {
    // 1. Security: Validate URL protocol to prevent SSRF
    if (urlString.rfind("http://", 0) != 0 && urlString.rfind("https://", 0) != 0) {
        throw std::runtime_error("Invalid protocol. Only HTTP and HTTPS are allowed.");
    }

    CURL *curl;
    CURLcode res;
    std::string readBuffer;
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (!curl) {
        curl_global_cleanup();
        throw std::runtime_error("Failed to initialize libcurl.");
    }
    
    try {
        curl_easy_setopt(curl, CURLOPT_URL, urlString.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        // 2. Security: Set timeouts to prevent DoS attacks
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, CONNECT_TIMEOUT);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, REQUEST_TIMEOUT);

        res = curl_easy_perform(curl);
        if (res == CURLE_WRITE_ERROR) {
            // Error message already printed by the callback
            throw std::runtime_error("Transfer aborted due to response size limit or memory allocation failure.");
        } else if (res != CURLE_OK) {
            throw std::runtime_error("curl_easy_perform() failed: " + std::string(curl_easy_strerror(res)));
        }

        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        if (http_code != 200) {
            throw std::runtime_error("HTTP request failed with response code: " + std::to_string(http_code));
        }

        curl_easy_cleanup(curl);
        curl_global_cleanup();

    } catch (...) {
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        throw; // Re-throw the current exception
    }
    
    // 4. Parse JSON
    try {
        return json::parse(readBuffer);
    } catch (json::parse_error& e) {
        throw std::runtime_error("Failed to parse JSON: " + std::string(e.what()));
    }
}

int main() {
    std::vector<std::string> testUrls = {
        "https://jsonplaceholder.typicode.com/posts/1",      // Test 1: Success, single object
        "https://jsonplaceholder.typicode.com/posts",         // Test 2: Success, array of objects
        "https://jsonplaceholder.typicode.com/posts/999999",  // Test 3: Client Error (404)
        "https://nonexistent-domain-for-testing123.com",     // Test 4: Network Error
        "https://www.google.com",                             // Test 5: JSON Parse Error
        "file:///etc/hosts"                                   // Test 6: Security Error (Invalid Protocol)
    };

    for (size_t i = 0; i < testUrls.size(); ++i) {
        std::cout << "--- Test Case " << (i + 1) << ": " << testUrls[i] << " ---" << std::endl;
        try {
            json jsonObj = fetchAndParseJson(testUrls[i]);
            std::cout << "Success! Parsed JSON object (first 200 chars):" << std::endl;
            std::string prettyJson = jsonObj.dump(2); // 2 spaces for indentation
            std::cout << prettyJson.substr(0, 200) << (prettyJson.length() > 200 ? "..." : "") << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed: " << e.what() << std::endl;
        }
        std::cout << std::endl;
    }

    return 0;
}