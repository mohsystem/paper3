#include <iostream>
#include <string>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include <vector>
#include <regex>

/**
 * Validates if the string contains only characters suitable for a hostname.
 * This is a critical security function to prevent command injection.
 * @param hostname The string to validate.
 * @return True if the string is a valid hostname format, false otherwise.
 */
bool isValidHostname(const std::string& hostname) {
    // Regex allows alphanumeric characters, hyphens, and dots.
    const std::regex pattern("^[a-zA-Z0-9.-]+$");
    return std::regex_match(hostname, pattern);
}

/**
 * Executes the ping command and captures its output.
 * Uses popen, so input validation is crucial.
 * @param url The URL or IP to ping.
 * @return The stdout of the ping command.
 */
std::string pingUrl(const std::string& url) {
    if (!isValidHostname(url)) {
        return "Error: Invalid characters in URL. Possible command injection attempt blocked.";
    }

    std::string command;
#ifdef _WIN32
    // For Windows, use -n for count
    command = "ping -n 4 " + url;
#else
    // For Linux, macOS, etc., use -c for count
    command = "ping -c 4 " + url;
#endif

    std::array<char, 128> buffer;
    std::string result;
    // Using a smart pointer for the pipe to ensure pclose is called.
    // popen opens a shell, which is why the input validation above is critical.
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe) {
        return "popen() failed!";
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

int main() {
    std::vector<std::string> testUrls = {
        "google.com",                  // Valid domain
        "8.8.8.8",                     // Valid IP address
        "nonexistent.domain.xyz",      // Should fail to resolve
        "127.0.0.1",                   // Localhost
        "google.com; ls"               // Command injection attempt
    };

    for (const auto& url : testUrls) {
        std::cout << "--- Pinging: " << url << " ---" << std::endl;
        std::string result = pingUrl(url);
        std::cout << result << std::endl;
        std::cout << "----------------------------------" << std::endl << std::endl;
    }

    return 0;
}