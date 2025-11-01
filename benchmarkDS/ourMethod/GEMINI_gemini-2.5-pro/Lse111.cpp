#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <stdexcept>
#include <cstdio>
#include <memory>
#include <array>

// Note: This C++ code is POSIX-specific (Linux, macOS) due to the use of popen.
// A cross-platform solution using fork/exec or CreateProcess (Windows) is more complex.
// `popen` is used here for simplicity but is less secure than fork/exec because it
// involves a shell. We mitigate this by strictly validating the input.

// Regex for validating a hostname or IP address.
// This is the primary defense against command injection.
bool isValidHostnameOrIp(const std::string& host) {
    // A simplified regex to allow alphanumeric characters, dots, and hyphens.
    // This effectively blocks shell metacharacters.
    static const std::regex pattern("^[a-zA-Z0-9\\.\\-]{1,253}$");
    return std::regex_match(host, pattern);
}

/**
 * Executes the ping command for a given URL/hostname.
 *
 * @param url The URL or IP address to ping.
 * @return The output of the ping command as a string.
 */
std::string ping(const std::string& url) {
    // Rule #1: Input Validation
    if (!isValidHostnameOrIp(url)) {
        return "Error: Invalid or potentially malicious hostname/IP provided.";
    }

    std::string command_str;
#ifdef _WIN32
    command_str = "ping -n 4 " + url;
#else
    command_str = "ping -c 4 " + url;
#endif

    // Rule #2: Use dangerous functions with caution. popen invokes a shell,
    // so input validation is absolutely critical. A safer alternative on POSIX
    // would be fork() + execvp(), which avoids the shell entirely.
    std::array<char, 128> buffer;
    std::string result;
    
    // We append " 2>&1" to redirect stderr to stdout
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen((command_str + " 2>&1").c_str(), "r"), pclose);
    if (!pipe) {
        return "popen() failed!";
    }

    // Rule #3: Ensure buffer operations are safe.
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return result;
}

int main() {
    // The prompt's decorator concept points to a web server context, which is too complex
    // for a single C++ file example. We demonstrate the core function with test cases.
    std::vector<std::string> testUrls = {
        "8.8.8.8",
        "localhost",
        "example.com",
        "127.0.0.1",
        "invalid-hostname; ls -la" // Malicious input
    };
    
    std::cout << "--- Running 5 Test Cases ---" << std::endl;
    for (int i = 0; i < testUrls.size(); ++i) {
        std::cout << "\n--- Test Case " << (i + 1) << ": pinging '" << testUrls[i] << "' ---" << std::endl;
        std::string result = ping(testUrls[i]);
        std::cout << result;
        std::cout << "--- End Test Case " << (i + 1) << " ---" << std::endl;
    }

    return 0;
}