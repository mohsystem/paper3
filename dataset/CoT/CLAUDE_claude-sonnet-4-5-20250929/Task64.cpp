
#include <iostream>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <algorithm>
#include <cctype>

using json = nlohmann::json;

const size_t MAX_SIZE = 10 * 1024 * 1024; // 10 MB
const long TIMEOUT = 10L; // seconds

class SecurityError : public std::runtime_error {
public:
    explicit SecurityError(const std::string& message) 
        : std::runtime_error(message) {}
};

// Callback function for CURL to write data
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    std::string* str = static_cast<std::string*>(userp);
    
    if (str->size() + realsize > MAX_SIZE) {
        return 0; // Stop receiving data
    }
    
    str->append(static_cast<char*>(contents), realsize);
    return realsize;
}

std::string toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return result;
}

bool isPrivateAddress(const std::string& host) {
    std::string hostLower = toLower(host);
    return (hostLower == "localhost" || 
            hostLower == "127.0.0.1" || 
            hostLower == "0.0.0.0" ||
            hostLower.find("192.168.") == 0 ||
            hostLower.find("10.") == 0 ||
            hostLower.find("172.16.") == 0);
}

json fetchAndParseJson(const std::string& urlString) {
    // Input validation
    if (urlString.empty()) {
        throw std::invalid_argument("URL cannot be empty");
    }
    
    // Protocol validation
    std::string urlLower = toLower(urlString);
    if (urlLower.find("http://") != 0 && urlLower.find("https://") != 0) {
        throw SecurityError("Only HTTP and HTTPS protocols are allowed");
    }
    
    // Extract hostname for SSRF check (basic implementation)
    size_t protocolEnd = urlString.find("://");
    if (protocolEnd != std::string::npos) {
        size_t hostStart = protocolEnd + 3;
        size_t hostEnd = urlString.find("/", hostStart);
        size_t portPos = urlString.find(":", hostStart);
        
        if (portPos != std::string::npos && (hostEnd == std::string::npos || portPos < hostEnd)) {
            hostEnd = portPos;
        }
        
        std::string host = urlString.substr(hostStart, 
            hostEnd == std::string::npos ? std::string::npos : hostEnd - hostStart);
        
        if (isPrivateAddress(host)) {
            throw SecurityError("Access to private network addresses is not allowed");
        }
    }
    
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    
    if (!curl) {
        curl_global_cleanup();
        throw std::runtime_error("Failed to initialize CURL");
    }
    
    try {
        curl_easy_setopt(curl, CURLOPT_URL, urlString.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, TIMEOUT);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L); // Disable redirects
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "SecureJSONFetcher/1.0");
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
        
        res = curl_easy_perform(curl);
        
        if (res != CURLE_OK) {
            std::string error = "CURL error: " + std::string(curl_easy_strerror(res));
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            throw std::runtime_error(error);
        }
        
        long response_code;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
        
        if (response_code != 200) {
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            throw std::runtime_error("HTTP error code: " + std::to_string(response_code));
        }
        
        if (readBuffer.size() > MAX_SIZE) {
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            throw SecurityError("Response size exceeds maximum allowed size");
        }
        
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        
        // Parse JSON
        json jsonObject = json::parse(readBuffer);
        return jsonObject;
        
    } catch (...) {
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        throw;
    }
}

int main(int argc, char* argv[]) {
    // Test cases with safe public URLs
    std::string testUrls[] = {
        "https://jsonplaceholder.typicode.com/posts/1",
        "https://api.github.com/users/github",
        "https://jsonplaceholder.typicode.com/users/1",
        "https://jsonplaceholder.typicode.com/todos/1",
        "https://jsonplaceholder.typicode.com/comments/1"
    };
    
    for (int i = 0; i < 5; i++) {
        std::cout << "\\nTest Case " << (i + 1) << ":" << std::endl;
        std::cout << "URL: " << testUrls[i] << std::endl;
        try {
            json result = fetchAndParseJson(testUrls[i]);
            std::cout << "Result: " << result.dump(2) << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }
    
    return 0;
}
