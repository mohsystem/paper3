#include <iostream>
#include <string>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include <vector>

/**
 * Executes 'cat /etc/passwd | grep <username>' and returns the output.
 * @param username The username to search for.
 * @return A string containing the user's entry or a status message.
 */
std::string getUserInfo(const std::string& username) {
    // Use a more specific grep to avoid partial matches and anchor to the start of the line.
    std::string command = "cat /etc/passwd | grep '^" + username + ":'";
    std::string result;
    std::array<char, 256> buffer;

    // popen is a POSIX function that runs a command in a shell and connects
    // a pipe to its standard input or output. 'r' is for reading.
    // A unique_ptr with a custom deleter (pclose) ensures the pipe is closed.
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
    
    if (!pipe) {
        return "popen() failed!\n";
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    if (result.empty()) {
        return "User '" + username + "' not found.\n";
    }
    
    return result;
}

int main(int argc, char* argv[]) {
    // argc is the argument count; argv[0] is the program name.
    if (argc > 1) {
        // If a username is provided, use it.
        std::cout << getUserInfo(argv[1]);
    } else {
        // Otherwise, run test cases.
        std::cout << "No command line argument provided. Running built-in test cases:" << std::endl;
        std::cout << "===============================================================" << std::endl;
        std::vector<std::string> testCases = {"root", "daemon", "bin", "sys", "nonexistentuser123"};
        for (const std::string& user : testCases) {
            std::cout << "\n--- Testing for user: '" << user << "' ---" << std::endl;
            std::cout << getUserInfo(user);
        }
    }
    return 0;
}