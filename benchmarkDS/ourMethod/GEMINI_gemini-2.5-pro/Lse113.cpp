#include <iostream>
#include <string>
#include <cstdio>
#include <array>
#include <stdexcept>
#include <regex>

#ifndef _WIN32
#include <sys/wait.h>
#endif

// Rule #1: Validate input to prevent OS command injection.
// A simple regex for a hostname. It allows alphanumeric chars, dots, and hyphens.
// This whitelist approach prevents shell metacharacters like ';', '&', '|', etc.
bool isValidHostname(const std::string& host) {
    if (host.empty()) {
        return false;
    }
    const std::regex pattern("^[a-zA-Z0-9.-]+$");
    return std::regex_match(host, pattern);
}

// The function ping() is called when the URL <url> ping is requested.
void ping(const std::string& url) {
    std::cout << "--- Pinging " << url << " ---" << std::endl;

    if (!isValidHostname(url)) {
        std::cerr << "Invalid or potentially malicious URL/hostname format: " << url << std::endl;
        std::cout << "Result: URL is not alive." << std::endl;
        std::cout << "------------------------------" << std::endl;
        return;
    }

    std::string command;
#ifdef _WIN32
    command = "ping -n 1 -w 1000 " + url;
#else
    command = "ping -c 1 -W 1 " + url;
#endif

    std::string pong; // To store the command output
    FILE* pipe = nullptr;

    // Rule #2: Use potentially dangerous functions with caution.
    // popen() invokes a shell, so input validation (performed above) is critical to its safe use.
#ifdef _WIN32
    pipe = _popen(command.c_str(), "r");
#else
    pipe = popen(command.c_str(), "r");
#endif

    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }

    // Rule #3: Ensure buffer operations are safe.
    std::array<char, 128> buffer;
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        pong += buffer.data();
    }
    
    int status;
#ifdef _WIN32
    status = _pclose(pipe);
#else
    status = pclose(pipe);
#endif

    std::cout << "Command output (pong):" << std::endl;
    std::cout << pong;

    int exit_code = -1;
    if (status == -1) {
        // pclose failed
        exit_code = -1;
    } else {
#ifdef _WIN32
        exit_code = status;
#else
        if (WIFEXITED(status)) {
            exit_code = WEXITSTATUS(status);
        }
#endif
    }
    
    // If the ping command returns 0 the URL is alive.
    if (exit_code == 0) {
        std::cout << "Result: URL '" << url << "' is alive." << std::endl;
    } else {
        // Else the URL is not alive.
        std::cout << "Result: URL '" << url << "' is not alive (exit code: " << exit_code << ")." << std::endl;
    }
    std::cout << "------------------------------" << std::endl;
}

int main() {
    std::cout << "The prompt's decorator concept is simulated here by calling the `ping` function for each test URL." << std::endl;
    std::cout << std::endl;
    
    const std::array<std::string, 5> testUrls = {
        "8.8.8.8",                  // A known alive IP
        "google.com",               // A known alive hostname
        "invalid-hostname-that-does-not-exist.com", // A likely dead hostname
        "127.0.0.1",                // Localhost, should be alive
        "google.com; rm -rf /"      // Malicious input to test validation
    };

    for (const auto& url : testUrls) {
        try {
            ping(url);
        } catch (const std::exception& e) {
            std::cerr << "An error occurred while pinging " << url << ": " << e.what() << std::endl;
            std::cout << "------------------------------" << std::endl;
        }
    }

    return 0;
}