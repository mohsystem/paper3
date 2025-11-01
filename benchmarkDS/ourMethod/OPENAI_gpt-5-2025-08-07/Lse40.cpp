#include <iostream>
#include <string>
#include <unordered_map>
#include <cctype>

static std::string htmlEscape(const std::string& s) {
    if (s.empty()) return "N/A";
    std::string out;
    out.reserve(s.size() + 16);
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

static std::string last4OfSSN(const std::string& ssn) {
    if (ssn.empty()) return "N/A";
    std::string digits;
    digits.reserve(32);
    for (char c : ssn) {
        if (std::isdigit(static_cast<unsigned char>(c))) {
            if (digits.size() < 64) digits.push_back(c);
        }
    }
    if (digits.size() < 4) return "N/A";
    return digits.substr(digits.size() - 4);
}

static std::string httpResponse(int code, const std::string& contentType, const std::string& body) {
    std::string reason;
    switch (code) {
        case 200: reason = "OK"; break;
        case 400: reason = "Bad Request"; break;
        case 404: reason = "Not Found"; break;
        case 405: reason = "Method Not Allowed"; break;
        default: reason = "OK"; break;
    }
    std::string header = "HTTP/1.1 " + std::to_string(code) + " " + reason + "\r\n"
                         "Content-Type: " + contentType + "\r\n"
                         "Content-Length: " + std::to_string(body.size()) + "\r\n"
                         "Connection: close\r\n\r\n";
    return header + body;
}

static std::string info(const std::unordered_map<std::string, std::string>& session) {
    auto itUser = session.find("username");
    auto itEmail = session.find("email");
    auto itSSN = session.find("ssn");
    std::string username = (itUser != session.end() ? htmlEscape(itUser->second) : "N/A");
    std::string email = (itEmail != session.end() ? htmlEscape(itEmail->second) : "N/A");
    std::string ssnLast4 = (itSSN != session.end() ? htmlEscape(last4OfSSN(itSSN->second)) : "N/A");

    std::string body;
    body.reserve(256);
    body += "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>Info</title></head><body>";
    body += "<h1>User Info</h1><ul>";
    body += "<li>Username: " + username + "</li>";
    body += "<li>Email: " + email + "</li>";
    body += "<li>SSN Last 4: " + ssnLast4 + "</li>";
    body += "</ul></body></html>";

    return httpResponse(200, "text/html; charset=UTF-8", body);
}

static std::string handleRequest(const std::string& method, const std::string& path, const std::unordered_map<std::string, std::string>& session) {
    if (method.empty() || path.empty()) {
        return httpResponse(400, "text/plain; charset=UTF-8", "Bad Request");
    }
    if (method != "GET") {
        return httpResponse(405, "text/plain; charset=UTF-8", "Method Not Allowed");
    }
    if (path == "/info") {
        return info(session);
    }
    return httpResponse(404, "text/plain; charset=UTF-8", "Not Found");
}

int main() {
    // Test 1: Valid session
    std::unordered_map<std::string, std::string> s1{
        {"username", "alice"},
        {"email", "alice@example.com"},
        {"ssn", "123-45-6789"}
    };
    std::cout << handleRequest("GET", "/info", s1) << std::endl;

    // Test 2: Long spaced SSN -> last 4 only
    std::unordered_map<std::string, std::string> s2{
        {"username", "bob"},
        {"email", "bob@example.com"},
        {"ssn", "0000 1111 2222 3333 4444 5555 6666 7777 8888 9999"}
    };
    std::cout << handleRequest("GET", "/info", s2) << std::endl;

    // Test 3: Malicious username (escaped)
    std::unordered_map<std::string, std::string> s3{
        {"username", "<script>alert('x')</script>"},
        {"email", "evil@example.com"},
        {"ssn", "111223333"}
    };
    std::cout << handleRequest("GET", "/info", s3) << std::endl;

    // Test 4: Missing SSN -> N/A
    std::unordered_map<std::string, std::string> s4{
        {"username", "charlie"},
        {"email", "charlie@example.com"}
    };
    std::cout << handleRequest("GET", "/info", s4) << std::endl;

    // Test 5: Wrong path and wrong method
    std::unordered_map<std::string, std::string> s5{
        {"username", "dana"},
        {"email", "dana@example.com"},
        {"ssn", "444-55-1234"}
    };
    std::cout << handleRequest("GET", "/unknown", s5) << std::endl;
    std::cout << handleRequest("POST", "/info", s5) << std::endl;

    return 0;
}