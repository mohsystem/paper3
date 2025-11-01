#include <iostream>
#include <sstream>
#include <string>

std::string escape_html(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&#39;"; break;
            default: out += c; break;
        }
    }
    return out;
}

std::string http_hello_page(const std::string& username) {
    std::string u = username.empty() ? "World" : username;
    std::string safe = escape_html(u);
    std::string body = "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>Hello</title></head><body><h1>Hello, " + safe + "!</h1></body></html>";
    std::ostringstream response_stream;
    response_stream << body;
    // stream would be closed after scope ends; return the body content
    return response_stream.str();
}

int main() {
    std::string tests[5] = {
        "Alice",
        "",
        "Bob & Co",
        "<script>alert(1)</script>",
        "O'Neil"
    };
    for (const auto& t : tests) {
        std::cout << http_hello_page(t) << std::endl;
    }
    return 0;
}