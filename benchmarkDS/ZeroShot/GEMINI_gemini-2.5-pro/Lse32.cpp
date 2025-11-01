// Note: This code requires the single-header file `httplib.h` from the cpp-httplib project.
// You can get it here: https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h
// Place `httplib.h` in the same directory as this source file.
//
// Compile with: g++ -std=c++11 -pthread Lse32.cpp -o Lse32_server

#define CPPHTTPLIB_OPENSSL_SUPPORT // Uncomment for HTTPS support in client/server if needed

#include "httplib.h"
#include <iostream>
#include <regex>
#include <string>

class Lse32 {
public:
    static void redirect_invalid_url(const httplib::Request& req, httplib::Response& res) {
        if (!req.has_param("url")) {
            res.status = 400;
            res.set_content("URL parameter is missing.", "text/plain");
            return;
        }

        std::string url_str = req.get_param_value("url");
        if (url_str.empty()) {
            res.status = 400;
            res.set_content("URL parameter is empty.", "text/plain");
            return;
        }

        // Use a regular expression for robust extraction of the hostname.
        // This regex handles schemes (http/https), optional user info (user@), and ports.
        // It captures the hostname in group 1.
        // It's safer than manual string splitting.
        std::regex url_regex(R"(^(?:https?:\/\/)?(?:[^\/?#]+@)?([^\/?#:]+))", std::regex::icase);
        std::smatch match;

        if (std::regex_search(url_str, match, url_regex) && match.size() > 1) {
            std::string host = match[1].str();
            
            // Convert host to lower case for case-insensitive comparison
            std::transform(host.begin(), host.end(), host.begin(),
                [](unsigned char c){ return std::tolower(c); });

            if (host == "example.com") {
                // Valid redirect
                res.set_redirect(url_str.c_str(), 302);
            } else {
                // Invalid host
                res.status = 400;
                res.set_content("Invalid or untrusted URL. Redirects are only allowed to example.com.", "text/plain");
            }
        } else {
            // URL does not match the basic structure / malformed
            res.status = 400;
            res.set_content("Malformed URL provided.", "text/plain");
        }
    }

    void run() {
        httplib::Server svr;
        svr.Get("/redirect", redirect_invalid_url);

        std::cout << "Server is running on http://localhost:8088" << std::endl;
        svr.listen("0.0.0.0", 8088);
    }
};

int main() {
    std::cout << "Server starting..." << std::endl;
    std::cout << "Use a tool like curl to test the /redirect endpoint." << std::endl;
    std::cout << "\n--- 5 Test Cases ---" << std::endl;
    std::cout << "1. Valid URL (should redirect):" << std::endl;
    std::cout << "   curl -iL \"http://localhost:8088/redirect?url=http://example.com/some/path\"" << std::endl;
    std::cout << "\n2. Valid HTTPS URL (should redirect):" << std::endl;
    std::cout << "   curl -iL \"http://localhost:8088/redirect?url=https://example.com\"" << std::endl;
    std::cout << "\n3. Invalid Domain (should return 400 Bad Request):" << std::endl;
    std::cout << "   curl -i \"http://localhost:8088/redirect?url=http://google.com\"" << std::endl;
    std::cout << "\n4. Tricky Subdomain (should return 400 Bad Request):" << std::endl;
    std::cout << "   curl -i \"http://localhost:8088/redirect?url=http://example.com.evil.com\"" << std::endl;
    std::cout << "\n5. User info in URL (should redirect):" << std::endl;
    std::cout << "   curl -iL \"http://localhost:8088/redirect?url=http://user:pass@example.com/login\"" << std::endl;

    Lse32 server_app;
    server_app.run();

    return 0;
}