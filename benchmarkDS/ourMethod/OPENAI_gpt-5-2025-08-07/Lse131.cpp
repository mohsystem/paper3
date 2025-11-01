#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <algorithm>

struct Environment {
    std::string templateStr;
    explicit Environment(const std::string& t) : templateStr(t) {}
};

struct Request {
    std::string name;
    explicit Request(const std::string& n) : name(n) {}
};

static std::string htmlEscape(const std::string& in) {
    std::string out;
    out.reserve(std::min<size_t>(4096, in.size() + 16));
    for (char c : in) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&#x27;"; break;
            default: out.push_back(c); break;
        }
        if (out.size() >= 4096) {
            out.resize(4096);
            break;
        }
    }
    return out;
}

static bool isAllowedChar(char c) {
    return std::isalnum(static_cast<unsigned char>(c)) || c==' ' || c=='_' || c=='-' || c=='.';
}

static std::string sanitizeName(const std::string& input) {
    // Trim
    size_t start = 0;
    while (start < input.size() && std::isspace(static_cast<unsigned char>(input[start]))) start++;
    size_t end = input.size();
    while (end > start && std::isspace(static_cast<unsigned char>(input[end-1]))) end--;
    std::string trimmed = input.substr(start, end - start);
    if (trimmed.empty() || trimmed.size() > 100) return "Guest";
    for (char c : trimmed) {
        if (!isAllowedChar(c)) return "Guest";
    }
    return trimmed;
}

static std::string getTemplate(const Environment* env) {
    const std::string def = "Hello, {{name}}!";
    if (!env) return def;
    const std::string& t = env->templateStr;
    if (t.empty()) return def;
    if (t.size() > 2000) return def;
    return t;
}

std::string render_response_from_env(const Environment& env, const Request& req) {
    std::string name = sanitizeName(req.name);
    std::string escaped = htmlEscape(name);
    std::string tpl = getTemplate(&env);

    const std::string token = "{{name}}";
    std::string out;
    out.reserve(std::min<size_t>(4096, tpl.size() + escaped.size() + 16));

    for (size_t i = 0; i < tpl.size();) {
        if (i + token.size() <= tpl.size() && tpl.compare(i, token.size(), token) == 0) {
            if (out.size() + escaped.size() > 4096) {
                size_t remain = 4096 - out.size();
                out.append(escaped.data(), std::min(remain, escaped.size()));
                break;
            }
            out += escaped;
            i += token.size();
        } else {
            if (out.size() + 1 > 4096) break;
            out.push_back(tpl[i]);
            i++;
        }
    }
    return out;
}

std::string home(const Environment& env, const Request& req) {
    return render_response_from_env(env, req);
}

int main() {
    std::vector<std::string> outputs;

    // Test 1: Normal
    outputs.push_back(home(Environment("Welcome, {{name}}."), Request("Alice")));

    // Test 2: Escaping
    outputs.push_back(home(Environment("<h1>Hello, {{name}}</h1>"), Request("<script>alert(1)</script>")));

    // Test 3: Too long name
    outputs.push_back(home(Environment("Hi {{name}}!"), Request(std::string(150, 'A')));

    // Test 4: Invalid chars
    outputs.push_back(home(Environment("User: {{name}}"), Request("Bob!!@@##")));

    // Test 5: Missing template (default)
    outputs.push_back(home(Environment(""), Request("Charlie")));

    for (size_t i = 0; i < outputs.size(); ++i) {
        std::cout << "Test " << (i + 1) << ": " << outputs[i] << "\n";
    }

    return 0;
}