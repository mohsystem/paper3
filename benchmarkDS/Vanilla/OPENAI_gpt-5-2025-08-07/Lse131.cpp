#include <iostream>
#include <string>
#include <vector>

struct Env {
    std::string templateStr;
    Env() : templateStr("") {}
    Env(const std::string& t) : templateStr(t) {}
    std::string getTemplate() const { return templateStr; }
};

struct Request {
    std::string name;
    Request(const std::string& n) : name(n) {}
    std::string getParameter(const std::string& key) const {
        return (key == "name") ? name : "";
    }
};

struct Response {
    std::string body;
    Response(const std::string& b) : body(b) {}
};

Env env;
Request* currentRequest = nullptr;

static std::string replaceAll(const std::string& text, const std::string& from, const std::string& to) {
    if (from.empty()) return text;
    std::string result;
    result.reserve(text.size());
    std::size_t start = 0, pos;
    while ((pos = text.find(from, start)) != std::string::npos) {
        result.append(text, start, pos - start);
        result.append(to);
        start = pos + from.size();
    }
    result.append(text, start, std::string::npos);
    return result;
}

Response render_response_from_env(const Env& environment) {
    std::string name = (currentRequest ? currentRequest->getParameter("name") : "");
    std::string tpl = environment.getTemplate();
    std::string rendered = replaceAll(tpl, "{{name}}", name);
    return Response(rendered);
}

Response home() {
    return render_response_from_env(env);
}

int main() {
    env = Env("Hello, {{name}}!");
    std::vector<std::string> names = {"Alice", "Bob", "Charlie", "Dana", "Eve"};
    for (const auto& n : names) {
        Request req(n);
        currentRequest = &req;
        Response r = home();
        std::cout << r.body << std::endl;
        currentRequest = nullptr;
    }
    return 0;
}