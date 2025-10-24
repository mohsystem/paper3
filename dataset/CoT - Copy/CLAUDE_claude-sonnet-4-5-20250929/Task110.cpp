
#include <iostream>
#include <string>
#include <regex>
#include <algorithm>

class Task110 {
private:
    static const int MAX_URL_LENGTH = 2048;
    
    static bool isValidScheme(const std::string& scheme) {
        std::string lower_scheme = scheme;
        std::transform(lower_scheme.begin(), lower_scheme.end(), lower_scheme.begin(), ::tolower);
        return lower_scheme == "http" || lower_scheme == "https" || lower_scheme == "ftp";
    }
    
    static bool isValidHost(const std::string& host) {
        std::regex host_pattern("^[a-zA-Z0-9]([a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?(\\\\.[a-zA-Z0-9]([a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?)*$");
        std::regex ip_pattern("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
        return std::regex_match(host, host_pattern) || std::regex_match(host, ip_pattern);
    }
    
    static std::string trim(const std::string& str) {
        size_t start = str.find_first_not_of(" \\t\\n\\r");
        size_t end = str.find_last_not_of(" \\t\\n\\r");
        if (start == std::string::npos) return "";
        return str.substr(start, end - start + 1);
    }
    
public:
    static bool validateURL(const std::string& url) {
        if (url.empty()) {
            return false;
        }
        
        std::string trimmed_url = trim(url);
        
        if (trimmed_url.length() > MAX_URL_LENGTH) {
            return false;
        }
        
        if (trimmed_url.find(' ') != std::string::npos || 
            trimmed_url.find('\\n') != std::string::npos || 
            trimmed_url.find('\\r') != std::string::npos || 
            trimmed_url.find('\\t') != std::string::npos) {
            return false;
        }
        
        std::regex url_pattern("^([a-zA-Z]+)://([^/:?#]+)(:[0-9]+)?([^?#]*)(\\\\?[^#]*)?(#.*)?$");
        std::smatch matches;
        
        if (!std::regex_match(trimmed_url, matches, url_pattern)) {
            return false;
        }
        
        std::string scheme = matches[1].str();
        if (!isValidScheme(scheme)) {
            return false;
        }
        
        std::string host = matches[2].str();
        std::transform(host.begin(), host.end(), host.begin(), ::tolower);
        
        if (!isValidHost(host)) {
            return false;
        }
        
        if (matches[3].length() > 0) {
            std::string port_str = matches[3].str().substr(1);
            try {
                int port = std::stoi(port_str);
                if (port < 1 || port > 65535) {
                    return false;
                }
            } catch (...) {
                return false;
            }
        }
        
        std::string path = matches[4].str();
        if (path.find("..") != std::string::npos || 
            path.find("//") != std::string::npos || 
            path.find("\\\\") != std::string::npos) {\n            return false;\n        }\n        \n        if (host.find('@') != std::string::npos) {\n            return false;\n        }\n        \n        return true;\n    }\n};\n\nint main() {\n    std::string testCases[] = {\n        "https://www.example.com",\n        "http://192.168.1.1:8080/path/to/resource",\n        "ftp://files.example.org/downloads",\n        "javascript:alert('XSS')",\n        "https://example.com/path/../../../etc/passwd"\n    };\n    \n    std::cout << "URL Validation Results:" << std::endl;\n    for (int i = 0; i < 5; i++) {\n        bool isValid = Task110::validateURL(testCases[i]);\n        std::cout << "Test " << (i + 1) << ": " << testCases[i] << std::endl;\n        std::cout << "Valid: " << (isValid ? "true" : "false") << std::endl;
        std::cout << std::endl;
    }
    
    return 0;
}
