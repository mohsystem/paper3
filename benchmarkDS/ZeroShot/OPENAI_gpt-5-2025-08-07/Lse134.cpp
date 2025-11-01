#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <functional>

static std::string escapeHtml(const std::string& input) {
    std::string out;
    out.reserve(input.size());
    for (char c : input) {
        switch (c) {
            case '&': out.append("&amp;"); break;
            case '<': out.append("&lt;"); break;
            case '>': out.append("&gt;"); break;
            case '"': out.append("&quot;"); break;
            case '\'': out.append("&#39;"); break;
            default: out.push_back(c);
        }
    }
    return out;
}

static std::string hello(const std::string& username) {
    return std::string("Hello ") + escapeHtml(username);
}

class Router {
public:
    using Handler = std::function<std::string(const std::vector<std::string>&)>;

    void addRoute(const std::string& templ, Handler h) {
        // Convert "/hello/{username}" -> "^/hello/([^/]+)$"
        std::string escaped;
        escaped.reserve(templ.size() * 2);
        for (char c : templ) {
            if (std::string("().[]{}^$|*+?\\").find(c) != std::string::npos) {
                escaped.push_back('\\');
            }
            escaped.push_back(c);
        }
        std::string regexStr;
        // Replace "{username}" with capture group
        const std::string needle = "\\{username\\}";
        // But we escaped braces too; actual sequence is "\{username\}"
        // Let's find and replace that sequence.
        // Build the escaped braces sequence
        std::string escBraceSeq = "\\{username\\}";
        // Perform replacement
        size_t pos = 0;
        while (true) {
            size_t found = escaped.find(escBraceSeq, pos);
            if (found == std::string::npos) {
                regexStr.append(escaped.substr(pos));
                break;
            }
            regexStr.append(escaped.substr(pos, found - pos));
            regexStr.append("([^/]+)");
            pos = found + escBraceSeq.size();
        }
        regexStr = std::string("^") + regexStr + "$";
        std::regex re(regexStr);
        routes_.push_back({re, std::move(h)});
    }

    std::string navigate(const std::string& url) const {
        for (const auto& r : routes_) {
            std::smatch m;
            if (std::regex_match(url, m, r.re)) {
                std::vector<std::string> params;
                for (size_t i = 1; i < m.size(); ++i) {
                    params.push_back(m[i].str());
                }
                return r.h(params);
            }
        }
        return std::string();
    }

private:
    struct Route {
        std::regex re;
        Handler h;
    };
    std::vector<Route> routes_;
};

int main() {
    Router router;
    router.addRoute("/hello/{username}", [](const std::vector<std::string>& params) {
        return hello(params.empty() ? std::string() : params[0]);
    });

    std::vector<std::string> tests = {
        "/hello/World",
        "/hello/<script>alert(1)</script>",
        "/hello/O'Connor & Sons",
        "/hello/Tom \"T\"",
        "/hello/../path?x=1&y=2"
    };

    for (const auto& t : tests) {
        std::string res = router.navigate(t);
        if (!res.empty()) {
            std::cout << res << "\n";
        } else {
            std::cout << "404 Not Found\n";
        }
    }
    return 0;
}