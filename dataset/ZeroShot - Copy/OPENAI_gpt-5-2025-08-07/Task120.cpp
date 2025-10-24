#include <iostream>
#include <string>

class Task120CPP {
public:
    static std::string escapeHTML(const std::string& s) {
        std::string out;
        out.reserve(s.size());
        for (char c : s) {
            switch (c) {
                case '&': out += "&amp;"; break;
                case '<': out += "&lt;"; break;
                case '>': out += "&gt;"; break;
                case '"': out += "&quot;"; break;
                case '\'': out += "&#x27;"; break;
                case '/': out += "&#x2F;"; break;
                default: out += c; break;
            }
        }
        return out;
    }

    static std::string displayUserInput(const std::string& input) {
        std::string safe = escapeHTML(input);
        std::string html;
        html.reserve(safe.size() + 200);
        html += "<!DOCTYPE html>\n";
        html += "<html lang=\"en\">\n<head>\n";
        html += "<meta charset=\"UTF-8\">\n";
        html += "<meta http-equiv=\"Content-Security-Policy\" content=\"default-src 'none';\">\n";
        html += "<title>Safe Echo</title>\n";
        html += "</head>\n<body>\n";
        html += "<h1>Echo</h1>\n";
        html += "<pre>";
        html += safe;
        html += "</pre>\n";
        html += "</body>\n</html>\n";
        return html;
    }
};

int main() {
    std::string tests[5] = {
        "Hello, world!",
        "<script>alert('XSS');</script>",
        "Tom & Jerry > Mickey & Minnie",
        "\"Quotes\" and 'single quotes' and /slashes/",
        "Unicode: Привет, 你好, مرحبا, 😀"
    };
    for (int i = 0; i < 5; ++i) {
        std::cout << "----- Test " << (i + 1) << " -----\n";
        std::cout << Task120CPP::displayUserInput(tests[i]) << "\n";
    }
    return 0;
}