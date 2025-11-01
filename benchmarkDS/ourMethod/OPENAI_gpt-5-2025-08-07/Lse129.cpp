#include <iostream>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <cctype>

struct Request {
    std::unordered_map<std::string, std::string> params;
    std::string getParam(const std::string& key) const {
        auto it = params.find(key);
        if (it == params.end()) return "";
        return it->second;
    }
};

struct Env {
    Request request;
    std::string templ; // template is a reserved word in C++
};

struct Response {
    int status{200};
    std::string body;
};

static std::string sanitizeName(const std::string& rawIn) {
    std::string s = rawIn;
    // Trim
    auto notspace = [](unsigned char ch){ return !std::isspace(ch); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notspace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notspace).base(), s.end());
    if (s.empty()) return "Guest";
    std::string out;
    out.reserve(std::min<size_t>(100, s.size()));
    for (char c : s) {
        if ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            c == ' ' || c == '-' || c == '_') {
            out.push_back(c);
            if (out.size() >= 100) break;
        }
    }
    // Trim again
    out.erase(out.begin(), std::find_if(out.begin(), out.end(), notspace));
    out.erase(std::find_if(out.rbegin(), out.rend(), notspace).base(), out.end());
    if (out.empty()) return "Guest";
    return out;
}

static std::string htmlEscape(const std::string& s) {
    std::string out;
    out.reserve(std::min<size_t>(4096, s.size() * 2));
    for (char c : s) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&#x27;"; break;
            case '/': out += "&#x2F;"; break;
            default: out += c; break;
        }
        if (out.size() > 8192) {
            return "Error: content too large";
        }
    }
    return out;
}

static std::string renderTemplate(const std::string& templ, const std::string& nameVal) {
    const std::string token = "{{name}}";
    const std::string& safeTemplate = (templ.empty() || templ.size() > 8000) ? std::string("Hello, {{name}}!") : templ;
    std::string out;
    out.reserve(std::min<size_t>(8192, safeTemplate.size() + nameVal.size()));
    for (size_t i = 0; i < safeTemplate.size();) {
        if (i + token.size() <= safeTemplate.size() && safeTemplate.compare(i, token.size(), token) == 0) {
            out += nameVal;
            i += token.size();
        } else {
            out.push_back(safeTemplate[i]);
            i++;
        }
        if (out.size() > 8192) {
            return "Error: content too large";
        }
    }
    return out;
}

Response render_response_from_env(const Env& env) {
    std::string raw = env.request.getParam("name");
    std::string sanitized = sanitizeName(raw);
    std::string escaped = htmlEscape(sanitized);
    std::string body = renderTemplate(env.templ, escaped);
    return Response{200, body};
}

Response home(const Env& env) {
    return render_response_from_env(env);
}

int main() {
    // Test 1
    Env e1;
    e1.request.params["name"] = "Alice";
    e1.templ = "Hello, {{name}}! Welcome.";
    std::cout << home(e1).body << "\n";

    // Test 2
    Env e2;
    e2.request.params["name"] = "Bob <script>alert(1)</script>";
    e2.templ = "Hi {{name}}, your input is safe.";
    std::cout << home(e2).body << "\n";

    // Test 3
    Env e3;
    e3.request.params["name"] = "   ";
    e3.templ = "Hello, {{name}}!";
    std::cout << home(e3).body << "\n";

    // Test 4
    Env e4;
    e4.request.params["name"] = std::string(150, 'X');
    e4.templ = "Long name test: {{name}}.";
    std::cout << home(e4).body << "\n";

    // Test 5
    Env e5;
    e5.request.params["name"] = "Eve-User_42";
    e5.templ = ""; // will default internally
    std::cout << home(e5).body << "\n";

    return 0;
}