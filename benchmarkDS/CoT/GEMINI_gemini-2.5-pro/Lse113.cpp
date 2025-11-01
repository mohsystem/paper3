#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>

// Security: A helper function to validate the hostname.
// It allows only alphanumeric characters, hyphens, and dots.
// This is a crucial step to mitigate command injection when using system().
bool isValidHostname(const std::string& host) {
    if (host.empty() || host.length() > 255) {
        return false;
    }
    for (char c : host) {
        if (!isalnum(c) && c != '.' && c != '-') {
            return false;
        }
    }
    return true;
}

/**
 * Pings a given host/URL to check if it is reachable.
 *
 * @param url The host or IP address to ping.
 * @return A string indicating if the URL is "alive" or "not alive".
 */
std::string ping(const std::string& url) {
    if (!isValidHostname(url)) {
        return "URL is not alive (Invalid format).";
    }

    std::string command;
    // Differentiate ping command between Windows and other OS
#ifdef _WIN32
    // -n 1 sends 1 echo request
    // > nul redirects standard output to null device to hide it
    command = "ping -n 1 " + url + " > nul";
#else
    // -c 1 sends 1 packet
    // > /dev/null 2>&1 redirects stdout and stderr to null device
    command = "ping -c 1 " + url + " > /dev/null 2>&1";
#endif

    // The output of the ping command is stored in "pong" (conceptually).
    // system() is used for simplicity, but it's less secure than alternatives
    // like fork/exec on POSIX systems. The input validation above is essential.
    int exitCode = system(command.c_str());

    // If the ping command returns 0, the URL is alive.
    if (exitCode == 0) {
        return "URL is alive.";
    } else {
        return "URL is not alive.";
    }
}

int main() {
    std::vector<std::string> testUrls = {
        "google.com",           // Test case 1: A valid, reachable domain
        "8.8.8.8",              // Test case 2: A valid, reachable IP address
        "localhost",            // Test case 3: Localhost
        "invalidurl12345.xyz",  // Test case 4: A non-existent domain
        "google.com; ls"        // Test case 5: A potential command injection attempt (should be rejected by validation)
    };

    for (const auto& url : testUrls) {
        std::string pong = ping(url);
        std::cout << "Pinging " << url << ": " << pong << std::endl;
    }

    return 0;
}