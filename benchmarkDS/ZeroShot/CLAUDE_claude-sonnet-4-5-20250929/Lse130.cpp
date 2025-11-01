
#include <iostream>
#include <string>
#include <map>
#include <algorithm>

class Response {
private:
    std::string content;
    int statusCode;
    
public:
    Response(const std::string& content, int statusCode) 
        : content(content), statusCode(statusCode) {}
    
    std::string getContent() const {
        return content;
    }
    
    int getStatusCode() const {
        return statusCode;
    }
};

class Environment {
private:
    std::map<std::string, std::string> templates;
    
public:
    Environment() {
        templates["default"] = "<html><body><h1>Hello, {{name}}!</h1></body></html>";
    }
    
    std::string getTemplate(const std::string& templateName) {
        if (templates.find(templateName) != templates.end()) {
            return templates[templateName];
        }
        return templates["default"];
    }
};

std::string sanitizeInput(const std::string& input) {
    std::string result = input;
    
    // Replace special characters to prevent XSS
    size_t pos = 0;
    while ((pos = result.find("&", pos)) != std::string::npos) {
        result.replace(pos, 1, "&amp;");
        pos += 5;
    }
    
    pos = 0;
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
    while ((pos = result.find("\\"", pos)) != std::string::npos) {
        result.replace(pos, 1, "&quot;");
        pos += 6;
    }
    
    pos = 0;
    while ((pos = result.find("'", pos)) != std::string::npos) {
        result.replace(pos, 1, "&#x27;");
        pos += 6;
    }
    
    return result;
}

Response renderResponseFromEnv(Environment& env, const std::map<std::string, std::string>& params) {
    // Get name parameter from request (sanitize to prevent XSS)
    std::string name = "Guest";
    auto it = params.find("name");
    if (it != params.end()) {
        name = it->second;
    }
    name = sanitizeInput(name);
    
    // Get template from environment
    std::string templateStr = env.getTemplate("default");
    
    // Render template with name parameter
    std::string renderedContent = templateStr;
    size_t pos = renderedContent.find("{{name}}");
    if (pos != std::string::npos) {
        renderedContent.replace(pos, 8, name);
    }
    
    // Return response with rendered template
    return Response(renderedContent, 200);
}

Response home(Environment& env, const std::map<std::string, std::string>& params) {
    return renderResponseFromEnv(env, params);
}

class Lse130 {
public:
    static void main() {
        Environment env;
        
        // Test case 1: Normal name
        std::map<std::string, std::string> params1;
        params1["name"] = "John";
        Response response1 = home(env, params1);
        std::cout << "Test 1: " << response1.getContent() << std::endl;
        
        // Test case 2: Empty name (should default to Guest)
        std::map<std::string, std::string> params2;
        Response response2 = home(env, params2);
        std::cout << "Test 2: " << response2.getContent() << std::endl;
        
        // Test case 3: Name with special characters (XSS attempt)
        std::map<std::string, std::string> params3;
        params3["name"] = "<script>alert('XSS')</script>";
        Response response3 = home(env, params3);
        std::cout << "Test 3: " << response3.getContent() << std::endl;
        
        // Test case 4: Name with quotes
        std::map<std::string, std::string> params4;
        params4["name"] = "John \\"Doe\\"";
        Response response4 = home(env, params4);
        std::cout << "Test 4: " << response4.getContent() << std::endl;
        
        // Test case 5: Unicode name
        std::map<std::string, std::string> params5;
        params5["name"] = "José";
        Response response5 = home(env, params5);
        std::cout << "Test 5: " << response5.getContent() << std::endl;
    }
};

int main() {
    Lse130::main();
    return 0;
}
