#include <iostream>
#include <string>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include <vector>
#include <algorithm>
#include <cctype>

#ifdef _WIN32
#define POPEN _popen
#define PCLOSE _pclose
#else
#define POPEN popen
#define PCLOSE pclose
#endif

// Security: Whitelist of allowed characters for a hostname.
// This is a crucial step to prevent command injection.
bool isValidHostnameChar(char c) {
    return isalnum(static_cast<unsigned char>(c)) || c == '.' || c == '-';
}

// Security: Validates the input string to ensure it only contains characters
// suitable for a hostname, preventing shell metacharacters.
bool isSecureHostname(const std::string& url) {
    if (url.empty() || url.length() > 255) {
        return false;
    }
    if (url.front() == '-' || url.back() == '-' || url.front() == '.' || url.back() == '.') {
        return false;
    }
    // Check if all characters are valid hostname characters.
    return std::all_of(url.begin(), url.end(), isValidHostnameChar);
}

/**
 * Executes the ping command for a given URL.
 * @param url The URL or IP address to ping.
 * @return The output of the ping command, or an error message.
 */
std::string ping(const std::string& url) {
    // Security: Sanitize and validate the input URL.
    if (!isSecureHostname(url)) {
        return "Error: Invalid or insecure hostname format provided.";
    }

    std::string command;
#ifdef _WIN32
    // For Windows, -n specifies the number of echo requests.
    command = "ping -n 4 " + url;
#else
    // For Linux/macOS, -c specifies the count.
    command = "ping -c 4 " + url;
#endif

    // Security Note: popen executes the command via a shell (/bin/sh -c).
    // Because we have strictly validated the 'url' input against a whitelist
    // of characters, command injection is mitigated. However, using APIs like
    // fork/exec (on POSIX) or CreateProcess (on Windows) that do not invoke a shell
    // would be an even more secure approach, as it separates the command and its arguments completely.
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&PCLOSE)> pipe(POPEN(command.c_str(), "r"), PCLOSE);
    if (!pipe) {
        return "Error: popen() failed!";
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

int main() {
    std::vector<std::string> testUrls = {
        "8.8.8.8",                  // Valid IP
        "google.com",               // Valid hostname
        "localhost",                // Valid hostname
        "invalid!hostname",         // Invalid characters
        "8.8.8.8; ls -la"           // Malicious command injection attempt
    };

    for (const auto& url : testUrls) {
        std::cout << "--- Pinging: " << url << " ---" << std::endl;
        std::string output = ping(url);
        std::cout << output << std::endl;
        std::cout << "----------------------------------" << std::endl << std::endl;
    }

    return 0;
}