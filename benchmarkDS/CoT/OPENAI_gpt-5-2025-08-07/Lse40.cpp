// Chain-of-Through process in code generation:
// 1) Problem understanding:
//    - A simple web-like application with single route: GET /info.
//    - info() reads session (unordered_map) for username, email, ssn; returns HTML with username,
//      email, and last 4 digits of SSN.
// 2) Security requirements:
//    - Escape HTML for username/email to avoid XSS.
//    - Only show last 4 digits of SSN; strip non-digits; mask if fewer than 4 digits.
// 3) Secure coding generation:
//    - Implement HTML escape and safe last-4 extraction.
// 4) Code review:
//    - No full SSN exposure, no logging sensitive data.
// 5) Secure code output:
//    - Provide info(session) and handle(method, path, session) with 5 test cases.

#include <iostream>
#include <string>
#include <unordered_map>

class Lse40 {
public:
    static std::string escapeHtml(const std::string& s) {
        std::string out;
        out.reserve(s.size());
        for (char c : s) {
            switch (c) {
                case '&': out += "&amp;"; break;
                case '<': out += "&lt;"; break;
                case '>': out += "&gt;"; break;
                case '"': out += "&quot;"; break;
                case '\'': out += "&#x27;"; break;
                default: out += c; break;
            }
        }
        return out;
    }

    static std::string last4Safe(const std::string& ssn) {
        std::string digits;
        digits.reserve(ssn.size());
        for (char c : ssn) {
            if (c >= '0' && c <= '9') digits.push_back(c);
        }
        if (digits.size() >= 4) {
            return digits.substr(digits.size() - 4);
        } else {
            std::string res;
            res.assign(4 - digits.size(), '*');
            res += digits;
            return res;
        }
    }

    static std::string info(std::unordered_map<std::string, std::string>& session) {
        if (session.find("username") == session.end()) {
            session["username"] = "Jane Doe";
        }
        if (session.find("email") == session.end()) {
            session["email"] = "jane.doe@example.com";
        }
        if (session.find("ssn") == session.end()) {
            session["ssn"] = "123-45-6789";
        }

        std::string username = escapeHtml(session["username"]);
        std::string email = escapeHtml(session["email"]);
        std::string ssnLast4 = last4Safe(session["ssn"]);

        std::string html;
        html.reserve(256);
        html += "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>User Info</title></head><body>";
        html += "<h1>User Information</h1><ul>";
        html += "<li>Username: " + username + "</li>";
        html += "<li>Email: " + email + "</li>";
        html += "<li>SSN (last 4): " + ssnLast4 + "</li>";
        html += "</ul></body></html>";
        return std::move(html);
    }

    static std::string handle(const std::string& method, const std::string& path, std::unordered_map<std::string, std::string>& session) {
        if (!method.empty() && !path.empty() && (method == "GET" || method == "get" || method == "Get") && path == "/info") {
            return info(session);
        }
        return "404 Not Found";
    }
};

int main() {
    // Test 1: Empty session
    std::unordered_map<std::string, std::string> s1;
    std::cout << Lse40::handle("GET", "/info", s1) << "\n-----\n";

    // Test 2: Provided clean data
    std::unordered_map<std::string, std::string> s2{
        {"username", "Alice"},
        {"email", "alice@example.com"},
        {"ssn", "987654321"}
    };
    std::cout << Lse40::handle("GET", "/info", s2) << "\n-----\n";

    // Test 3: XSS attempt in username; short SSN
    std::unordered_map<std::string, std::string> s3{
        {"username", "<script>alert(1)</script>"},
        {"email", "bob@example.com"},
        {"ssn", "12"}
    };
    std::cout << Lse40::handle("GET", "/info", s3) << "\n-----\n";

    // Test 4: SSN with non-digits and spaces
    std::unordered_map<std::string, std::string> s4{
        {"username", "Charlie"},
        {"email", "charlie+tag@example.co.uk"},
        {"ssn", "  999-88  7777 "}
    };
    std::cout << Lse40::handle("GET", "/info", s4) << "\n-----\n";

    // Test 5: Missing ssn; present username/email
    std::unordered_map<std::string, std::string> s5{
        {"username", "Dana"},
        {"email", "dana@example.org"}
    };
    std::cout << Lse40::handle("GET", "/info", s5) << "\n";

    return 0;
}