
#include <iostream>
#include <string>
#include <map>
#include <memory>

class Template {
private:
    std::string templateContent;
    
public:
    Template(const std::string& content) : templateContent(content) {}
    
    std::string render(const std::map<std::string, std::string>& context) {
        std::string result = templateContent;
        for (const auto& pair : context) {
            std::string placeholder = "{{" + pair.first + "}}";
            size_t pos = result.find(placeholder);
            if (pos != std::string::npos) {
                result.replace(pos, placeholder.length(), pair.second);
            }
        }
        return result;
    }
};

class Request {
private:
    std::map<std::string, std::string> parameters;
    
public:
    Request(const std::map<std::string, std::string>& params) : parameters(params) {}
    
    std::string getParameter(const std::string& name) {
        auto it = parameters.find(name);
        if (it != parameters.end()) {
            return it->second;
        }
        return "";
    }
};

class Environment {
private:
    Request* request;
    std::map<std::string, Template> templates;
    
public:
    Environment(Request* req) : request(req) {
        templates["default"] = Template("Hello, {{name}}!");
    }
    
    std::string getNameParameter() {
        return request->getParameter("name");
    }
    
    Template* getTemplate(const std::string& templateName) {
        return &templates[templateName];
    }
};

class Response {
private:
    std::string content;
    
public:
    Response(const std::string& cont) : content(cont) {}
    
    std::string getContent() const {
        return content;
    }
};

Response renderResponseFromEnv(Environment* env) {
    std::string name = env->getNameParameter();
    Template* tmpl = env->getTemplate("default");
    
    std::map<std::string, std::string> context;
    context["name"] = name.empty() ? "World" : name;
    
    std::string renderedContent = tmpl->render(context);
    return Response(renderedContent);
}

Response home(Environment* env) {
    return renderResponseFromEnv(env);
}

int main() {
    // Test case 1
    std::map<std::string, std::string> params1 = {{"name", "Alice"}};
    Request request1(params1);
    Environment env1(&request1);
    Response response1 = home(&env1);
    std::cout << "Test 1: " << response1.getContent() << std::endl;
    
    // Test case 2
    std::map<std::string, std::string> params2 = {{"name", "Bob"}};
    Request request2(params2);
    Environment env2(&request2);
    Response response2 = home(&env2);
    std::cout << "Test 2: " << response2.getContent() << std::endl;
    
    // Test case 3
    std::map<std::string, std::string> params3 = {{"name", "Charlie"}};
    Request request3(params3);
    Environment env3(&request3);
    Response response3 = home(&env3);
    std::cout << "Test 3: " << response3.getContent() << std::endl;
    
    // Test case 4
    std::map<std::string, std::string> params4 = {{"name", "Diana"}};
    Request request4(params4);
    Environment env4(&request4);
    Response response4 = home(&env4);
    std::cout << "Test 4: " << response4.getContent() << std::endl;
    
    // Test case 5
    std::map<std::string, std::string> params5;
    Request request5(params5);
    Environment env5(&request5);
    Response response5 = home(&env5);
    std::cout << "Test 5: " << response5.getContent() << std::endl;
    
    return 0;
}
