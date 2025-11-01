
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

std::string readFileFromSafeDirectory(const std::string& filename) {
    std::string safePath = "/safe/" + filename;
    std::ifstream file(safePath);
    
    if (!file.is_open()) {
        return "Error reading file: Unable to open file";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    
    return buffer.str();
}

int main() {
    // Test cases
    std::string testFiles[] = {
        "test1.txt",
        "test2.txt",
        "data.txt",
        "config.txt",
        "info.txt"
    };
    
    for (const auto& file : testFiles) {
        std::cout << "Reading file: " << file << std::endl;
        std::string content = readFileFromSafeDirectory(file);
        std::cout << "Content: " << content << std::endl;
        std::cout << "---" << std::endl;
    }
    
    return 0;
}
