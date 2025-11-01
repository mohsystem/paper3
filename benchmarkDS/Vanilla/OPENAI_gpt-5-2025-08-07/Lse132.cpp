#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <unordered_map>
#include <functional>

class Router {
public:
    using Params = std::unordered_map<std::string, std::string>;
    using Handler = std::function<std::string(const Params&)>;

    void route(const std::string& pattern, Handler handler) {
        std::vector<std::string> names;
        std::string re = "^";
        for (size_t i = 0; i < pattern.size();) {
            if (pattern[i] == '<') {
                size_t j = pattern.find('>', i + 1);
                std::string name = pattern.substr(i + 1, j - (i + 1));
                names.push_back(name);
                re += "([^/]+)";
                i = j + 1;
            } else {
                char c = pattern[i++];
                if (std::string(".^$|()[]{}*+?\\").find(c) != std::string::npos) {
                    re.push_back('\\');
                }
                re.push_back(c);
            }
        }
        re += "$";
        routes_.push_back(Route{std::regex(re), names, std::move(handler)});
    }

    std::string handle(const std::string& path) const {
        for (const auto& r : routes_) {
            std::smatch m;
            if (std::regex_match(path, m, r.re)) {
                Params params;
                for (size_t i = 0; i < r.names.size(); ++i) {
                    params[r.names[i]] = m[i + 1].str();
                }
                return r.handler(params);
            }
        }
        return "404 Not Found";
    }

private:
    struct Route {
        std::regex re;
        std::vector<std::string> names;
        Handler handler;
    };
    std::vector<Route> routes_;
};

static std::unordered_map<std::string, std::string> templates = {
    {"hello.html", "Hello, {{ username }}!"}
};

std::string renderTemplate(const std::string& name, const std::unordered_map<std::string, std::string>& ctx) {
    auto it = templates.find(name);
    std::string tpl = (it == templates.end()) ? "" : it->second;
    std::string out;
    out.reserve(tpl.size() + 32);
    for (size_t i = 0; i < tpl.size();) {
        if (i + 1 < tpl.size() && tpl[i] == '{' && tpl[i + 1] == '{') {
            size_t j = tpl.find("}}", i + 2);
            if (j == std::string::npos) { out.append(tpl.substr(i)); break; }
            std::string key = tpl.substr(i + 2, j - (i + 2));
            // trim spaces
            size_t s = key.find_first_not_of(" \t\r\n");
            size_t e = key.find_last_not_of(" \t\r\n");
            std::string trimmed = (s == std::string::npos) ? "" : key.substr(s, e - s + 1);
            auto kv = ctx.find(trimmed);
            out.append(kv == ctx.end() ? "" : kv->second);
            i = j + 2;
        } else {
            out.push_back(tpl[i++]);
        }
    }
    return out;
}

// Public API: accept a URL path, return rendered output.
std::string handle_request(const std::string& path) {
    Router r;
    r.route("/hello/<username>", [](const Router::Params& params) {
        return renderTemplate("hello.html", params);
    });
    return r.handle(path);
}

int main() {
    std::vector<std::string> tests = {"Alice", "Bob", "Carol_123", "Dora", "Eve"};
    for (const auto& name : tests) {
        std::cout << handle_request("/hello/" + name) << "\n";
    }
    return 0;
}