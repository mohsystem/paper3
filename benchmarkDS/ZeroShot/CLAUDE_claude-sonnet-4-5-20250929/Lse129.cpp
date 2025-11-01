
#include <iostream>
#include <string>
#include <map>
#include <sstream>
#include <algorithm>

class Lse129 {
public:
    // Simple template environment simulator
    class Environment {
    private:
        std::map<std::string, std::string> templates;
        
    public:
        Environment() {
            // Simple template storage
            templates["home.html"] = "<html><body>Hello, {{name}}!</body></html>";
        }
        
        std::string getTemplate(const std::string& templateName) {
            if (templates.find(templateName) != templates.end()) {
                return templates[templateName];
            }
            return "";
        }
        
        void addTemplate(const std::string& name, const std::string& content) {
            templates[name] = content;
        }
    };
    
    // Sanitize input to prevent XSS
    static std::string sanitizeInput(const std::string& input) {
        std::string result = input;
        
        // Replace dangerous characters
        size_t pos = 0;
        while ((pos = result.find("<", pos)) != std::string::npos) {
            result.replace(pos, 1, "&lt;");
            pos += 4;
        }
        
        pos = 0;
        while ((pos = result.find(">", pos)) != std::string::npos) {
            result.replace(pos, 1, "&gt;");
            pos += 4;
        }
        
        pos = 0;
        while ((pos = result.find("&", pos)) != std::string::npos) {
            if (result.substr(pos, 4) != "&lt;" && result.substr(pos, 4) != "&gt;") {
                result.replace(pos, 1, "&amp;");
                pos += 5;
            } else {
                pos++;
            }
        }
        
        return result;
    }
    
    // Render response from environment
    static std::string renderResponseFromEnv(Environment& env, const std::string& name) {
        try {
            // Get the template from the environment
            std::string templateStr = env.getTemplate("home.html");
            
            if (templateStr.empty()) {
                return "Error: Template not found";
            }
            
            // Sanitize the name parameter
            std::string sanitizedName = sanitizeInput(name);
            
            // Simple template rendering (replace {{name}} with actual name)
            std::string rendered = templateStr;
            size_t pos = rendered.find("{{name}}");
            if (pos != std::string::npos) {
                rendered.replace(pos, 8, sanitizedName);
            }
            
            return rendered;
        } catch (const std::exception& e) {
            return "Error rendering template: " + std::string(e.what());
        }
    }
    
    // Home function
    static std::string home(Environment& env, const std::string& name) {
        return renderResponseFromEnv(env, name);
    }
};

int main() {
    // Setup environment
    Lse129::Environment env;
    
    // Test cases
    std::string testCases[] = {
        "John",
        "Alice",
        "<script>alert('XSS')</script>",
        "Bob O'Brien",
        ""
    };
    
    std::cout << "Testing renderResponseFromEnv and home functions:" << std::endl;
    std::cout << "=================================================" << std::endl;
    std::cout << std::endl;
    
    for (int i = 0; i < 5; i++) {
        std::cout << "Test Case " << (i + 1) << ":" << std::endl;
        std::cout << "Input: " << testCases[i] << std::endl;
        std::string response = Lse129::home(env, testCases[i]);
        std::cout << "Response: " << response << std::endl;
        std::cout << std::endl;
    }
    
    return 0;
}
