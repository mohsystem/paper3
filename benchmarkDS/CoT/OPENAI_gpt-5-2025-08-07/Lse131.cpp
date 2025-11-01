#include <iostream>
#include <string>

struct Request {
    std::string name;
};

struct Env {
    std::string templ;
    Request request;
};

static std::string replace_all(std::string s, const std::string& from, const std::string& to) {
    if (from.empty()) return s;
    size_t pos = 0;
    while ((pos = s.find(from, pos)) != std::string::npos) {
        s.replace(pos, from.size(), to);
        pos += to.size();
    }
    return s;
}

std::string render_response_from_env(const Env& env) {
    std::string template_str = env.templ;
    std::string name = env.request.name;
    return replace_all(template_str, "{name}", name);
}

std::string home(const Env& env) {
    return render_response_from_env(env);
}

int main() {
    Env e1{ "Hello, {name}!", {"Alice"} };
    Env e2{ "Welcome {name} to the system.", {"Bob"} };
    Env e3{ "Goodbye, {name}.", {"Charlie"} };
    Env e4{ "{name}, your order is ready.", {""} };
    Env e5{ "Hi {name}, hi again {name}!", {"Dana"} };

    std::cout << home(e1) << "\n";
    std::cout << home(e2) << "\n";
    std::cout << home(e3) << "\n";
    std::cout << home(e4) << "\n";
    std::cout << home(e5) << "\n";
    return 0;
}