
#include <iostream>
#include <string>
#include <map>
#include <memory>

class Template {
private:
    std::string content;
    
public:
    Template(const std::string& content) : content(content) {}
    
    std::string render(const std::map<std::string, std::string>& context) {
        std::string result = content;
        for (const auto& pair : context) {
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

class Environment {
private:
    std::map<std::string, std::shared_ptr<Template>> templates;
    
public:
    Environment() {
        templates["default"] = std::make_shared<Template>("Hello, {{name}}!");
    }
    
    std::shared_ptr<Template> getTemplate(const std::string& templateName) {
        if (templates.find(templateName) != templates.end()) {
            return templates[templateName];
        }
        return templates["default"];
    }
    
    void addTemplate(const std::string& name, const std::string& content) {
        templates[name] = std::make_shared<Template>(content);
    }
};

class Request {
private:
    std::map<std::string, std::string> parameters;
    
public:
    Request(const std::map<std::string, std::string>& parameters) : parameters(parameters) {}
    
    std::string getParameter(const std::string& key) {
        if (parameters.find(key) != parameters.end()) {
            return parameters[key];
        }
        return "Guest";
    }
};

class Response {
private:
    std::string body;
    
public:
    Response(const std::string& body) : body(body) {}
    
    std::string getBody() const {
        return body;
    }
};

Response renderResponseFromEnv(Environment& env, Request& request) {
    std::string name = request.getParameter("name");
    auto tmpl = env.getTemplate("default");
    std::map<std::string, std::string> context;
    context["name"] = name;
    std::string rendered = tmpl->render(context);
    return Response(rendered);
}

Response home(Environment& env, Request& request) {
    return renderResponseFromEnv(env, request);
}

int main() {
    Environment env;
    
    // Test case 1
    std::map<std::string, std::string> params1;
    params1["name"] = "John";
    Request request1(params1);
    Response response1 = home(env, request1);
    std::cout << "Test 1: " << response1.getBody() << std::endl;
    
    // Test case 2
    std::map<std::string, std::string> params2;
    params2["name"] = "Alice";
    Request request2(params2);
    Response response2 = home(env, request2);
    std::cout << "Test 2: " << response2.getBody() << std::endl;
    
    // Test case 3
    std::map<std::string, std::string> params3;
    params3["name"] = "Bob";
    Request request3(params3);
    Response response3 = home(env, request3);
    std::cout << "Test 3: " << response3.getBody() << std::endl;
    
    // Test case 4 - No name parameter
    std::map<std::string, std::string> params4;
    Request request4(params4);
    Response response4 = home(env, request4);
    std::cout << "Test 4: " << response4.getBody() << std::endl;
    
    // Test case 5
    std::map<std::string, std::string> params5;
    params5["name"] = "Charlie";
    Request request5(params5);
    Response response5 = home(env, request5);
    std::cout << "Test 5: " << response5.getBody() << std::endl;
    
    return 0;
}
