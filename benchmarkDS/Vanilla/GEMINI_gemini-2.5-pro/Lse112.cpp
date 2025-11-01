// This code requires the cpp-httplib library.
// Download httplib.h from https://github.com/yhirose/cpp-httplib
// and place it in the same directory as this file or in your include path.
// Then compile with: g++ -o your_program_name your_source_file.cpp -std=c++17 -pthread

#include "httplib.h"
#include <iostream>
#include <string>
#include <memory>
#include <stdexcept>
#include <array>
#include <regex>
#include <vector>

/**
 * Executes the system's ping command for a given URL.
 * @param url The URL or IP address to ping.
 * @return The output of the ping command as a std::string.
 */
std::string executePing(const std::string& url) {
    // Basic validation to prevent command injection
    if (url.empty() || !std::regex_match(url, std::regex("^[a-zA-Z0-9.\\-]+$"))) {
        return "Error: Invalid or potentially malicious URL format provided.";
    }

    std::string command;
#ifdef _WIN32
    // Command for Windows
    command = "ping -n 4 " + url;
#else
    // Command for Linux, macOS, and other Unix-like systems
    command = "ping -c 4 " + url;
#endif

    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);

    if (!pipe) {
        return "popen() failed!";
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return result;
}

/**
 * Runs 5 test cases directly from the command line.
 */
void runTestCases() {
    std::cout << "--- Running 5 Command-Line Test Cases ---" << std::endl;
    std::vector<std::string> testUrls = {
        "google.com",               // Valid domain
        "127.0.0.1",                // Localhost IP
        "8.8.8.8",                  // Public IP
        "nonexistent-domain-1234.com", // Invalid domain
        "google.com; ls -la"        // Malicious input attempt
    };

    for (const auto& url : testUrls) {
        std::cout << "\n--- Pinging: " << url << " ---" << std::endl;
        std::string result = executePing(url);
        std::cout << result << std::endl;
    }
}

int main() {
    // 1. Run 5 test cases and print results to console
    runTestCases();

    // 2. Start the web server
    httplib::Server svr;

    // The decorator equivalent: binding the /ping path to a lambda function
    svr.Get("/ping", [](const httplib::Request& req, httplib::Response& res) {
        std::string url_to_ping;
        if (req.has_param("url")) {
            url_to_ping = req.get_param_value("url");
        }
        
        std::string response_body;
        if (url_to_ping.empty()) {
            response_body = "Please provide a URL to ping. Example: /ping?url=google.com";
        } else {
            response_body = executePing(url_to_ping);
        }
        
        res.set_content(response_body, "text/plain");
    });
    
    int port = 8080;
    std::cout << "\n--- Starting C++ Web Server on port " << port << " ---" << std::endl;
    std::cout << "To test the server, open a browser or use curl with the following URLs:" << std::endl;
    std::cout << "  curl 'http://localhost:" << port << "/ping?url=google.com'" << std::endl;
    std::cout << "  curl 'http://localhost:" << port << "/ping?url=1.1.1.1'" << std::endl;
    std::cout << "Server started. Press Ctrl+C to stop." << std::endl;
    
    svr.listen("0.0.0.0", port);

    return 0;
}