#include <bits/stdc++.h>
using namespace std;

struct Env {
    map<string, string> request;
    string templateStr;
};

static string htmlEscape(const string& input) {
    string out;
    out.reserve(input.size());
    for (char c : input) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&#x27;"; break;
            default: out.push_back(c); break;
        }
    }
    return out;
}

static string replaceAll(const string& text, const string& from, const string& to) {
    if (from.empty()) return text;
    string result;
    result.reserve(text.size() + 16);
    size_t pos = 0, prev = 0;
    while ((pos = text.find(from, prev)) != string::npos) {
        result.append(text, prev, pos - prev);
        result.append(to);
        prev = pos + from.size();
    }
    result.append(text, prev, string::npos);
    return result;
}

string render_response_from_env(const Env& env) {
    const string name = (env.request.count("name") ? env.request.at("name") : string("World"));
    const string tmpl = (!env.templateStr.empty() ? env.templateStr : string("Hello, {{name}}!"));
    const size_t MAX_TEMPLATE_LEN = 1'000'000;
    if (tmpl.size() > MAX_TEMPLATE_LEN) {
        return "Template too large";
    }
    string safeName = htmlEscape(name);
    string rendered = replaceAll(tmpl, "{{name}}", safeName);
    return rendered;
}

string home(const Env& env) {
    return render_response_from_env(env);
}

int main() {
    // Test case 1: Normal name with default template
    Env env1;
    env1.request["name"] = "Alice";
    env1.templateStr = "Hello, {{name}}!";
    cout << home(env1) << "\n";

    // Test case 2: Missing name parameter -> default "World"
    Env env2;
    env2.templateStr = "Welcome, {{name}}.";
    cout << home(env2) << "\n";

    // Test case 3: Name with HTML that should be escaped
    Env env3;
    env3.request["name"] = "<script>alert(1)</script>";
    env3.templateStr = "<div>User: {{name}}</div>";
    cout << home(env3) << "\n";

    // Test case 4: Missing template -> default template used
    Env env4;
    env4.request["name"] = "Bob";
    env4.templateStr = ""; // missing
    cout << home(env4) << "\n";

    // Test case 5: Multiple placeholders
    Env env5;
    env5.request["name"] = "Charlie";
    env5.templateStr = "Hi {{name}}, bye {{name}}!";
    cout << home(env5) << "\n";

    return 0;
}