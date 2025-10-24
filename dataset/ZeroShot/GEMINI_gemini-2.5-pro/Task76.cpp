// INSTRUCTIONS:
// This code requires the 'cpp-httplib' single-header library and OpenSSL.
// 1. Download 'httplib.h' from: https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h
// 2. Place 'httplib.h' in the same directory as this source file or in your include path.
// 3. Install OpenSSL development libraries (e.g., 'sudo apt-get install libssl-dev' on Debian/Ubuntu).
// 4. Compile with: g++ -std=c++17 Task76.cpp -o task76 -lssl -lcrypto -lpthread

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

/**
 * Helper function to find a substring case-insensitively.
 * @param str The string to search in.
 * @param sub The substring to search for.
 * @param pos The position to start the search from.
 * @return The starting position of the substring, or std::string::npos if not found.
 */
size_t find_case_insensitive(const std::string& str, const std::string& sub, size_t pos = 0) {
    auto it = std::search(
        str.begin() + pos, str.end(),
        sub.begin(), sub.end(),
        [](char ch1, char ch2) { return std::toupper(static_cast<unsigned char>(ch1)) == std::toupper(static_cast<unsigned char>(ch2)); }
    );
    return it == str.end() ? std::string::npos : std::distance(str.begin(), it);
}

/**
 * Fetches an HTTPS URL and extracts the content of the <title> tag.
 * @param url_string The HTTPS URL to fetch.
 * @return The extracted page title, or an error message.
 */
std::string extractPageTitle(const std::string& url_string) {
    // 1. Input Validation
    if (url_string.rfind("https://", 0) != 0) {
        return "Error: Invalid or insecure URL provided. Please use HTTPS.";
    }

    try {
        // 2. Parse URL into host and path for cpp-httplib
        size_t path_start = url_string.find('/', 8); // 8 is the length of "https://"
        std::string host_part;
        std::string path_part;

        if (path_start == std::string::npos) {
            host_part = url_string;
            path_part = "/";
        } else {
            host_part = url_string.substr(0, path_start);
            path_part = url_string.substr(path_start);
        }

        // 3. Setup Client and make request. SSL is used automatically for "https://".
        httplib::Client cli(host_part);
        cli.set_connection_timeout(10, 0); // 10s timeout
        cli.set_read_timeout(10, 0);       // 10s timeout
        cli.set_follow_location(true);     // Follow redirects

        auto res = cli.Get(path_part.c_str());

        // 4. Handle connection or request errors
        if (!res) {
            auto err = res.error();
            if (err == httplib::Error::Connection || err == httplib::Error::Unknown) {
                 return "Error: Connection or DNS resolution failed for " + host_part;
            }
            return "Error: Request failed - " + httplib::to_string(err);
        }

        // 5. Handle HTTP status code errors
        if (res->status != 200) {
            return "Error: Received non-200 status code: " + std::to_string(res->status);
        }

        const std::string& body = res->body;

        // 6. Extract title using safe string searching
        size_t title_start_tag_pos = find_case_insensitive(body, "<title>");
        if (title_start_tag_pos == std::string::npos) {
            return "Error: Title tag not found.";
        }
        size_t title_start_pos = title_start_tag_pos + std::string("<title>").length();

        size_t title_end_tag_pos = find_case_insensitive(body, "</title>", title_start_pos);
        if (title_end_tag_pos == std::string::npos) {
            return "Error: Closing title tag not found.";
        }

        std::string title = body.substr(title_start_pos, title_end_tag_pos - title_start_pos);
        
        // Trim leading/trailing whitespace
        const char* whitespace = " \t\n\r\f\v";
        title.erase(0, title.find_first_not_of(whitespace));
        title.erase(title.find_last_not_of(whitespace) + 1);

        return title;

    } catch (const std::exception& e) {
        return std::string("Error: An exception occurred - ") + e.what();
    }
}

int main() {
    std::vector<std::string> testUrls = {
        "https://www.google.com",
        "https://github.com",
        "https://github.com/nonexistent/repo",  // 404 test
        "https://thissitedoesnotexist12345.com",  // DNS error
        "http://www.google.com"  // Insecure URL test
    };

    for (const auto& url : testUrls) {
        std::cout << "Testing URL: " << url << std::endl;
        std::string title = extractPageTitle(url);
        std::cout << "Page Title: " << title << std::endl;
        std::cout << "--------------------" << std::endl;
    }

    return 0;
}