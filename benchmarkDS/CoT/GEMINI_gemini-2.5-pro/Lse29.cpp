#include <iostream>
#include <string>
#include <vector>

class Lse29 {
public:
    static std::string getRedirectUrl(const std::string& target) {
        if (target.empty()) {
            return "/";
        }

        std::string host;
        size_t scheme_pos = target.find("://");

        if (scheme_pos == std::string::npos) {
            return "/"; // Not a valid absolute URL
        }

        size_t host_start_pos = scheme_pos + 3;
        size_t host_end_pos = target.find_first_of("/?#", host_start_pos);

        if (host_end_pos != std::string::npos) {
            host = target.substr(host_start_pos, host_end_pos - host_start_pos);
        } else {
            host = target.substr(host_start_pos);
        }
        
        // Remove port if present
        size_t port_pos = host.find(":");
        if (port_pos != std::string::npos) {
            host = host.substr(0, port_pos);
        }

        if (host.empty()) {
            return "/";
        }
        
        // Lowercase for case-insensitive comparison
        for (char &c : host) {
            c = tolower(c);
        }

        // Secure check for host
        if (host == "example.com") {
            return target;
        }
        
        std::string suffix = ".example.com";
        if (host.length() > suffix.length() && 
            host.substr(host.length() - suffix.length()) == suffix) {
            return target;
        }

        return "/";
    }
};

void runTest(const std::string& name, const std::string& input, const std::string& expected) {
    std::string result = Lse29::getRedirectUrl(input);
    std::cout << name << ": " << (result == expected ? "Pass" : "Fail") << std::endl;
    std::cout << "  Input: " << input << std::endl;
    std::cout << "  Output: " << result << ", Expected: " << expected << std::endl;
}

int main() {
    std::cout << "--- C++ Test Cases ---" << std::endl;

    runTest("1. Valid Main Domain", "http://example.com/home", "http://example.com/home");
    runTest("2. Valid Subdomain", "https://sub.example.com/path?q=1", "https://sub.example.com/path?q=1");
    runTest("3. Invalid Domain", "http://evilexample.com", "/");
    runTest("4. Tricky Invalid Domain", "http://evil.com?p=example.com", "/");
    runTest("5. Malformed/Relative URL", "/local/path", "/");
    
    return 0;
}