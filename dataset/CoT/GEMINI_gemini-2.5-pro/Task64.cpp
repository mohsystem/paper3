/*
 * This program requires the following third-party libraries:
 * 1. libcurl: For making HTTP requests.
 * 2. nlohmann/json: For parsing JSON. (A header-only library)
 *
 * Installation (Ubuntu/Debian):
 * sudo apt-get install libcurl4-openssl-dev
 *
 * Download nlohmann/json.hpp from its GitHub repository and place it in your include path.
 *
 * Compilation command:
 * g++ -std=c++17 your_source_file.cpp -o your_executable -lcurl
 */
#include <iostream>
#include <string>
#include <vector>
#include <curl/curl.h>
#include "json.hpp" // nlohmann/json library

// For convenience
using json = nlohmann::json;

// Callback function for libcurl to write received data into a std::string
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

/**
 * @brief Fetches content from a URL and parses it as JSON.
 *
 * @param url_string The URL to fetch the JSON from.
 * @return A json object. It will be `null` if an error occurs.
 */
json fetchJsonFromUrl(const std::string& url_string) {
    if (url_string.empty()) {
        std::cerr << "Error: URL string is empty." << std::endl;
        return nullptr;
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Error: Failed to initialize libcurl." << std::endl;
        return nullptr;
    }

    std::string readBuffer;
    json result = nullptr;

    // Set libcurl options
    curl_easy_setopt(curl, CURLOPT_URL, url_string.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L); // 5-second timeout
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirects

    CURLcode res = curl_easy_perform(curl);

    if (res == CURLE_OK) {
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        if (response_code == 200) {
            try {
                result = json::parse(readBuffer);
            } catch (json::parse_error& e) {
                std::cerr << "Error: Failed to parse JSON. " << e.what() << std::endl;
            }
        } else {
            std::cerr << "Error: HTTP GET request failed with response code: " << response_code << std::endl;
        }
    } else {
        std::cerr << "Error: libcurl failed: " << curl_easy_strerror(res) << std::endl;
    }
    
    curl_easy_cleanup(curl);
    return result;
}

int main(int argc, char* argv[]) {
    // It's good practice to initialize libcurl globally.
    curl_global_init(CURL_GLOBAL_ALL);

    // Command-line argument handling
    if (argc > 1) {
        std::cout << "Fetching JSON from command-line argument: " << argv[1] << std::endl;
        json result = fetchJsonFromUrl(argv[1]);
        if (!result.is_null()) {
            std::cout << "Successfully fetched and parsed JSON:\n" << result.dump(4) << std::endl;
        } else {
            std::cout << "Failed to fetch or parse JSON." << std::endl;
        }
        std::cout << "\n--- Running built-in test cases ---\n" << std::endl;
    }

    const std::vector<std::string> test_urls = {
        // 1. Valid JSON URL
        "https://jsonplaceholder.typicode.com/posts/1",
        // 2. URL pointing to non-JSON content (HTML)
        "http://example.com",
        // 3. URL that results in a 404 Not Found error
        "https://jsonplaceholder.typicode.com/posts/99999",
        // 4. Malformed URL (libcurl often handles this gracefully)
        "htp:/invalid-url",
        // 5. Non-existent domain
        "http://domain.that.does.not.exist"
    };

    for (size_t i = 0; i < test_urls.size(); ++i) {
        std::cout << "--- Test Case " << (i + 1) << ": " << test_urls[i] << " ---" << std::endl;
        json result = fetchJsonFromUrl(test_urls[i]);
        if (!result.is_null()) {
             std::cout << "Success! Fetched JSON object. Type: " << result.type_name() << std::endl;
             // std::cout << result.dump(4) << std::endl; // Uncomment for full output
        } else {
            std::cout << "Failure! As expected or due to an error." << std::endl;
        }
        std::cout << std::endl;
    }

    curl_global_cleanup();
    return 0;
}