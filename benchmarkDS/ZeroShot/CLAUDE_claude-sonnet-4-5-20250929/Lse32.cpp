
#include <iostream>
#include <string>
#include <set>
#include <algorithm>
#include <regex>

class Lse32 {
private:
    static std::set<std::string> getAllowedDomains() {
        return {"example.com", "www.example.com"};
    }
    
    static std::string urlDecode(const std::string& str) {
        std::string result;
        for (size_t i = 0; i < str.length(); ++i) {
            if (str[i] == '%' && i + 2 < str.length()) {
                int value;
                std::sscanf(str.substr(i + 1, 2).c_str(), "%x", &value);
                result += static_cast<char>(value);
                i += 2;
            } else if (str[i] == '+') {
                result += ' ';
            } else {
                result += str[i];
            }
        }
        return result;
    }
    
    static std::string extractHost(const std::string& url) {
        std::regex urlRegex("^(?:https?://)?([^:/]+)");
        std::smatch match;
        
        if (std::regex_search(url, match, urlRegex) && match.size() > 1) {
            return match[1].str();
        }
        return "";
    }
    
    static std::string toLower(std::string str) {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        return str;
    }

public:
    static std::string redirectInvalidUrl(const std::string& url) {
        try {
            if (url.empty()) {
                return "Error: URL cannot be empty";
            }
            
            std::string decodedUrl = urlDecode(url);
            std::string host = extractHost(decodedUrl);
            
            if (host.empty()) {
                return "Error: Invalid URL format";
            }
            
            host = toLower(host);
            
            std::set<std::string> allowedDomains = getAllowedDomains();
            
            if (allowedDomains.find(host) != allowedDomains.end()) {
                return "Redirect to: " + decodedUrl;
            } else {
                return "Error: URL is not from an allowed domain";
            }
            
        } catch (...) {
            return "Error: Invalid URL";
        }
    }
};

int main() {
    std::cout << "Test 1: " << Lse32::redirectInvalidUrl("https://example.com/page") << std::endl;
    std::cout << "Test 2: " << Lse32::redirectInvalidUrl("https://malicious.com/page") << std::endl;
    std::cout << "Test 3: " << Lse32::redirectInvalidUrl("https://www.example.com/secure") << std::endl;
    std::cout << "Test 4: " << Lse32::redirectInvalidUrl("") << std::endl;
    std::cout << "Test 5: " << Lse32::redirectInvalidUrl("https://example.com.malicious.com/fake") << std::endl;
    
    return 0;
}
