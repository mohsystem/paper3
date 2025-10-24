// Chain-of-Through Step 1: Problem understanding
// Safely return an HTML page showing user input.
//
// Step 2: Security requirements
// - Escape HTML to prevent XSS
// - No unsafe buffers
//
// Step 3: Secure coding generation
// - Robust escaping with std::string
//
// Step 4: Code review
// - Coverage: &, <, >, ", ', /
//
// Step 5: Secure code output

#include <iostream>
#include <string>

std::string escapeHtml(const std::string& s) {
    std::string out;
    out.reserve(s.size() * 6);
    for (char c : s) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&#39;"; break;
            case '/': out += "&#47;"; break;
            default: out.push_back(c); break;
        }
    }
    return out;
}

std::string displayUserInputOnWebPage(const std::string& userInput) {
    std::string safe = escapeHtml(userInput);
    std::string html;
    html.reserve(safe.size() + 256);
    html += "<!doctype html><html lang=\"en\"><head>";
    html += "<meta charset=\"utf-8\">";
    html += "<meta http-equiv=\"Content-Security-Policy\" content=\"default-src 'none'; style-src 'self' 'unsafe-inline';\">";
    html += "<title>Safe Echo</title></head><body>";
    html += "<h1>User Input</h1><p>";
    html += safe;
    html += "</p></body></html>";
    return html;
}

int main() {
    std::string tests[] = {
        "Hello, world!",
        "<script>alert('xss')</script>",
        "Tom & Jerry > Spike",
        "\"quoted\" and 'single'",
        "こんにちは <b>世界</b> & emojis 😊"
    };
    for (const auto& t : tests) {
        std::cout << "----- C++ Output -----\n";
        std::cout << displayUserInputOnWebPage(t) << "\n";
    }
    return 0;
}