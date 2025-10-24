#include <iostream>
#include <string>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include <regex>
#include <vector>

// Note: This implementation depends on the 'curl' command-line tool.
// It must be installed and available in the system's PATH to function.

/**
 * Executes a command and returns its standard output.
 * @param cmd The command to execute.
 * @return The stdout of the command.
 */
std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

/**
 * Fetches the content of a URL via curl and extracts the page title.
 * @param url The HTTPS URL to fetch.
 * @return The extracted page title, or an error/not found message.
 */
std::string getPageTitle(const std::string& url) {
    try {
        std::string command = "curl -sL \"";
        command += url;
        command += "\"";
        std::string html = exec(command.c_str());
        
        if (html.empty()) {
            return "Error: Failed to retrieve page content (is curl installed?).";
        }
        
        std::smatch match;
        // Regex to find content inside <title> tag, case-insensitive
        std::regex title_regex("<title.*?>(.*?)</title>", std::regex::icase | std::regex::ECMAScript);
        
        if (std::regex_search(html, match, title_regex) && match.size() > 1) {
            std::string title = match[1].str();
            // Trim whitespace
            const char* t = " \t\n\r\f\v";
            title.erase(0, title.find_first_not_of(t));
            title.erase(title.find_last_not_of(t) + 1);
            return title;
        } else {
            return "Title not found.";
        }
    } catch (const std::exception& e) {
        return std::string("Error: ") + e.what();
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::cout << "Processing URL from command line: " << argv[1] << std::endl;
        std::string title = getPageTitle(argv[1]);
        std::cout << "Title: " << title << std::endl;
        std::cout << "\n--- Running built-in test cases ---" << std::endl;
    }

    std::vector<std::string> testUrls = {
        "https://www.google.com",
        "https://github.com",
        "https://www.oracle.com/java/",
        "https://www.python.org",
        "https://isocpp.org"
    };

    for (const auto& url : testUrls) {
        std::cout << "Fetching title for: " << url << std::endl;
        std::string title = getPageTitle(url);
        std::cout << "-> Title: " << title << std::endl << std::endl;
    }

    return 0;
}