#include <iostream>
#include <string>
#include <cctype>
#include <algorithm>

static const size_t MAX_NAME_LEN = 100;
static const size_t MAX_TEMPLATE_LEN = 4096;

struct Env {
    std::string templ;
    std::string requestName;
    Env(const std::string& t, const std::string& n) : templ(t), requestName(n) {}
};

struct Response {
    int status;
    std::string body;
    Response(int s, const std::string& b) : status(s), body(b) {}
};

static std::string sanitizeName(const std::string& name) {
    if (name.empty()) return "Guest";
    std::string out;
    out.reserve(std::min(name.size(), MAX_NAME_LEN));
    size_t limit = std::min(name.size(), MAX_NAME_LEN);
    for (size_t i = 0; i < limit; ++i) {
        unsigned char c = static_cast<unsigned char>(name[i]);
        if (std::isalnum(c) || c == ' ' || c == '-' || c == '_' || c == '.' || c == ',') {
            out.push_back(static_cast<char>(c));
        }
    }
    // trim
    auto notSpace = [](int ch){ return !std::isspace(ch); };
    out.erase(out.begin(), std::find_if(out.begin(), out.end(), notSpace));
    out.erase(std::find_if(out.rbegin(), out.rend(), notSpace).base(), out.end());
    if (out.empty()) return "Guest";
    return out;
}

static std::string escapeHtml(const std::string& s) {
    std::string out;
    out.reserve(std::min(s.size() * 6, static_cast<size_t>(1024)));
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

static std::string renderTemplate(const std::string& templ, const std::string& escapedName) {
    if (templ.size() > MAX_TEMPLATE_LEN) {
        return "Invalid template";
    }
    std::string result;
    result.reserve(templ.size() + 16);
    const std::string placeholder = "{name}";
    size_t pos = 0, found;
    while ((found = templ.find(placeholder, pos)) != std::string::npos) {
        result.append(templ, pos, found - pos);
        result.append(escapedName);
        pos = found + placeholder.size();
    }
    result.append(templ, pos, templ.size() - pos);
    return result;
}

Response render_response_from_env(const Env& env) {
    std::string sanitized = sanitizeName(env.requestName);
    std::string escaped = escapeHtml(sanitized);
    std::string rendered = renderTemplate(env.templ, escaped);
    return Response(200, rendered);
}

Response home(const Env& env) {
    return render_response_from_env(env);
}

int main() {
    Env t1("Hello, {name}!", "Alice");
    Env t2("<div>User: {name}</div>", "Bob <script>alert(1)</script>");
    Env t3("Len test: {name}", std::string(500, 'a'));
    Env t4("Safe SQL-ish: {name}", "Eve; DROP TABLE users;--");
    Env t5("Welcome!", "");

    Response r1 = home(t1);
    Response r2 = home(t2);
    Response r3 = home(t3);
    Response r4 = home(t4);
    Response r5 = home(t5);

    std::cout << "Test 1 -> Status: " << r1.status << ", Body: " << r1.body << "\n";
    std::cout << "Test 2 -> Status: " << r2.status << ", Body: " << r2.body << "\n";
    std::cout << "Test 3 -> Status: " << r3.status << ", Body: " << r3.body << "\n";
    std::cout << "Test 4 -> Status: " << r4.status << ", Body: " << r4.body << "\n";
    std::cout << "Test 5 -> Status: " << r5.status << ", Body: " << r5.body << "\n";

    return 0;
}