#include <iostream>
#include <string>
#include <sstream>

static const std::string HELLO_HTML =
    "<!doctype html>\n"
    "<html lang=\"en\">\n"
    "<head>\n"
    "  <meta charset=\"utf-8\">\n"
    "  <title>Hello</title>\n"
    "</head>\n"
    "<body>\n"
    "  <h1>Hello, {{username}}</h1>\n"
    "</body>\n"
    "</html>\n";

std::string html_escape(const std::string& s) {
    std::string out;
    out.reserve(s.size() * 2);
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

std::string render_hello(const std::string& username) {
    std::string safe = html_escape(username);
    std::string result = HELLO_HTML;
    const std::string placeholder = "{{username}}";
    size_t pos = result.find(placeholder);
    if (pos != std::string::npos) {
        result.replace(pos, placeholder.size(), safe);
    }
    return result;
}

int main() {
    // 5 test cases
    std::string tests[5] = {
        "Alice",
        "Bob",
        "Eve<script>",
        "李雷",
        "O'Reilly & Co"
    };
    for (const auto& t : tests) {
        std::cout << render_hello(t) << "\n----\n";
    }
    return 0;
}