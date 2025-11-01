
#include <iostream>
#include <string>
#include <map>
#include <regex>
#include <algorithm>
#include <cctype>

// Environment holds templates
class Environment {
private:
    std::map<std::string, std::string> templates;
    
public:
    Environment() {
        // Pre-register safe templates only
        templates["greeting"] = "Hello, {{name}}!";
        templates["welcome"] = "Welcome, {{name}}!";
    }
    
    // Get template by name - returns empty string if not found (safe default)
    std::string getTemplate(const std::string& templateName) const {
        // Validate template name: alphanumeric only, max 50 chars
        if (templateName.empty() || templateName.length() > 50) {
            return "";
        }
        
        std::regex validPattern("^[a-zA-Z0-9_]+$");
        if (!std::regex_match(templateName, validPattern)) {
            return "";
        }
        
        auto it = templates.find(templateName);
        if (it != templates.end()) {
            return it->second;
        }
        return "";
    }
};

// Safely escape HTML special characters to prevent XSS
std::string escapeHtml(const std::string& input) {
    std::string output;
    output.reserve(input.length() * 2); // Reserve space for efficiency
    
    for (char c : input) {
        switch (c) {
            case '&': output += "&amp;"; break;
            case '<': output += "&lt;"; break;
            case '>': output += "&gt;"; break;
            case '"': output += "&quot;"; break;
            case '\\'': output += "&#x27;"; break;
            case '/': output += "&#x2F;"; break;
            default: output += c; break;
        }
    }
    
    return output;
}

// Validate name parameter: alphanumeric, spaces, hyphens only, max 100 chars
std::string validateName(std::string name) {
    if (name.empty()) {
        return "Guest"; // Safe default
    }
    
    // Trim whitespace
    name.erase(0, name.find_first_not_of(" \\t\\n\\r"));
    name.erase(name.find_last_not_of(" \\t\\n\\r") + 1);
    
    // Limit length
    if (name.length() > 100) {
        name = name.substr(0, 100);
    }
    
    // Only allow safe characters
    std::regex safePattern("^[a-zA-Z0-9 \\\\-]+$");
    if (!std::regex_match(name, safePattern)) {
        return "Guest"; // Reject invalid input
    }
    
    return name;
}

// Render template with name parameter - escapes output to prevent XSS
std::string renderTemplate(const std::string& templateStr, const std::string& name) {
    if (templateStr.empty()) {
        return "Invalid template";
    }
    
    // Validate and sanitize name before rendering
    std::string safeName = validateName(name);
    
    // Escape the name for HTML context before substitution
    std::string escapedName = escapeHtml(safeName);
    
    // Simple template rendering with safe substitution
    std::string result = templateStr;
    size_t pos = result.find("{{name}}");
    if (pos != std::string::npos) {
        result.replace(pos, 8, escapedName);
    }
    
    return result;
}

// Extract query parameter from request string
std::string getParameter(const std::string& queryString, const std::string& paramName) {
    if (queryString.empty()) {
        return "";
    }
    
    // Simple query string parsing (URL decode not shown for brevity)
    size_t pos = queryString.find(paramName + "=");
    if (pos == std::string::npos) {
        return "";
    }
    
    pos += paramName.length() + 1;
    size_t endPos = queryString.find('&', pos);
    if (endPos == std::string::npos) {
        endPos = queryString.length();
    }
    
    return queryString.substr(pos, endPos - pos);
}

// Render response from environment - secure implementation
std::string renderResponseFromEnv(const Environment& env, const std::string& requestData) {
    // Get name parameter from request (untrusted input)
    std::string name = getParameter(requestData, "name");
    
    // Get template from environment (always use "greeting" template)
    std::string templateStr = env.getTemplate("greeting");
    if (templateStr.empty()) {
        return "Error: Template not found"; // Generic error message
    }
    
    // Render template with validated and escaped name parameter
    return renderTemplate(templateStr, name);
}

// Home handler that calls renderResponseFromEnv
std::string home(const Environment& env, const std::string& requestData) {
    return renderResponseFromEnv(env, requestData);
}

int main() {
    Environment env;
    
    // Test cases
    std::cout << "Test Case 1 (valid name):" << std::endl;
    std::cout << home(env, "name=Alice") << std::endl;
    
    std::cout << "\\nTest Case 2 (XSS attempt):" << std::endl;
    std::cout << home(env, "name=<script>alert('XSS')</script>") << std::endl;
    
    std::cout << "\\nTest Case 3 (no name parameter):" << std::endl;
    std::cout << home(env, "") << std::endl;
    
    std::cout << "\\nTest Case 4 (valid name with spaces):" << std::endl;
    std::cout << home(env, "name=John Doe") << std::endl;
    
    std::cout << "\\nTest Case 5 (special characters):" << std::endl;
    std::cout << home(env, "name=Test&User") << std::endl;
    
    return 0;
}
