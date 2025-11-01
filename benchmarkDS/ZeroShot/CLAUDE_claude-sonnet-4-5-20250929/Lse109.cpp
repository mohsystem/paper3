
#include <iostream>
#include <fstream>
#include <string>
#include <regex>

std::string getUserInfo(const std::string& username) {
    // Validate input
    if (username.empty()) {
        return "Error: Username cannot be empty";
    }
    
    // Validate username format (alphanumeric, underscore, hyphen only)
    std::regex validPattern("^[a-zA-Z0-9_-]+$");
    if (!std::regex_match(username, validPattern)) {
        return "Error: Invalid username format";
    }
    
    // Directly read the file without shell execution
    std::ifstream passwdFile("/etc/passwd");
    if (!passwdFile.is_open()) {
        return "Error: Cannot open /etc/passwd";
    }
    
    std::string line;
    std::string searchPrefix = username + ":";
    
    while (std::getline(passwdFile, line)) {
        if (line.rfind(searchPrefix, 0) == 0) {
            passwdFile.close();
            return line;
        }
    }
    
    passwdFile.close();
    return "";
}

int main() {
    std::string testUsers[] = {"root", "daemon", "nobody", "testuser", "admin"};
    
    std::cout << "=== User Information Lookup (Secure) ===" << std::endl << std::endl;
    
    for (const auto& user : testUsers) {
        std::cout << "Searching for user: " << user << std::endl;
        std::string result = getUserInfo(user);
        std::cout << (result.empty() ? "User not found" : result) << std::endl;
        std::cout << "---" << std::endl;
    }
    
    return 0;
}
