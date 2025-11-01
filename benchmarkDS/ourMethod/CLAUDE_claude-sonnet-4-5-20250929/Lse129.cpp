
#include <iostream>
#include <string>
#include <map>
#include <regex>
#include <algorithm>
#include <sstream>
#include <cctype>

// Simple template environment that stores templates
class Environment {
private:
    std::map<std::string, std::string> templates;

public:
    void addTemplate(const std::string& name, const std::string& content) {
        if (name.empty() || content.empty()) {
            throw std::invalid_argument("Template name and content cannot be empty");
        }
        templates[name] = content;
    }
    
    std::string getTemplate(const std::string& name) const {
        auto it = templates.find(name);
        if (it != templates.end()) {
            return it->second;
        }
        return "";
    }
};

// Template class with safe rendering
class Template {
private:
    std::string content;
    
    // Escape HTML special characters to prevent XSS attacks
    std::string escapeHtml(const std::string& input) const {
        std::string result;
        result.reserve(input.length() * 2); // Reserve space for efficiency
        
        for (char c : input) {
            switch (c) {
                case '&': result += "&amp;"; break;
                case '<': result += "&lt;"; break;
                case '>': result += "&gt;"; break;
                case '"': result += "&quot;"; break;
                case '\\'': result += "&#x27;"; break;
                case '/': result += "&#x2F;"; break;
                default: result += c; break;
            }
        }
        return result;
    }
    
    // Validate name matches safe pattern (alphanumeric, underscore, hyphen, space)
    bool isValidName(const std::string& name) const {
        if (name.empty() || name.length() > 100) {
            return false;
        }
        
        std::regex safePattern("^[a-zA-Z0-9_\\\\-\\\\s]+$");
        return std::regex_match(name, safePattern);
    }

public:
    Template(const std::string& content) : content(content) {}
    
    // Render template with name parameter, escaping HTML to prevent XSS
    std::string render(const std::string& name) const {
        std::string safeName;
        
        // Validate name parameter - reject if doesn't match safe pattern\n        if (name.empty() || !isValidName(name)) {\n            // Fail closed: use a safe default instead of user input\n            safeName = "Guest";\n        } else {\n            safeName = name;\n        }\n        \n        // Escape HTML special characters to prevent XSS\n        std::string escapedName = escapeHtml(safeName);\n        \n        // Simple template variable replacement\n        std::string result = content;\n        size_t pos = result.find("{{name}}");\n        if (pos != std::string::npos) {\n            result.replace(pos, 8, escapedName);\n        }\n        \n        return result;\n    }\n};\n\n// Response class to encapsulate HTTP response\nclass Response {\nprivate:\n    std::string body;\n    int statusCode;\n\npublic:\n    Response(const std::string& body, int statusCode = 200) \n        : body(body), statusCode(statusCode) {}\n    \n    std::string getBody() const { return body; }\n    int getStatusCode() const { return statusCode; }\n};\n\n// URL decode helper function\nstd::string urlDecode(const std::string& str) {\n    std::string result;\n    result.reserve(str.length());\n    \n    for (size_t i = 0; i < str.length(); ++i) {\n        if (str[i] == '%' && i + 2 < str.length()) {\n            // Check if next two chars are valid hex\n            if (std::isxdigit(str[i + 1]) && std::isxdigit(str[i + 2])) {\n                int value;\n                std::stringstream ss;\n                ss << std::hex << str.substr(i + 1, 2);\n                ss >> value;\n                result += static_cast<char>(value);\n                i += 2;\n            } else {\n                result += str[i];\n            }\n        } else if (str[i] == '+') {\n            result += ' ';\n        } else {\n            result += str[i];\n        }\n    }\n    return result;\n}\n\n// Extract query parameter safely from URI\nstd::string getQueryParameter(const std::string& requestUri, const std::string& paramName) {\n    if (requestUri.empty() || paramName.empty()) {\n        return "";\n    }\n    \n    // Find query string start\n    size_t queryStart = requestUri.find('?');\n    if (queryStart == std::string::npos) {\n        return "";\n    }\n    \n    std::string query = requestUri.substr(queryStart + 1);\n    \n    // Parse query parameters\n    std::stringstream ss(query);\n    std::string pair;\n    \n    while (std::getline(ss, pair, '&')) {\n        size_t eqPos = pair.find('=');
        if (eqPos != std::string::npos) {
            std::string key = urlDecode(pair.substr(0, eqPos));
            if (key == paramName) {
                std::string value = urlDecode(pair.substr(eqPos + 1));
                // Limit parameter length to prevent DoS
                if (value.length() > 1000) {
                    return "";
                }
                return value;
            }
        }
    }
    
    return "";
}

// Render response from environment with security controls
Response renderResponseFromEnv(const Environment& env, const std::string& requestUri) {
    if (requestUri.empty()) {
        return Response("Bad Request", 400);
    }
    
    // Get name parameter from request with input validation
    std::string name = getQueryParameter(requestUri, "name");
    
    // Get template from environment - use a fixed template name to prevent template injection
    std::string templateContent = env.getTemplate("greeting");
    if (templateContent.empty()) {
        return Response("Template not found", 500);
    }
    
    // Create template and render with validated name
    Template tmpl(templateContent);
    std::string rendered = tmpl.render(name);
    
    // Return response with rendered content
    return Response(rendered, 200);
}

// Home function that calls renderResponseFromEnv
Response home(const Environment& env, const std::string& requestUri) {
    // Delegate to renderResponseFromEnv
    return renderResponseFromEnv(env, requestUri);
}

// Test cases
int main() {
    Environment env;
    env.addTemplate("greeting", "<html><body>Hello, {{name}}!</body></html>");
    
    std::cout << "Test Case 1: Valid name parameter" << std::endl;
    Response resp1 = home(env, "http://localhost/?name=Alice");
    std::cout << "Status: " << resp1.getStatusCode() << std::endl;
    std::cout << "Body: " << resp1.getBody() << std::endl << std::endl;
    
    std::cout << "Test Case 2: Name with spaces" << std::endl;
    Response resp2 = home(env, "http://localhost/?name=John%20Doe");
    std::cout << "Status: " << resp2.getStatusCode() << std::endl;
    std::cout << "Body: " << resp2.getBody() << std::endl << std::endl;
    
    std::cout << "Test Case 3: XSS attempt (should be escaped)" << std::endl;
    Response resp3 = home(env, "http://localhost/?name=%3Cscript%3Ealert(1)%3C%2Fscript%3E");
    std::cout << "Status: " << resp3.getStatusCode() << std::endl;
    std::cout << "Body: " << resp3.getBody() << std::endl << std::endl;
    
    std::cout << "Test Case 4: No name parameter (should use default)" << std::endl;
    Response resp4 = home(env, "http://localhost/");
    std::cout << "Status: " << resp4.getStatusCode() << std::endl;
    std::cout << "Body: " << resp4.getBody() << std::endl << std::endl;
    
    std::cout << "Test Case 5: Invalid characters (should use default)" << std::endl;
    Response resp5 = home(env, "http://localhost/?name=%00%01%02");
    std::cout << "Status: " << resp5.getStatusCode() << std::endl;
    std::cout << "Body: " << resp5.getBody() << std::endl;
    
    return 0;
}
