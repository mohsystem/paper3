#include <iostream>
#include <string>
#include <vector>

std::string escapeHtml(const std::string& s) {
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
    std::string name = username.empty() ? "Guest" : escapeHtml(username);
    std::string html;
    html.reserve(128 + name.size());
    html += "<!DOCTYPE html>\n";
    html += "<html><head><meta charset=\"UTF-8\"><title>Hello</title></head><body>";
    html += "<h1>Hello, " + name + "!</h1>";
    html += "<p>Welcome to the hello page.</p>";
    html += "</body></html>";
    return html;
}

int main() {
    std::vector<std::string> tests = {"Alice", "Bob & Co", "<script>", "", "O'Connor"};
    for (const auto& t : tests) {
        std::cout << "=== Test ===\n";
        std::cout << http_hello_page(t) << "\n";
    }
    return 0;
}