#include <iostream>
#include <sstream>
#include <string>
#include <cctype>

static inline std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) start++;
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) end--;
    return s.substr(start, end - start);
}

std::string http_hello_page(std::istream& request, std::ostream& response) {
    std::ostringstream buf;
    buf << request.rdbuf();
    std::string username = trim(buf.str());
    std::string message = "hello " + username;
    response << message;
    return message;
}

int main() {
    // Test case 1
    std::istringstream req1("Alice");
    std::ostringstream resp1;
    std::string out1 = http_hello_page(req1, resp1);
    std::cout << resp1.str() << std::endl;

    // Test case 2
    std::istringstream req2("Bob");
    std::ostringstream resp2;
    std::string out2 = http_hello_page(req2, resp2);
    std::cout << resp2.str() << std::endl;

    // Test case 3
    std::istringstream req3("   Charlie   ");
    std::ostringstream resp3;
    std::string out3 = http_hello_page(req3, resp3);
    std::cout << resp3.str() << std::endl;

    // Test case 4 (empty)
    std::istringstream req4("");
    std::ostringstream resp4;
    std::string out4 = http_hello_page(req4, resp4);
    std::cout << resp4.str() << std::endl;

    // Test case 5 (with newline)
    std::istringstream req5("Dana\n");
    std::ostringstream resp5;
    std::string out5 = http_hello_page(req5, resp5);
    std::cout << resp5.str() << std::endl;

    return 0;
}