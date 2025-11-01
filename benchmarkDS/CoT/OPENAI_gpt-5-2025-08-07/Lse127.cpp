#include <iostream>
#include <sstream>
#include <string>
#include <vector>

static inline std::string trim(const std::string& s) {
    const std::string ws = " \t\n\r\f\v";
    size_t start = s.find_first_not_of(ws);
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(ws);
    return s.substr(start, end - start + 1);
}

std::string http_hello_page(std::istream& request, std::ostream& response) {
    std::ostringstream buf;
    buf << request.rdbuf();
    std::string username = trim(buf.str());
    std::string out = "Hello " + username;
    response << out;
    response.flush();
    return out;
}

int main() {
    std::vector<std::string> tests = {"Alice", "  Bob  ", "", "Bob123", "世界"};
    for (size_t i = 0; i < tests.size(); ++i) {
        std::istringstream req(tests[i]);
        std::ostringstream res;
        std::string ret = http_hello_page(req, res);
        std::cout << "Test " << (i + 1) << ": return=" << ret << ", response=" << res.str() << std::endl;
    }
    return 0;
}