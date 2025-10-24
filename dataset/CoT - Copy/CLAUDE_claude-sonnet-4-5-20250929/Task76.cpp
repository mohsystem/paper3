
#include <iostream>
#include <string>
#include <regex>
#include <curl/curl.h>
#include <algorithm>
#include <cctype>

class Task76 {
private:
    static const int TIMEOUT = 10;
    static const size_t MAX_CONTENT_LENGTH = 5 * 1024 * 1024;
    
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
        size_t totalSize = size * nmemb;
        if (userp->size() + totalSize > MAX_CONTENT_LENGTH) {
            return 0;
        }
        userp->append((char*)contents, totalSize);
        return totalSize;
    }
    
    static bool isValidHttpsUrl(const std::string& url) {
        if (url.empty() || url.substr(0, 8) != "https://") {
            return false;
        }
        
        // Extract hostname
        size_t hostStart = 8;
        size_t hostEnd = url.find('/', hostStart);
        if (hostEnd == std::string::npos) {
            hostEnd = url.length();
        }
        
        std::string host = url.substr(hostStart, hostEnd - hostStart);
        
        // Remove port if present
        size_t portPos = host.find(':');
        if (portPos != std::string::npos) {
            host = host.substr(0, portPos);
        }
        
        if (host.empty()) {
            return false;
        }
        
        // Block localhost and private IPs
        if (host == "localhost" || host == "127.0.0.1" || host == "::1" ||
            host.substr(0, 8) == "192.168." || host.substr(0, 3) == "10." ||
            host.substr(0, 4) == "172.") {
            return false;
        }
        
        return true;
    }
    
    static std::string extractTitleFromHtml(const std::string& html) {
        if (html.empty()) {
            return "No title found";
        }
        
        std::regex titlePattern("<title[^>]*>\\\\s*([^<]+)\\\\s*</title>", std::regex::icase);
        std::smatch match;
        
        if (std::regex_search(html, match, titlePattern)) {
            std::string title = match[1].str();
            return sanitizeOutput(title);
        }
        
        return "No title found";
    }
    
    static std::string sanitizeOutput(const std::string& text) {
        if (text.empty()) {
            return "";
        }
        
        std::string result = text;
        
        // Remove HTML entities
        result = std::regex_replace(result, std::regex("&[a-zA-Z0-9#]+;"), "");
        
        // Replace newlines with spaces
        result = std::regex_replace(result, std::regex("[\\\\r\\\\n]+"), " ");
        
        // Trim whitespace
        size_t start = result.find_first_not_of(" \\t\\n\\r");
        size_t end = result.find_last_not_of(" \\t\\n\\r");
        
        if (start == std::string::npos) {
            return "";
        }
        
        return result.substr(start, end - start + 1);
    }
    
    static std::string sanitizeErrorMessage(const std::string& message) {
        if (message.empty()) {
            return "Unknown error";
        }
        
        std::string sanitized = std::regex_replace(message, std::regex("https?://[^\\\\s]+"), "[URL]");
        
        if (sanitized.length() > 100) {
            sanitized = sanitized.substr(0, 100);
        }
        
        return sanitized;
    }
    
public:
    static std::string extractPageTitle(const std::string& urlString) {
        if (urlString.empty()) {
            return "Error: URL cannot be empty";
        }
        
        if (!isValidHttpsUrl(urlString)) {
            return "Error: Invalid URL or non-HTTPS URL provided";
        }
        
        CURL* curl = curl_easy_init();
        if (!curl) {
            return "Error: Failed to initialize CURL";
        }
        
        std::string responseData;
        CURLcode res;
        
        curl_easy_setopt(curl, CURLOPT_URL, urlString.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, TIMEOUT);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "SecurePageTitleExtractor/1.0");
        
        res = curl_easy_perform(curl);
        
        if (res != CURLE_OK) {
            std::string error = "Error: " + sanitizeErrorMessage(curl_easy_strerror(res));
            curl_easy_cleanup(curl);
            return error;
        }
        
        long httpCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        curl_easy_cleanup(curl);
        
        if (httpCode != 200) {
            return "Error: HTTP " + std::to_string(httpCode);
        }
        
        return extractTitleFromHtml(responseData);
    }
};

int main(int argc, char* argv[]) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <https-url>" << std::endl;
        std::cout << "\\nTest cases:" << std::endl;
        
        std::string testUrls[] = {
            "https://www.example.com",
            "https://www.google.com",
            "http://www.example.com",
            "https://localhost",
            "invalid-url"
        };
        
        for (const auto& url : testUrls) {
            std::cout << "\\nURL: " << url << std::endl;
            std::cout << "Title: " << Task76::extractPageTitle(url) << std::endl;
        }
    } else {
        std::string result = Task76::extractPageTitle(argv[1]);
        std::cout << result << std::endl;
    }
    
    curl_global_cleanup();
    return 0;
}
