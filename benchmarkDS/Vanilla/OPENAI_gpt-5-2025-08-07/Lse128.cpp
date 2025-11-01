#include <iostream>
#include <sstream>
#include <string>
#include <cctype>

std::string trim_copy(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;
    return s.substr(start, end - start);
}

std::string http_hello_page(std::istream& request, std::ostream& response) {
    std::ostringstream buf;
    buf << request.rdbuf();
    std::string username = trim_copy(buf.str());
    std::string message = "hello " + username;
    response << message;
    response.flush();
    return message;
}

int main() {
    std::string names[5] = {"", "Alice", "Bob", "  Charlie  ", "Dana_Smith"};
    for (const auto& name : names) {
        std::istringstream req(name);
        std::ostringstream resp;
        std::string out = http_hello_page(req, resp);
        std::cout << out << std::endl;
    }
    return 0;
}