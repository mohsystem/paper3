#include <iostream>
#include <string>
#include <unordered_map>

struct Request {
    std::unordered_map<std::string, std::string> params;
    std::string getParam(const std::string& key) const {
        auto it = params.find(key);
        return it == params.end() ? "" : it->second;
    }
};

struct Env {
    Request request;
    std::string templ;
    std::string getTemplate() const { return templ; }
};

struct Response {
    int status;
    std::string body;
};

static std::string replaceAll(std::string s, const std::string& target, const std::string& repl) {
    if (target.empty()) return s;
    size_t pos = 0;
    while ((pos = s.find(target, pos)) != std::string::npos) {
        s.replace(pos, target.size(), repl);
        pos += repl.size();
    }
    return s;
}

Response render_response_from_env(const Env& env) {
    std::string name = env.request.getParam("name");
    if (name.empty()) name = "World";
    std::string tpl = env.getTemplate();
    std::string rendered = replaceAll(tpl, "{{name}}", name);
    return Response{200, rendered};
}

Response home(const Env& env) {
    return render_response_from_env(env);
}

int main() {
    // Test case 1
    Env e1{{{{"name", "Alice"}}}, "Hello, {{name}}!"};
    std::cout << home(e1).body << std::endl;

    // Test case 2
    Env e2{{{{"name", "Bob"}}}, "Welcome, {{name}}."};
    std::cout << home(e2).body << std::endl;

    // Test case 3 (empty name -> default)
    Env e3{{{{"name", ""}}}, "Hi, {{name}}"};
    std::cout << home(e3).body << std::endl;

    // Test case 4 (multiple occurrences)
    Env e4{{{{"name", "Eve"}}}, "User: {{name}} | Echo: {{name}}"};
    std::cout << home(e4).body << std::endl;

    // Test case 5 (no name provided -> default)
    Env e5{{{},}, "Greetings, {{name}}!"};
    std::cout << home(e5).body << std::endl;

    return 0;
}