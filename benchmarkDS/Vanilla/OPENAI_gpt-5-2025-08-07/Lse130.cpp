#include <iostream>
#include <string>

struct Request {
    std::string name;
    Request(const std::string& n = "") : name(n) {}
};

struct Environment {
    Request request;
    std::string tmpl; // 'template' is a reserved keyword in C++
    Environment(const Request& r = Request(), const std::string& t = "") : request(r), tmpl(t) {}
};

struct Response {
    std::string body;
    Response(const std::string& b = "") : body(b) {}
};

Response render_response_from_env(const Environment& env) {
    std::string tmpl = env.tmpl;
    std::string name = env.request.name.empty() ? std::string("World") : env.request.name;
    const std::string ph = "{name}";
    std::string out;
    out.reserve(tmpl.size() + 8);
    std::size_t pos = 0;
    while (true) {
        std::size_t i = tmpl.find(ph, pos);
        if (i == std::string::npos) {
            out.append(tmpl.substr(pos));
            break;
        }
        out.append(tmpl.substr(pos, i - pos));
        out.append(name);
        pos = i + ph.size();
    }
    return Response(out);
}

Response home(const Environment& env) {
    return render_response_from_env(env);
}

int main() {
    Environment tests[] = {
        Environment(Request("Alice"), "Hello, {name}!"),
        Environment(Request("Bob"), "Welcome, {name}."),
        Environment(Request(""), "Hi {name}"),
        Environment(Request("Charlie"), "{name} logged in"),
        Environment(Request("Dana"), "Bye, {name}! See you.")
    };

    for (const auto& t : tests) {
        Response r = home(t);
        std::cout << r.body << std::endl;
    }
    return 0;
}