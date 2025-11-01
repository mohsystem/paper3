
#include <iostream>
#include <string>
#include <map>
#include <sstream>

class Environment {
private:
    std::map<std::string, std::string> templates;
    
public:
    Environment() {
        templates["main"] = "Hello, {{name}}! Welcome to our service.";
    }
    
    std::string getTemplate(const std::string& templateName) {
        if (templates.find(templateName) != templates.end()) {
            return templates[templateName];
        }
        return "Template not found";
    }
    
    std::string render(std::string templateStr, const std::map<std::string, std::string>& params) {
        std::string result = templateStr;
        for (const auto& pair : params) {
            std::string placeholder = "{{" + pair.first + "}}";
            size_t pos = result.find(placeholder);
            while (pos != std::string::npos) {
                result.replace(pos, placeholder.length(), pair.second);
                pos = result.find(placeholder, pos + pair.second.length());
            }
        }
        return result;
    }
};

class Request {
private:
    std::map<std::string, std::string> params;
    
public:
    Request(const std::string& queryString) {
        if (!queryString.empty()) {
            std::istringstream stream(queryString);
            std::string pair;
            while (std::getline(stream, pair, '&')) {
                size_t pos = pair.find('=');
                if (pos != std::string::npos) {
                    std::string key = pair.substr(0, pos);
                    std::string value = pair.substr(pos + 1);
                    params[key] = value;
                }
            }
        }
    }
    
    std::string getParameter(const std::string& key, const std::string& defaultValue = "Guest") {
        if (params.find(key) != params.end()) {
            return params[key];
        }
        return defaultValue;
    }
};

class Response {
private:
    std::string content;
    
public:
    Response(const std::string& content) : content(content) {}
    
    std::string getContent() const {
        return content;
    }
};

Response renderResponseFromEnv(Environment& env, Request& request) {
    std::string name = request.getParameter("name");
    std::string templateStr = env.getTemplate("main");
    std::map<std::string, std::string> params;
    params["name"] = name;
    std::string renderedTemplate = env.render(templateStr, params);
    return Response(renderedTemplate);
}

Response home(Environment& env, Request& request) {
    return renderResponseFromEnv(env, request);
}

int main() {
    Environment env;
    
    // Test case 1: Name parameter provided
    Request req1("name=John");
    Response resp1 = home(env, req1);
    std::cout << "Test 1: " << resp1.getContent() << std::endl;
    
    // Test case 2: Name parameter with different value
    Request req2("name=Alice");
    Response resp2 = home(env, req2);
    std::cout << "Test 2: " << resp2.getContent() << std::endl;
    
    // Test case 3: No name parameter (should default to Guest)
    Request req3("");
    Response resp3 = home(env, req3);
    std::cout << "Test 3: " << resp3.getContent() << std::endl;
    
    // Test case 4: Name parameter with another value
    Request req4("name=Bob");
    Response resp4 = home(env, req4);
    std::cout << "Test 4: " << resp4.getContent() << std::endl;
    
    // Test case 5: Name parameter with special name
    Request req5("name=Charlie");
    Response resp5 = home(env, req5);
    std::cout << "Test 5: " << resp5.getContent() << std::endl;
    
    return 0;
}
