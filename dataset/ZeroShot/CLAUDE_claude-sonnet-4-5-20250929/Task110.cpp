
#include <iostream>
#include <string>
#include <regex>
#include <algorithm>
#include <cctype>

class Task110 {
private:
    static const int MAX_URL_LENGTH = 2048;
    
    static std::string toLower(const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(),
                      [](unsigned char c){ return std::tolower(c); });
        return result;
    }
    
    static std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \\t\\n\\r");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \\t\\n\\r");
        return str.substr(first, last - first + 1);
    }
    
public:
    static bool validateURL(const std::string& url) {
        if (url.empty()) {
            return false;
        }
        
        std::string trimmedUrl = trim(url);
        
        if (trimmedUrl.empty()) {
            return false;
        }
        
        // Check maximum length
        if (trimmedUrl.length() > MAX_URL_LENGTH) {
            return false;
        }
        
        // Check for whitespace
        if (trimmedUrl.find(' ') != std::string::npos) {
            return false;
        }
        
        // URL pattern validation
        std::regex urlPattern(
            "^(https?://)?"
            "(([a-zA-Z0-9$\\\\-_.+!*'(),;?&=]|(%[0-9a-fA-F]{2}))+@)?"
            "(([a-zA-Z0-9]([a-zA-Z0-9\\\\-]{0,61}[a-zA-Z0-9])?\\\\.)*"
            "[a-zA-Z0-9]([a-zA-Z0-9\\\\-]{0,61}[a-zA-Z0-9])?|"
            "\\\\[[0-9a-fA-F:.]+\\\\])"
            "(:[0-9]{1,5})?"
            "(/[a-zA-Z0-9$\\\\-_.+!*'(),;?&=%]*)*"
            "(\\\\?[a-zA-Z0-9$\\\\-_.+!*'(),;?&=%]*)?"
            "(#[a-zA-Z0-9$\\\\-_.+!*'(),;?&=%]*)?$"
        );
        
        if (!std::regex_match(trimmedUrl, urlPattern)) {
            return false;
        }
        
        // Additional security checks
        std::string lowerUrl = toLower(trimmedUrl);
        
        // Check for dangerous protocols
        if (lowerUrl.find("javascript:") == 0 ||
            lowerUrl.find("data:") == 0 ||
            lowerUrl.find("vbscript:") == 0 ||
            lowerUrl.find("file:") == 0) {
            return false;
        }
        
        return true;
    }
};

int main() {
    // Test cases
    std::string testUrls[] = {
        "https://www.example.com",
        "http://subdomain.example.com:8080/path?query=value#fragment",
        "ftp://ftp.example.com/file.txt",
        "javascript:alert('XSS')",
        "https://example.com/path with spaces"
    };
    
    std::cout << "URL Validation Results:" << std::endl;
    std::cout << "======================" << std::endl;
    
    for (const auto& url : testUrls) {
        bool isValid = Task110::validateURL(url);
        std::cout << "URL: " << url << std::endl;
        std::cout << "Valid: " << (isValid ? "true" : "false") << std::endl;
        std::cout << std::endl;
    }
    
    return 0;
}
