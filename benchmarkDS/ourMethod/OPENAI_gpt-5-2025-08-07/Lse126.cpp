#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <limits>

static constexpr std::size_t MAX_USERNAME = 256;

// HTML escape to prevent XSS
std::string escape_html(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 16);
    for (unsigned char ch : s) {
        switch (ch) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&#x27;"; break;
            default:
                // filter control chars except common whitespace
                if (ch >= 0x20 || ch == '\n' || ch == '\r' || ch == '\t') {
                    out.push_back(static_cast<char>(ch));
                } else {
                    out.push_back(' ');
                }
        }
    }
    return out;
}

// Constructs a safe HTML response for /hello using the given request body (username).
std::string http_hello_page(const std::string& request_body) {
    std::string username = request_body;
    // Trim leading/trailing whitespace
    auto lpos = username.find_first_not_of(" \t\r\n");
    auto rpos = username.find_last_not_of(" \t\r\n");
    if (lpos == std::string::npos) {
        username.clear();
    } else {
        username = username.substr(lpos, rpos - lpos + 1);
    }
    if (username.size() > MAX_USERNAME) {
        username.resize(MAX_USERNAME);
    }

    std::string safe = escape_html(username);
    std::ostringstream outputbuf;
    outputbuf << "<!doctype html>"
              << "<html><head><meta charset=\"UTF-8\"><title>Hello</title></head><body>"
              << "<h1>Hello, " << safe << "!</h1>"
              << "</body></html>";
    return outputbuf.str();
}

int main() {
    std::vector<std::string> tests = {
        "Alice",
        "",
        std::string(300, 'A'), // will be truncated to 256
        "<script>alert('x')</script>",
        "Élodie & 张伟"
    };

    for (std::size_t i = 0; i < tests.size(); ++i) {
        // Server reads request body (username)
        std::string username = tests[i];

        // Server constructs the response body (HTML)
        std::string outputbuf = http_hello_page(username);

        // Server writes the response body to the response stream
        std::ostringstream response_stream;
        response_stream << outputbuf;
        response_stream.flush();
        // "Close" the response stream by destroying it at end of scope

        // For demonstration: print the response body
        std::cout << "=== Test Case " << (i + 1) << " ===\n";
        std::cout << response_stream.str() << "\n\n";
    }
    return 0;
}