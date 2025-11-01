
#include <iostream>
#include <string>
#include <cstdlib>
#include <array>
#include <memory>
#include <stdexcept>

std::string executeCommand(const std::string& username) {
    std::string command = "cat /etc/passwd | grep " + username;
    std::array<char, 128> buffer;
    std::string result;
    
    try {
        std::unique_ptr<FILE, decltype(&pclose)> pipe(
            popen(command.c_str(), "r"), pclose
        );
        
        if (!pipe) {
            return "Error executing command";
        }
        
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
    } catch (const std::exception& e) {
        return "Error: " + std::string(e.what());
    }
    
    return result;
}

int main(int argc, char* argv[]) {
    std::string testUsers[] = {"root", "daemon", "bin", "sys", "nobody"};
    
    if (argc > 1) {
        std::string username = argv[1];
        std::cout << "User information for: " << username << std::endl;
        std::cout << executeCommand(username);
    } else {
        std::cout << "Running test cases:" << std::endl;
        for (const auto& user : testUsers) {
            std::cout << "=== User: " << user << " ===" << std::endl;
            std::cout << executeCommand(user);
        }
    }
    
    return 0;
}
