#include <iostream>
#include <sstream>
#include <string>
#include <cctype>
#include <algorithm>

static constexpr std::size_t MAX_INPUT_BYTES = 4096;
static constexpr std::size_t MAX_USERNAME_LEN = 64;

static std::string sanitize_username(const std::string& s) {
    std::string out;
    out.reserve(std::min<std::size_t>(s.size(), MAX_USERNAME_LEN));
    for (char c : s) {
        if (out.size() >= MAX_USERNAME_LEN) break;
        // Allow printable ASCII safe set
        unsigned char uc = static_cast<unsigned char>(c);
        if (uc >= 0x20 && uc <= 0x7E) {
            if (std::isalnum(uc) || c == ' ' || c == '_' || c == '-' || c == '.') {
                out.push_back(static_cast<char>(uc));
            }
        }
    }
    // trim spaces
    auto start = out.find_first_not_of(' ');
    auto end = out.find_last_not_of(' ');
    if (start == std::string::npos) return std::string();
    return out.substr(start, end - start + 1);
}

void http_hello_page(std::istream& request, std::ostream& response) {
    char buf[MAX_INPUT_BYTES + 1];
    request.read(buf, static_cast<std::streamsize>(MAX_INPUT_BYTES));
    std::streamsize n = request.gcount();
    if (n < 0) n = 0;
    buf[n] = '\0';
    std::string input(buf, static_cast<std::size_t>(n));

    // take first line only
    std::size_t pos_r = input.find('\r');
    std::size_t pos_n = input.find('\n');
    std::size_t cut = std::min(pos_r == std::string::npos ? input.size() : pos_r,
                               pos_n == std::string::npos ? input.size() : pos_n);
    std::string first_line = input.substr(0, cut);

    std::string username = sanitize_username(first_line);
    if (username.empty()) username = "user";
    response << "hello " << username;
    response.flush();
}

static void run_test(const std::string& s) {
    std::istringstream req(s);
    std::ostringstream resp;
    http_hello_page(req, resp);
    std::cout << resp.str() << "\n";
}

int main() {
    // Test 1
    run_test("alice");
    // Test 2
    run_test("Bob-123");
    // Test 3: injection attempt
    run_test("eve\r\nSet-Cookie: x=1");
    // Test 4: long input
    run_test(std::string(200, 'x'));
    // Test 5: special chars and unicode (will be filtered for non-ASCII)
    run_test("张伟/..\\;|$");
    return 0;
}