#include <bits/stdc++.h>
using namespace std;

struct Request {
    unordered_map<string, string> params;
    Request(const unordered_map<string, string>& p = {}) : params(p) {}
    string getParam(const string& key) const {
        auto it = params.find(key);
        if (it == params.end()) return "";
        return it->second;
    }
};

struct TemplateT {
    string templateText;
    TemplateT(const string& t = "") : templateText(t) {}
    static string htmlEscape(const string& s) {
        string out;
        out.reserve(s.size());
        for (char c : s) {
            switch (c) {
                case '&': out += "&amp;"; break;
                case '<': out += "&lt;"; break;
                case '>': out += "&gt;"; break;
                case '"': out += "&quot;"; break;
                case '\'': out += "&#x27;"; break;
                case '/': out += "&#x2F;"; break;
                default: out.push_back(c); break;
            }
        }
        return out;
    }
    string render(const unordered_map<string, string>& context) const {
        auto it = context.find("name");
        string name = (it == context.end() ? "" : it->second);
        string safe = htmlEscape(name);
        // Replace all occurrences of {{name}}
        string result;
        result.reserve(templateText.size() + safe.size());
        size_t pos = 0;
        const string token = "{{name}}";
        while (true) {
            size_t idx = templateText.find(token, pos);
            if (idx == string::npos) {
                result.append(templateText, pos, string::npos);
                break;
            }
            result.append(templateText, pos, idx - pos);
            result.append(safe);
            pos = idx + token.size();
        }
        return result;
    }
};

struct Environment {
    Request request;
    TemplateT tmpl;
    Environment(const Request& r, const TemplateT& t) : request(r), tmpl(t) {}
    const Request& getRequest() const { return request; }
    const TemplateT& getTemplate() const { return tmpl; }
};

struct Response {
    int status;
    string body;
    Response(int s = 200, const string& b = "") : status(s), body(b) {}
    string toString() const {
        return string("Status: ") + to_string(status) + ", Body: " + body;
    }
};

static string sanitizeName(const string& name) {
    string n = name;
    // trim
    auto l = n.find_first_not_of(" \t\n\r\f\v");
    auto r = n.find_last_not_of(" \t\n\r\f\v");
    if (l == string::npos) n.clear();
    else n = n.substr(l, r - l + 1);
    // remove control characters
    string cleaned;
    cleaned.reserve(n.size());
    for (unsigned char c : n) {
        if (c >= 32 && c != 127) cleaned.push_back(static_cast<char>(c));
    }
    // limit length
    if (cleaned.size() > 100) cleaned.resize(100);
    if (cleaned.empty()) cleaned = "Guest";
    return cleaned;
}

Response render_response_from_env(const Environment& env) {
    string rawName = env.getRequest().getParam("name");
    string safeName = sanitizeName(rawName);
    const TemplateT& tpl = env.getTemplate();
    string rendered = tpl.render({{"name", safeName}});
    return Response(200, rendered);
}

Response home(const Environment& env) {
    return render_response_from_env(env);
}

int main() {
    TemplateT tpl("Hello, {{name}}!");
    // Test 1
    Response r1 = home(Environment(Request({{"name", "Alice"}}), tpl));
    cout << r1.toString() << "\n";
    // Test 2
    Response r2 = home(Environment(Request({}), tpl));
    cout << r2.toString() << "\n";
    // Test 3
    Response r3 = home(Environment(Request({{"name", "<script>alert(1)</script>"}}), tpl));
    cout << r3.toString() << "\n";
    // Test 4
    Response r4 = home(Environment(Request({{"name", "  Bob \n\t "}}), tpl));
    cout << r4.toString() << "\n";
    // Test 5
    string longName(200, 'A');
    Response r5 = home(Environment(Request({{"name", longName}}), tpl));
    cout << r5.toString() << "\n";
    return 0;
}