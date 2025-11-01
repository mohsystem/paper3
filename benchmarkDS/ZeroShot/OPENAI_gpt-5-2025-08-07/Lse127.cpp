#include <iostream>
#include <sstream>
#include <string>
#include <cctype>
#include <algorithm>

static const size_t MAX_READ_BYTES = 2048;
static const size_t MAX_NAME_CHARS = 100;

static std::string sanitizeUsername(const std::string& input) {
    // Remove CR/LF and trim
    std::string s;
    s.reserve(input.size());
    for (char c : input) {
        if (c != '\r' && c != '\n') s.push_back(c);
    }
    // Trim spaces
    auto l = s.find_first_not_of(' ');
    auto r = s.find_last_not_of(' ');
    if (l == std::string::npos) s.clear();
    else s = s.substr(l, r - l + 1);

    std::string out;
    out.reserve(std::min(MAX_NAME_CHARS, s.size()));
    for (char c : s) {
        unsigned char uc = static_cast<unsigned char>(c);
        if (std::isalnum(uc) || c == ' ' || c == '_' || c == '-' || c == '.' || c == '\'') {
            if (out.size() < MAX_NAME_CHARS) out.push_back(c);
            else break;
        }
    }
    // Trim again
    l = out.find_first_not_of(' ');
    r = out.find_last_not_of(' ');
    if (l == std::string::npos) out.clear();
    else out = out.substr(l, r - l + 1);

    if (out.empty()) return "Guest";
    return out;
}

std::string http_hello_page(std::istream& request, std::ostream& response) {
    std::string raw;
    raw.reserve(MAX_READ_BYTES);
    char buf[256];
    size_t remaining = MAX_READ_BYTES;
    while (remaining > 0 && request.good()) {
        size_t toRead = std::min(remaining, sizeof(buf));
        request.read(buf, static_cast<std::streamsize>(toRead));
        std::streamsize got = request.gcount();
        if (got <= 0) break;
        raw.append(buf, static_cast<size_t>(got));
        remaining -= static_cast<size_t>(got);
    }

    std::string username = sanitizeUsername(raw);
    std::string message = "Hello " + username;

    response.write(message.c_str(), static_cast<std::streamsize>(message.size()));
    response.flush();

    return message;
}

// 5 test cases
int main() {
    auto runTest = [](const std::string& input) {
        std::istringstream req(input);
        std::ostringstream res;
        std::string msg = http_hello_page(req, res);
        std::cout << "Returned: " << msg << "\n";
        std::cout << "Written : " << res.str() << "\n";
        std::cout << "---\n";
    };

    runTest("Alice");
    runTest("  Bob  ");
    runTest("Eve\r\nSet-Cookie: hack");
    runTest("Zeljko"); // ASCII fallback for unicode example
    runTest(std::string(500, 'A'));

    return 0;
}