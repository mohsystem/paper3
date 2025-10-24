#include <iostream>
#include <string>
#include <vector>
// This code requires libcurl.
// To compile: g++ your_file_name.cpp -o output -lcurl
#include <curl/curl.h>

// Callback function for libcurl to write received data into a std::string
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

/**
 * @brief Retrieves the content of a file from a server.
 * @param baseUrl The base URL of the server (e.g., "https://example.com/files").
 * @param fileName The name of the file to retrieve (e.g., "data.txt").
 * @return The content of the file as a std::string, or an error message if it fails.
 */
std::string retrieveFileFromServer(std::string baseUrl, const std::string& fileName) {
    if (fileName.empty() || fileName.find_first_not_of(" \t\n\v\f\r") == std::string::npos) {
        return "Error: File name cannot be empty.";
    }

    CURL *curl;
    CURLcode res;
    std::string readBuffer;
    
    // Ensure the base URL ends with a slash if it doesn't already
    if (!baseUrl.empty() && baseUrl.back() != '/') {
        baseUrl += "/";
    }
    std::string fullUrl = baseUrl + fileName;

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, fullUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirects
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L); // 5 second timeout

        res = curl_easy_perform(curl);
        
        if(res != CURLE_OK) {
            std::string errorMsg = "Error: curl_easy_perform() failed: ";
            errorMsg += curl_easy_strerror(res);
            curl_easy_cleanup(curl);
            return errorMsg;
        }

        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

        if (http_code < 200 || http_code >= 300) {
            readBuffer = "Error: Failed to retrieve file. HTTP status code: " + std::to_string(http_code);
        }

        curl_easy_cleanup(curl);
    } else {
        return "Error: Could not initialize libcurl.";
    }

    return readBuffer;
}

int main() {
    std::string baseUrl = "https://jsonplaceholder.typicode.com";
    std::vector<std::string> testFiles = {
        "todos/1",
        "posts/10",
        "users/5",
        "nonexistent/path", // This will cause a 404
        "" // Empty file name
    };

    for (size_t i = 0; i < testFiles.size(); ++i) {
        std::cout << "--- Test Case " << (i + 1) << ": Retrieving '" << testFiles[i] << "' ---" << std::endl;
        std::string result = retrieveFileFromServer(baseUrl, testFiles[i]);
        std::cout << "Result:\n" << result << std::endl;
        std::cout << "--- End Test Case " << (i + 1) << " ---\n" << std::endl;
    }

    return 0;
}