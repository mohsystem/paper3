#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <curl/curl.h>

// Callback function to write received data into a std::string
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    try {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    } catch (const std::bad_alloc& e) {
        return 0; // Signal error
    }
}

// RAII wrapper for curl_global_init and curl_global_cleanup
struct CurlGlobalInitializer {
    CurlGlobalInitializer() { curl_global_init(CURL_GLOBAL_ALL); }
    ~CurlGlobalInitializer() { curl_global_cleanup(); }
    CurlGlobalInitializer(const CurlGlobalInitializer&) = delete;
    CurlGlobalInitializer& operator=(const CurlGlobalInitializer&) = delete;
};

/**
 * @brief Fetches content from a URL and extracts the page title.
 * 
 * @param url The HTTPS URL to fetch.
 * @return The page title, or an empty string if not found or an error occurs.
 */
std::string extractPageTitle(const std::string& url) {
    if (url.rfind("https://", 0) != 0) {
        return "";
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        return "";
    }

    std::string readBuffer;
    std::string title;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.64.1");
    
    // By default, libcurl verifies the peer's certificate and the host name.

    CURLcode res = curl_easy_perform(curl);
    if (res == CURLE_OK) {
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        if (response_code == 200) {
            std::string content_lower = readBuffer;
            std::transform(content_lower.begin(), content_lower.end(), content_lower.begin(),
                           [](unsigned char c){ return std::tolower(c); });
            
            size_t start_tag = content_lower.find("<title");
            if (start_tag != std::string::npos) {
                size_t start_content = content_lower.find('>', start_tag);
                if (start_content != std::string::npos) {
                    size_t end_tag = content_lower.find("</title>", start_content);
                    if (end_tag != std::string::npos) {
                        size_t real_start = start_content + 1;
                        title = readBuffer.substr(real_start, end_tag - real_start);
                    }
                }
            }
        }
    }

    curl_easy_cleanup(curl);
    return title;
}

int main(int argc, char* argv[]) {
    CurlGlobalInitializer curl_initializer;

    if (argc > 1) {
        std::string url = argv[1];
        std::cout << "Extracting title from URL provided via command line: " << url << std::endl;
        std::string title = extractPageTitle(url);
        if (!title.empty()) {
            std::cout << "Page Title: " << title << std::endl;
        } else {
            std::cout << "Could not extract page title." << std::endl;
        }
        std::cout << "---" << std::endl;
    }

    std::cout << "Running test cases..." << std::endl;
    const std::vector<std::string> testUrls = {
        "https://www.google.com",
        "https://example.com",
        "https://httpbin.org/html",
        "invalid-url",
        "https://thishostshouldnotexist12345.com"
    };

    for (size_t i = 0; i < testUrls.size(); ++i) {
        std::cout << "Test Case " << (i + 1) << ": " << testUrls[i] << std::endl;
        std::string title = extractPageTitle(testUrls[i]);
        if (!title.empty()) {
            std::cout << "  -> Title: " << title << std::endl;
        } else {
            std::cout << "  -> Could not extract title (as expected for invalid cases)." << std::endl;
        }
    }

    return 0;
}

// To compile: g++ -std=c++11 Task76.cpp -o Task76 -lcurl