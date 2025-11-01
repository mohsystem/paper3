#include <iostream>
#include <sstream>
#include <string>
#include <cctype>
#include <limits>

static const std::size_t MAX_INPUT_BYTES = 1024;
static const std::size_t MAX_USERNAME_CHARS = 100;

static bool isAllowedChar(unsigned char c) {
    if (c >= 'a' && c <= 'z') return true;
    if (c >= 'A' && c <= 'Z') return true;
    if (c >= '0' && c <= '9') return true;
    if (c == ' ' || c == '_' || c == '-' || c == '.' || c == '@') return true;
    return false;
}

static std::string sanitizeUsername(const std::string& s) {
    std::string out;
    out.reserve(std::min<std::size_t>(s.size(), MAX_USERNAME_CHARS));
    for (char ch : s) {
        if (ch == '\r' || ch == '\n') continue;
        unsigned char c = static_cast<unsigned char>(ch);
        if (isAllowedChar(c)) {
            out.push_back(static_cast<char>(c));
            if (out.size() >= MAX_USERNAME_CHARS) break;
        }
    }
    // trim
    std::size_t start = 0;
    while (start < out.size() && out[start] == ' ') start++;
    std::size_t end = out.size();
    while (end > start && out[end - 1] == ' ') end--;
    std::string trimmed = out.substr(start, end - start);
    if (trimmed.empty()) return "anonymous";
    return trimmed;
}

std::string http_hello_page(std::istream& request, std::ostream& response) {
    char buffer[256];
    std::string input;
    input.reserve(MAX_INPUT_BYTES);
    while (request && input.size() < MAX_INPUT_BYTES) {
        request.read(buffer, std::min<std::size_t>(sizeof(buffer), MAX_INPUT_BYTES - input.size()));
        std::streamsize got = request.gcount();
        if (got > 0) input.append(buffer, static_cast<std::size_t>(got));
        if (got == 0) break;
    }

    std::string username = sanitizeUsername(input);
    std::string output = "hello " + username;

    response.write(output.data(), static_cast<std::streamsize>(output.size()));
    response.flush();

    return output;
}

// 5 test cases
int main() {
    const char* tests[] = {
        "Alice",
        "Bob<script>alert(1)</script>",
        "Carol\r\nSet-Cookie: session=evil",
        u8"Δelta-用户",
        "Dave ../../etc/passwd\t\n"
    };

    for (int i = 0; i < 5; ++i) {
        std::istringstream req(tests[i]);
        std::ostringstream resp;
        std::string result = http_hello_page(req, resp);
        std::cout << "Test " << (i + 1) << ": " << result << " | ResponseStream: " << resp.str() << "\n";
    }
    return 0;
}