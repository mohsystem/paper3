// Note: This code requires two third-party header-only libraries:
// 1. cpp-httplib: https://github.com/yhirose/cpp-httplib
//    - Download httplib.h and place it in your include path.
// 2. nlohmann/json: https://github.com/nlohmann/json
//    - Download json.hpp and place it in your include path.
//
// Compilation command (on Linux/macOS with OpenSSL installed):
// g++ -std=c++17 your_file_name.cpp -o Task64 -lssl -lcrypto -pthread

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include "json.hpp"
#include <iostream>
#include <string>
#include <stdexcept>
#include <memory>

// for convenience
using json = nlohmann::json;

/**
 * Fetches content from a URL, parses it as JSON, and returns the object.
 * @param url_string The full URL (e.g., "https://example.com/data.json").
 * @return A nlohmann::json object.
 * @throws std::runtime_error if fetching or parsing fails.
 */
json readJsonFromUrl(const std::string& url_string) {
    // 1. Parse the URL
    const auto url = httplib::detail::parse_url(url_string);
    if (!url.is_valid) {
        throw std::runtime_error("Invalid URL: " + url_string);
    }
    const std::string& host = url.host;
    const std::string& path = url.path.empty() ? "/" : url.path;

    // 2. Create the appropriate client (HTTP or HTTPS)
    std::unique_ptr<httplib::Client> cli;
    if (url.scheme == "https") {
        cli = std::make_unique<httplib::SSLClient>(host, url.port);
    } else if (url.scheme == "http") {
        cli = std::make_unique<httplib::Client>(host, url.port);
    } else {
        throw std::runtime_error("Unsupported protocol: " + url.scheme);
    }

    cli->set_follow_location(true); // Allow redirects
    cli->set_connection_timeout(5);

    // 3. Make the GET request
    auto res = cli->Get(path);

    // 4. Check the response and parse JSON
    if (res && res->status >= 200 && res->status < 300) {
        try {
            return json::parse(res->body);
        } catch (json::parse_error& e) {
            throw std::runtime_error("JSON parse error: " + std::string(e.what()));
        }
    } else {
        std::string error_msg = "HTTP request failed. ";
        if (res) {
            error_msg += "Status code: " + std::to_string(res->status);
        } else {
            auto err = res.error();
            error_msg += "Error: " + httplib::to_string(err);
        }
        throw std::runtime_error(error_msg);
    }
}

int main() {
    std::string testUrls[] = {
        "https://jsonplaceholder.typicode.com/todos/1",
        "https://jsonplaceholder.typicode.com/posts/1",
        "https://jsonplaceholder.typicode.com/users/1",
        "https://jsonplaceholder.typicode.com/posts", // This is a JSON array
        "https://jsonplaceholder.typicode.com/nonexistent" // This will cause a 404 error
    };

    for (const auto& url : testUrls) {
        std::cout << "Fetching from: " << url << std::endl;
        try {
            json jsonData = readJsonFromUrl(url);
            // Pretty print with an indent of 4 spaces
            std::cout << jsonData.dump(4) << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
        std::cout << "----------------------------------------" << std::endl;
    }

    return 0;
}