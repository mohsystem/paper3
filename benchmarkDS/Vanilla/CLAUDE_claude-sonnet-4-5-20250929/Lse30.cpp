
#include <iostream>
#include <string>
#include <regex>

class Lse30 {
public:
    static std::string processRedirect(const std::string& target) {
        // Regular expression: one or more alphanumeric characters or hyphens followed by a period
        std::regex pattern("^[a-zA-Z0-9-]+\\\\..*");
        
        if (std::regex_match(target, pattern)) {
            return target;
        } else {
            return "/";
        }
    }
};

int main() {
    // Test cases
    std::string testCases[] = {
        "example.com",
        "test-site.org",
        "my-page.html",
        ".invalid",
        "noperiod"
    };
    
    std::cout << "C++ Test Results:" << std::endl;
    for (const std::string& testCase : testCases) {
        std::string result = Lse30::processRedirect(testCase);
        std::cout << "Input: '" << testCase << "' -> Redirect to: '" << result << "'" << std::endl;
    }
    
    return 0;
}
