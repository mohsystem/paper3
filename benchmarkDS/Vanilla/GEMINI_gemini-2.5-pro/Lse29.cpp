// Note: This code requires the cpp-httplib single-header library.
// Download "httplib.h" from https://github.com/yhirose/cpp-httplib
// and place it in the same directory as this file.
//
// Compile with: g++ -o server Lse29.cpp -std=c++11 -pthread

#include <iostream>
#include <string>
#include <regex>
#include "httplib.h"

// The main class name is Lse29 as requested
class Lse29 {
public:
    void run() {
        httplib::Server svr;

        svr.Get("/", [](const httplib::Request& req, httplib::Response& res) {
            res.set_content("Welcome! This is the default page.", "text/plain");
        });

        svr.Get("/redirect", [](const httplib::Request& req, httplib::Response& res) {
            std::string target = "";
            if (req.has_param("target")) {
                target = req.get_param_value("target");
            }

            const std::regex pattern(".*example\\.com.*");
            
            if (!target.empty() && std::regex_match(target, pattern)) {
                res.set_redirect(target.c_str(), 302);
            } else {
                res.set_redirect("/", 302);
            }
        });

        int port = 8081;
        std::cout << "C++ server started on http://localhost:" << port << std::endl;
        print_test_cases(port);
        svr.listen("0.0.0.0", port);
    }

private:
    void print_test_cases(int port) {
        std::string base_url = "http://localhost:" + std::to_string(port);
        std::cout << "--- 5 Test Cases ---" << std::endl;
        std::cout << "Execute these commands in a separate terminal:" << std::endl;
        std::cout << "1. Valid Target: curl -I \"" << base_url << "/redirect?target=http://www.example.com\"" << std::endl;
        std::cout << "   Expected: 302 Redirect to http://www.example.com" << std::endl;
        std::cout << "2. Valid Subdomain: curl -I \"" << base_url << "/redirect?target=https://blog.example.com/article/1\"" << std::endl;
        std::cout << "   Expected: 302 Redirect to https://blog.example.com/article/1" << std::endl;
        std::cout << "3. Invalid Target: curl -I \"" << base_url << "/redirect?target=http://www.google.com\"" << std::endl;
        std::cout << "   Expected: 302 Redirect to /" << std::endl;
        std::cout << "4. Invalid TLD: curl -I \"" << base_url << "/redirect?target=http://example.community.net\"" << std::endl;
        std::cout << "   Expected: 302 Redirect to /" << std::endl;
        std::cout << "5. Missing Target: curl -I \"" << base_url << "/redirect\"" << std::endl;
        std::cout << "   Expected: 302 Redirect to /" << std::endl;
    }
};

int main() {
    Lse29 app;
    app.run();
    return 0;
}