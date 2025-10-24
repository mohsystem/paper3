// To compile and run this code, you need to have libcurl installed.
// On Debian/Ubuntu: sudo apt-get install libcurl4-openssl-dev
// On Fedora/CentOS: sudo dnf install libcurl-devel
//
// Compile command: g++ your_file_name.cpp -o your_executable_name -lcurl
// Then run: ./your_executable_name

#include <iostream>
#include <string>
#include <vector>
#include <curl/curl.h>

/**
 * @brief Callback function for libcurl to write received data into a std::string.
 * 
 * @param contents Pointer to the data received.
 * @param size Size of each data element.
 * @param nmemb Number of data elements.
 * @param userp Pointer to the user-provided std::string.
 * @return size_t The total size of the received data.
 */
static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

/**
 * @brief Makes an HTTP GET request to the specified URL using libcurl.
 * 
 * @param url The URL to make the request to.
 * @return A string containing the response body or an error message.
 */
std::string makeHttpRequest(const std::string& url) {
    CURL *curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirects
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L); // 5 second timeout

        res = curl_easy_perform(curl);

        if(res != CURLE_OK) {
            readBuffer = "Error: curl_easy_perform() failed: ";
            readBuffer += curl_easy_strerror(res);
        }

        curl_easy_cleanup(curl);
    } else {
        return "Error: Could not initialize cURL.";
    }

    return readBuffer;
}

int main() {
    curl_global_init(CURL_GLOBAL_ALL);

    std::vector<std::string> testUrls = {
        "https://jsonplaceholder.typicode.com/todos/1",
        "https://httpbin.org/get",
        "https://api.ipify.org?format=json",
        "https://www.google.com",
        "http://thissitedoesnotexist.invalidtld/"
    };

    for (const auto& url : testUrls) {
        std::cout << "Testing URL: " << url << std::endl;
        std::string result = makeHttpRequest(url);
        
        if (result.length() > 300) {
            std::cout << "Result (first 300 chars):\n" << result.substr(0, 300) << "...\n" << std::endl;
        } else {
            std::cout << "Result:\n" << result << "\n" << std::endl;
        }
        std::cout << "----------------------------------------" << std::endl;
    }

    curl_global_cleanup();
    return 0;
}