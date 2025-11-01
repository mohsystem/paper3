#include <iostream>
#include <string>
#include <cstdlib>

#ifdef _WIN32
#define PING_COUNT_FLAG "-n 1"
#define DEVNULL " >NUL 2>&1"
#else
#define PING_COUNT_FLAG "-c 1"
#define DEVNULL " >/dev/null 2>&1"
#endif

// Simulated decorator macro placeholder (no real decorators in C++)
#define ROUTE(path, func) /* bind route path to func (placeholder) */

std::string run_ping(const std::string& url) {
    std::string cmd = std::string("ping ") + PING_COUNT_FLAG + " " + url + DEVNULL;
    std::system(cmd.c_str());
    return url;
}

// Simulated route-bound function
ROUTE("/ping", ping)
std::string ping() {
    std::string url = "example.com";
    run_ping(url);
    return url; // would be returned to a browser in a real HTTP handler
}

int main() {
    // 5 test cases
    std::string tests[5] = {"example.com", "localhost", "127.0.0.1", "google.com", "invalid.invalid"};
    for (const auto& u : tests) {
        std::string result = run_ping(u);
        std::cout << "Pinged and returned URL: " << result << std::endl;
    }

    // Demonstrate the simulated route function behavior
    std::cout << "Route /ping returned URL: " << ping() << std::endl;
    return 0;
}