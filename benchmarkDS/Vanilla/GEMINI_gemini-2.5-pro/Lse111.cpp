#include <iostream>
#include <string>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include <vector>

/**
 * Executes the system's ping command for a given URL.
 *
 * @param url The URL or IP address to ping.
 * @return The output of the ping command as a string.
 */
std::string ping(const std::string& url) {
    std::string command;
#ifdef _WIN32
    // For Windows, use -n for count
    command = "ping -n 2 " + url;
#else
    // For Linux, macOS, etc., use -c for count
    command = "ping -c 2 " + url;
#endif

    std::array<char, 128> buffer;
    std::string result;
    // Append " 2>&1" to redirect stderr to stdout, capturing all output
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen((command + " 2>&1").c_str(), "r"), pclose);
    
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    
    return result;
}

int main() {
    std::vector<std::string> testUrls = {
        "google.com",
        "localhost",
        "127.0.0.1",
        "nonexistentdomain12345.xyz",
        "8.8.8.8"
    };

    for (const auto& url : testUrls) {
        std::cout << "--- Pinging " << url << " ---" << std::endl;
        try {
            std::string result = ping(url);
            std::cout << result << std::endl;
        } catch (const std::runtime_error& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
        std::cout << "--- End of Ping for " << url << " ---\n" << std::endl;
    }

    return 0;
}