
#include <iostream>
#include <string>
#include <regex>
#include <curl/curl.h>

// Callback function to write data
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t totalSize = size * nmemb;
    if (userp->size() + totalSize > 1000000) {
        return 0; // Stop if content too large
    }
    userp->append((char*)contents, totalSize);
    return totalSize;
}

std::string getPageTitle(const std::string& urlString) {
    if (urlString.empty()) {
        return "Invalid URL";
    }
    
    // Validate HTTPS URL
    std::string lowerUrl = urlString;
    std::transform(lowerUrl.begin(), lowerUrl.end(), lowerUrl.begin(), ::tolower);
    if (lowerUrl.find("https://") != 0) {
        return "Only HTTPS URLs are allowed";
    }
    
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    
    if (!curl) {
        curl_global_cleanup();
        return "Failed to initialize CURL";
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, urlString.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    
    res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        std::string error = "Error: " + std::string(curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return error;
    }
    
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    
    // Extract title using regex
    std::regex titleRegex("<title>(.*?)</title>", std::regex_constants::icase);
    std::smatch match;
    
    if (std::regex_search(readBuffer, match, titleRegex)) {
        std::string title = match[1].str();
        // Trim whitespace
        title.erase(0, title.find_first_not_of(" \\t\\n\\r"));
        title.erase(title.find_last_not_of(" \\t\\n\\r") + 1);
        return title;
    }
    
    return "No title found";
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::string result = getPageTitle(argv[1]);
        std::cout << "Title: " << result << std::endl;
    } else {
        // Test cases
        std::cout << "Test Case 1:" << std::endl;
        std::cout << getPageTitle("https://www.example.com") << std::endl;
        
        std::cout << "\\nTest Case 2:" << std::endl;
        std::cout << getPageTitle("https://www.google.com") << std::endl;
        
        std::cout << "\\nTest Case 3:" << std::endl;
        std::cout << getPageTitle("http://www.example.com") << std::endl;
        
        std::cout << "\\nTest Case 4:" << std::endl;
        std::cout << getPageTitle("") << std::endl;
        
        std::cout << "\\nTest Case 5:" << std::endl;
        std::cout << getPageTitle("invalid-url") << std::endl;
    }
    
    return 0;
}
