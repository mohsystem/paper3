#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

using namespace std;

using Handler = function<string(const unordered_map<string,string>&)>;

struct Route {
    string pattern;
    Handler handler;
    Route(const string& p, Handler h) : pattern(p), handler(std::move(h)) {}
};

vector<Route> routes;

string escapeHTML(const string& s) {
    string out;
    out.reserve(s.size());
    for (char c : s) {
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

string hello(const string& username) {
    return string("Hello ") + escapeHTML(username);
}

static string trimSlashes(const string& s) {
    size_t start = 0, end = s.size();
    while (start < end && s[start] == '/') start++;
    while (end > start && s[end - 1] == '/') end--;
    return s.substr(start, end - start);
}

static vector<string> splitBySlash(const string& s) {
    vector<string> parts;
    string cur;
    for (char c : s) {
        if (c == '/') {
            parts.push_back(cur);
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    parts.push_back(cur);
    return parts;
}

bool match(const string& pattern, const string& url, unordered_map<string,string>& params) {
    string tp = trimSlashes(pattern);
    string tu = trimSlashes(url);
    vector<string> p = splitBySlash(tp);
    vector<string> u = splitBySlash(tu);
    if (pattern == "/") p = {""};
    if (url == "/") u = {""};
    if (p.size() != u.size()) return false;
    params.clear();
    for (size_t i = 0; i < p.size(); ++i) {
        const string& pp = p[i];
        const string& uu = u[i];
        if (pp.size() >= 2 && pp.front() == '<' && pp.back() == '>') {
            string name = pp.substr(1, pp.size() - 2);
            params[name] = uu;
        } else {
            if (pp != uu) return false;
        }
    }
    return true;
}

void route(const string& pattern, Handler handler) {
    routes.emplace_back(pattern, std::move(handler));
}

string handle_request(const string& url) {
    unordered_map<string,string> params;
    for (auto& r : routes) {
        if (match(r.pattern, url, params)) {
            return r.handler(params);
        }
    }
    return "404 Not Found";
}

int main() {
    route("/hello/<username>", [](const unordered_map<string,string>& params) {
        auto it = params.find("username");
        string username = (it != params.end() ? it->second : "");
        return hello(username);
    });

    vector<string> tests = {
        "/hello/Alice",
        "/hello/Bob & Co",
        "/hello/User<Name>",
        "/hello/Quote\"User'",
        "/hello/Greater>Less<",
    };
    for (const auto& t : tests) {
        cout << handle_request(t) << "\n";
    }
    return 0;
}