#include <iostream>
#include <string>

struct Request {
    std::string name;
};

struct Environment {
    std::string templ;
    Request request;
    std::string get_template() const { return templ; }
};

struct Response {
    std::string content;
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

static std::string render_template(const std::string& templ, const std::string& name) {
    std::string rendered = replace_all(templ, "{{ name }}", name);
    rendered = replace_all(rendered, "{{name}}", name);
    return rendered;
}

Response render_response_from_env(const Environment& env) {
    std::string name = env.request.name;
    std::string templ = env.get_template();
    std::string rendered = render_template(templ, name);
    return Response{rendered};
}

Response home(const Environment& env) {
    return render_response_from_env(env);
}

int main() {
    // Test case 1
    {
        Environment env{"Hello, {{ name }}!", Request{"Alice"}};
        Response res = home(env);
        std::cout << res.content << std::endl;
    }
    // Test case 2
    {
        Environment env{"Welcome, {{name}}.", Request{"Bob"}};
        Response res = home(env);
        std::cout << res.content << std::endl;
    }
    // Test case 3
    {
        Environment env{"Hi {{ name }}, you're number {{ name }}", Request{"Eve"}};
        Response res = home(env);
        std::cout << res.content << std::endl;
    }
    // Test case 4
    {
        Environment env{"No placeholder here.", Request{"Carol"}};
        Response res = home(env);
        std::cout << res.content << std::endl;
    }
    // Test case 5
    {
        Environment env{"{{ name }} only", Request{""}};
        Response res = home(env);
        std::cout << res.content << std::endl;
    }
    return 0;
}