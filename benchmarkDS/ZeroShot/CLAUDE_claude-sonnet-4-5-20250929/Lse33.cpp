
#include <iostream>
#include <string>
#include <map>
#include <algorithm>
#include <regex>

class Lse33 {
public:
    static std::map<std::string, std::string> redirectInvalidUrl(const std::string& urlString) {
        std::map<std::string, std::string> response;
        
        if (urlString.empty()) {
            response["status"] = "error";
            response["message"] = "URL parameter is required";
            return response;
        }
        
        try {
            std::string host = extractHost(urlString);
            
            if (host.empty()) {
                response["status"] = "error";
                response["message"] = "Invalid URL format";
                return response;
            }
            
            // Convert to lowercase for comparison
            std::transform(host.begin(), host.end(), host.begin(), ::tolower);
            
            // Remove port if present
            size_t colonPos = host.find(':');
            if (colonPos != std::string::npos) {
                host = host.substr(0, colonPos);
            }
            
            // Check if domain is exactly example.com or subdomain of example.com
            if (host == "example.com" || 
                (host.length() > 11 && host.substr(host.length() - 11) == ".example.com")) {
                response["status"] = "redirect";
                response["url"] = urlString;
                response["message"] = "Redirecting to: " + urlString;
            } else {
                response["status"] = "error";
                response["message"] = "Invalid domain. Only example.com domain is allowed";
            }
            
        } catch (const std::exception& e) {
            response["status"] = "error";
            response["message"] = std::string("Invalid URL format: ") + e.what();
        }
        
        return response;
    }
    
private:
    static std::string extractHost(const std::string& url) {
        std::regex urlRegex(R"(^(?:https?://)?([^/]+))", std::regex::icase);
        std::smatch match;
        
        if (std::regex_search(url, match, urlRegex) && match.size() > 1) {
            return match[1].str();
        }
        
        return "";
    }
    
public:
    static void printResponse(const std::string& testName, 
                            const std::map<std::string, std::string>& response) {
        std::cout << testName << ": {";
        for (auto it = response.begin(); it != response.end(); ++it) {
            if (it != response.begin()) std::cout << ", ";
            std::cout << it->first << ": " << it->second;
        }
        std::cout << "}" << std::endl;
    }
};

int main() {
    // Test case 1: Valid example.com URL
    Lse33::printResponse("Test 1", 
        Lse33::redirectInvalidUrl("https://example.com/page"));
    
    // Test case 2: Valid subdomain of example.com
    Lse33::printResponse("Test 2", 
        Lse33::redirectInvalidUrl("https://sub.example.com/path"));
    
    // Test case 3: Invalid domain
    Lse33::printResponse("Test 3", 
        Lse33::redirectInvalidUrl("https://malicious.com/page"));
    
    // Test case 4: Invalid URL format
    Lse33::printResponse("Test 4", 
        Lse33::redirectInvalidUrl("not-a-valid-url"));
    
    // Test case 5: Empty URL
    Lse33::printResponse("Test 5", 
        Lse33::redirectInvalidUrl(""));
    
    return 0;
}
