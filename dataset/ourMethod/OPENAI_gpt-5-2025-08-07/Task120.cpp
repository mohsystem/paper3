#include <iostream>
#include <string>
#include <vector>

static std::string sanitizeAndEscape(const std::string& input, std::size_t maxLen = 200) {
    std::string out;
    out.reserve(input.size() * 2);
    std::size_t included = 0;

    for (std::size_t i = 0; i < input.size(); ++i) {
        if (included >= maxLen) {
            break;
        }
        unsigned char b = static_cast<unsigned char>(input[i]);
        bool isAscii = b < 0x80;
        bool allowed = false;

        if (isAscii) {
            if (b >= 0x20 || b == '\t' || b == '\n' || b == '\r') {
                allowed = true;
            }
        } else {
            // Non-ASCII bytes (part of UTF-8) are passed through unchanged
            allowed = true;
        }

        if (!allowed) {
            continue;
        }

        if (isAscii) {
            switch (b) {
                case '&': out += "&amp;"; break;
                case '<': out += "&lt;"; break;
                case '>': out += "&gt;"; break;
                case '"': out += "&quot;"; break;
                case '\'': out += "&#x27;"; break;
                default: out.push_back(static_cast<char>(b)); break;
            }
        } else {
            out.push_back(static_cast<char>(b));
        }
        included++;
    }
    return out;
}

static std::string renderPage(const std::string& userInput) {
    const std::string safe = sanitizeAndEscape(userInput, 200);
    std::string html;
    html.reserve(512 + safe.size());
    html += "<!DOCTYPE html>";
    html += "<html lang=\"en\">";
    html += "<head>";
    html += "<meta charset=\"UTF-8\"/>";
    html += "<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\"/>";
    html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"/>";
    html += "<title>Safe Echo</title>";
    html += "<style>";
    html += "body{font-family:system-ui,-apple-system,Segoe UI,Roboto,Ubuntu,Cantarell,Noto Sans,sans-serif;line-height:1.5;padding:20px;background:#f8f9fa;color:#212529;}";
    html += "pre{background:#fff;border:1px solid #dee2e6;border-radius:8px;padding:12px;white-space:pre-wrap;word-wrap:break-word;}";
    html += ".note{color:#6c757d;font-size:0.9em;margin-top:8px;}";
    html += "</style>";
    html += "</head>";
    html += "<body>";
    html += "<h1>Safe Echo</h1>";
    html += "<p>The content below is sanitized and length-limited to 200 characters.</p>";
    html += "<pre>";
    html += safe;
    html += "</pre>";
    html += "<p class=\"note\">Try different inputs by modifying the program arguments.</p>";
    html += "</body>";
    html += "</html>";
    return html;
}

int main(int argc, char* argv[]) {
    // Optional: If an argument is provided, render that page first
    if (argc > 1) {
        std::cout << renderPage(argv[1]) << std::endl;
    }

    // 5 test cases
    std::vector<std::string> tests = {
        "Hello, world!",
        "<script>alert('xss')</script>",
        std::string(300, 'A'),
        "Line1\nLine2 & Line3 <test>",
        "Emoji: 😊 and quotes \" ' & other <tags>"
    };
    for (const auto& t : tests) {
        std::cout << renderPage(t) << std::endl;
    }
    return 0;
}