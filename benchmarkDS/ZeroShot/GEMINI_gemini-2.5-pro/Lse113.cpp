#include <iostream>
#include <string>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include <vector>
#include <regex>

#ifndef _WIN32
#include <sys/wait.h>
#endif

// Security: Validate hostname to contain only allowed characters.
// This is crucial to prevent command injection when using popen, which invokes a shell.
bool isValidHostname(const std::string& url) {
    if (url.empty()) {
        return false;
    }
    // Regex for valid hostname characters (alphanumeric, dot, hyphen)
    static const std::regex pattern("^[a-zA-Z0-9.-]+$");
    return std::regex_match(url, pattern);
}

struct PingResult {
    std::string status;
    std::string pong;
};

PingResult pingUrl(const std::string& url) {
    // Security: Abort if the URL is not in a valid format.
    if (!isValidHostname(url)) {
        return {"URL '" + url + "' is invalid.", "Error: Invalid characters in URL."};
    }

    // Platform-specific ping command
    #ifdef _WIN32
        std::string command = "ping -n 4 " + url;
    #else
        std::string command = "ping -c 4 " + url;
    #endif

    std::string pong;
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        return {"Error executing ping command.", "Failed to open pipe."};
    }
    
    try {
        std::array<char, 128> buffer;
        while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
            pong += buffer.data();
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    
    int return_code = pclose(pipe);

    #ifndef _WIN32
        if (return_code != -1) {
            return_code = WEXITSTATUS(return_code);
        }
    #endif

    if (return_code == 0) {
        return {"The URL '" + url + "' is alive.", pong};
    } else {
        return {"The URL '" + url + "' is not alive.", pong};
    }
}

int main() {
    std::vector<std::string> testUrls = {
        "8.8.8.8",
        "invalid-hostname-that-does-not-exist-12345.com",
        "127.0.0.1",
        "google.com",
        "google.com; ls -la" // Malicious input
    };

    for (const auto& url : testUrls) {
        PingResult result = pingUrl(url);
        std::cout << "--- Testing URL: " << url << " ---" << std::endl;
        std::cout << "Status: " << result.status << std::endl;
        std::cout << "Output (pong):" << std::endl;
        std::cout << result.pong << std::endl;
        std::cout << "---------------------------" << std::endl;
    }

    return 0;
}