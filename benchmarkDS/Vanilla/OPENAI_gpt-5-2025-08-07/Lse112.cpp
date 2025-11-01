#include <iostream>
#include <string>
#include <cstdlib>

std::string ping(const std::string& url) {
#ifdef _WIN32
    std::string cmd = "ping -n 1 " + url;
#else
    std::string cmd = "ping -c 1 " + url;
#endif
    std::system(cmd.c_str());
    return url;
}

int main() {
    std::string tests[5] = {"localhost", "example.com", "127.0.0.1", "8.8.8.8", "github.com"};
    for (const auto& t : tests) {
        std::cout << "Returned URL: " << ping(t) << std::endl;
    }
    return 0;
}