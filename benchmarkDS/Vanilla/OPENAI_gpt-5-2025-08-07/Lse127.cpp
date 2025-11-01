#include <iostream>
#include <sstream>
#include <string>
#include <cctype>

static std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) start++;
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) end--;
    return s.substr(start, end - start);
}

std::string http_hello_page(std::istream& request, std::ostream& response) {
    std::ostringstream buffer;
    buffer << request.rdbuf();
    std::string username = trim(buffer.str());
    std::string message = "Hello " + username;
    response << message;
    return message;
}

int main() {
    std::string usernames[5] = {"Alice", "Bob", "Charlie", "", "Eve Adams"};
    for (const auto& name : usernames) {
        std::istringstream req(name);
        std::ostringstream resp;
        std::string result = http_hello_page(req, resp);
        std::cout << "Returned: " << result << "\n";
        std::cout << "Response Stream: " << resp.str() << "\n";
    }
    return 0;
}