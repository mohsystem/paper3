#include <iostream>
#include <string>
#include <unordered_map>
#include <functional>

// Simple "web application" class
class WebApp {
public:
    // Register a route to a handler
    void route(const std::string& path, std::function<std::string(const std::string&)> handler) {
        routes_[path] = handler;
    }

    // Dispatch a path to the appropriate handler
    std::string dispatch(const std::string& path) const {
        auto it = routes_.find(path);
        if (it != routes_.end()) {
            return it->second(path);
        }
        return "404 Not Found";
    }

    // Run the application (simulated)
    void run() const {
        std::cout << "C++ WebApp running (simulated)" << std::endl;
    }

private:
    std::unordered_map<std::string, std::function<std::string(const std::string&)>> routes_;
};

// Function that accepts input and returns output
std::string rootHandler(const std::string& path) {
    if (path == "/") {
        return "Hello from C++ WebApp";
    }
    return "404 Not Found";
}

int main() {
    WebApp app;                               // Create application object
    app.route("/", rootHandler);              // Map URL to handler

    // 5 test cases
    std::string tests[5] = {"/", "/notfound", "/", "/x", "/"};
    for (const auto& t : tests) {
        std::cout << "GET " << t << " -> " << app.dispatch(t) << std::endl;
    }

    app.run();                                // Run application (simulated)
    return 0;
}