
#include <iostream>
#include <string>
#include <map>
#include <regex>
#include <algorithm>

class Lse131 {
public:
    // Simple template environment simulator
    class Environment {
    private:
        std::map<std::string, std::string> templates;
        
    public:
        Environment() {
            templates["greeting"] = "Hello, {{name}}!";
        }
        
        std::string getTemplate(const std::string& templateName) {
            if (templates.find(templateName) != templates.end()) {
                return templates[templateName];
            }
            return "";
        }
    };
    
    // Sanitize input to prevent XSS and injection attacks
    static std::string sanitizeInput(const std::string& input) {
        if (input.empty()) {
            return "";
        }
        std::string sanitized = input;
        
        // Remove potentially dangerous characters
        sanitized = std::regex_replace(sanitized, std::regex("[<>\\"'&;]"), "");
        sanitized = std::regex_replace(sanitized, std::regex("\\\\{\\\\{"), "");
        sanitized = std::regex_replace(sanitized, std::regex("\\\\}\\\\}"), "");
        
        // Trim whitespace
        sanitized.erase(0, sanitized.find_first_not_of(" \\t\\n\\r"));
        sanitized.erase(sanitized.find_last_not_of(" \\t\\n\\r") + 1);
        
        return sanitized;
    }
    
    // Validate name parameter
    static bool isValidName(const std::string& name) {
        if (name.empty() || name.length() > 50) {
            return false;
        }
        // Only allow alphanumeric characters and spaces
        std::regex pattern("^[a-zA-Z0-9 ]+$");
        return std::regex_match(name, pattern);
    }
    
    static std::string renderResponseFromEnv(Environment& env, const std::string& nameParam) {
        // Validate and sanitize input
        if (!isValidName(nameParam)) {
            return "Invalid name parameter";
        }
        
        std::string sanitizedName = sanitizeInput(nameParam);
        
        // Get template safely
        std::string templateStr = env.getTemplate("greeting");
        
        // Render template with sanitized input
        std::string rendered = templateStr;
        size_t pos = rendered.find("{{name}}");
        if (pos != std::string::npos) {
            rendered.replace(pos, 8, sanitizedName);
        }
        
        return rendered;
    }
    
    static std::string home(Environment& env, const std::string& nameParam) {
        return renderResponseFromEnv(env, nameParam);
    }
};

int main() {
    Lse131::Environment env;
    
    // Test cases
    std::cout << "Test 1: " << Lse131::home(env, "John") << std::endl;
    std::cout << "Test 2: " << Lse131::home(env, "Alice Smith") << std::endl;
    std::cout << "Test 3: " << Lse131::home(env, "<script>alert('xss')</script>") << std::endl;
    std::cout << "Test 4: " << Lse131::home(env, "{{malicious}}") << std::endl;
    std::cout << "Test 5: " << Lse131::home(env, "User123") << std::endl;
    
    return 0;
}
