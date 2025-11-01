
#include <iostream>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <cstdarg>

std::string executeCommand(const std::string& command) {
    std::array<char, 128> buffer;
    std::string result;
    try {
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
        if (!pipe) {
            return "Error executing command";
        }
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
    } catch (const std::exception& e) {
        return "Error executing command: " + std::string(e.what());
    }
    return result;
}

std::string formatCommand(const char* format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    return std::string(buffer);
}

int main() {
    // Test case 1: List current directory
    std::string cmd1 = formatCommand("ls %s", ".");
    std::cout << "Test 1: " << cmd1 << std::endl;
    std::cout << executeCommand(cmd1) << std::endl;

    // Test case 2: List with -l flag
    std::string cmd2 = formatCommand("ls %s", "-l");
    std::cout << "Test 2: " << cmd2 << std::endl;
    std::cout << executeCommand(cmd2) << std::endl;

    // Test case 3: List with -a flag
    std::string cmd3 = formatCommand("ls %s", "-a");
    std::cout << "Test 3: " << cmd3 << std::endl;
    std::cout << executeCommand(cmd3) << std::endl;

    // Test case 4: List with -la flags
    std::string cmd4 = formatCommand("ls %s %s", "-l", "-a");
    std::cout << "Test 4: " << cmd4 << std::endl;
    std::cout << executeCommand(cmd4) << std::endl;

    // Test case 5: List specific directory
    std::string cmd5 = formatCommand("ls %s", "/tmp");
    std::cout << "Test 5: " << cmd5 << std::endl;
    std::cout << executeCommand(cmd5) << std::endl;

    return 0;
}
