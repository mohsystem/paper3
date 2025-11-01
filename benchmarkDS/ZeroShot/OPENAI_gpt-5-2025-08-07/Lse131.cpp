#include <iostream>
#include <string>
#include <unordered_map>

static std::string escape_html(const std::string& s) {
    std::string out;
    out.reserve(s.size() * 2);
    for (char c : s) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&#39;"; break;
            default: out.push_back(c);
        }
    }
    return out;
}

std::string render_response_from_env(const std::unordered_map<std::string, std::string>& env,
                                     const std::unordered_map<std::string, std::string>& request) {
    std::string name;
    auto itn = request.find("name");
    if (itn != request.end()) name = itn->second; else name = "";

    std::string templ;
    auto itt = env.find("template");
    if (itt != env.end() && itt->second.size() <= 10000) templ = itt->second;
    else templ = "Hello, {{name}}";

    std::string escaped = escape_html(name);
    const std::string token = "{{name}}";

    std::string res;
    res.reserve(templ.size() + (escaped.size() > token.size() ? (escaped.size() - token.size()) : 0));
    std::size_t pos = 0;
    while (true) {
        std::size_t found = templ.find(token, pos);
        if (found == std::string::npos) {
            res.append(templ, pos, std::string::npos);
            break;
        }
        res.append(templ, pos, found - pos);
        res += escaped;
        pos = found + token.size();
    }
    return res;
}

std::string home(const std::unordered_map<std::string, std::string>& env,
                 const std::unordered_map<std::string, std::string>& request) {
    return render_response_from_env(env, request);
}

int main() {
    std::unordered_map<std::string, std::string> env{{"template", "<h1>Welcome, {{name}}</h1>"}};
    std::unordered_map<std::string, std::string> r1{{"name", "Alice"}};
    std::unordered_map<std::string, std::string> r2{{"name", "<script>alert(1)</script>"}};
    std::unordered_map<std::string, std::string> r3{};
    std::unordered_map<std::string, std::string> r4{{"name", "O'Reilly & Co"}};
    std::unordered_map<std::string, std::string> r5{{"name", std::string(50, 'A')}};

    std::cout << "Test 1: " << home(env, r1) << "\n";
    std::cout << "Test 2: " << home(env, r2) << "\n";
    std::cout << "Test 3: " << home(env, r3) << "\n";
    std::cout << "Test 4: " << home(env, r4) << "\n";
    std::cout << "Test 5: " << home(env, r5) << "\n";
    return 0;
}