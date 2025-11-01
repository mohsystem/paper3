#include <iostream>
#include <sstream>
#include <string>
#include <cctype>

class Lse126 {
public:
    static std::string http_hello_page(const std::string& requestBody) {
        std::string username = sanitizeUsername(requestBody);
        std::string safeName = escapeHtml(username);

        std::ostringstream builder;
        builder << "<!doctype html>\n"
                << "<html lang=\"en\">\n"
                << "<head>\n"
                << "<meta charset=\"utf-8\">\n"
                << "<title>Hello</title>\n"
                << "</head>\n"
                << "<body>\n"
                << "<h1>Hello, " << safeName << "!</h1>\n"
                << "</body>\n"
                << "</html>\n";

        std::string outputbuf = builder.str();

        std::ostringstream responseStream;
        responseStream << outputbuf;
        responseStream.flush();
        // Simulate closing by destroying the stream object at end of scope

        return responseStream.str();
    }

private:
    static std::string sanitizeUsername(const std::string& input) {
        std::string u = input;
        if (u.empty()) {
            u = "Guest";
        }
        if (u.size() > 256) {
            u = u.substr(0, 256);
        }
        std::string cleaned;
        cleaned.reserve(u.size());
        for (unsigned char ch : u) {
            if (ch >= 0x20 && ch != 0x7F) {
                cleaned.push_back(static_cast<char>(ch));
            }
        }
        return cleaned;
    }

    static std::string escapeHtml(const std::string& s) {
        std::string out;
        out.reserve(s.size());
        for (char c : s) {
            switch (c) {
                case '&': out.append("&amp;"); break;
                case '<': out.append("&lt;"); break;
                case '>': out.append("&gt;"); break;
                case '"': out.append("&quot;"); break;
                case '\'': out.append("&#x27;"); break;
                default: out.push_back(c); break;
            }
        }
        return out;
    }
};

int main() {
    std::string tests[5] = {
        "Alice",
        "<script>alert(1)</script>",
        std::string(500, 'X'),
        "",
        std::string("Eve\t\r\n\x7F")
    };
    for (int i = 0; i < 5; ++i) {
        std::string res = Lse126::http_hello_page(tests[i]);
        std::cout << "Test " << (i + 1) << ":\n" << res << "\n\n";
    }
    return 0;
}