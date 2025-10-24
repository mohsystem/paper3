
#include <iostream>
#include <string>
#include <cstdlib>
#include <array>
#include <memory>
#include <sstream>

#ifdef _WIN32
#define POPEN _popen
#define PCLOSE _pclose
#else
#define POPEN popen
#define PCLOSE pclose
#endif

// INSECURE: Vulnerable to command injection
std::string displayFileContents(const std::string& filename) {
    std::string command;
    
#ifdef _WIN32
    command = "type " + filename;
#else
    command = "cat " + filename;
#endif
    
    std::stringstream output;
    std::array<char, 128> buffer;
    
    FILE* pipe = POPEN(command.c_str(), "r");
    if (!pipe) {
        return "Error: Could not execute command";
    }
    
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        output << buffer.data();
    }
    
    PCLOSE(pipe);
    return output.str();
}

int main() {
    std::cout << "WARNING: This code is vulnerable to command injection!" << std::endl;
    std::cout << "Test cases:\\n" << std::endl;
    
    std::string testCases[] = {
        "test.txt",
        "document.log",
        "data.csv",
        "config.ini",
        "readme.md"
    };
    
    for (int i = 0; i < 5; i++) {
        std::cout << "Test Case " << (i + 1) << ": " << testCases[i] << std::endl;
        std::cout << displayFileContents(testCases[i]) << std::endl;
        std::cout << "---" << std::endl;
    }
    
    return 0;
}
