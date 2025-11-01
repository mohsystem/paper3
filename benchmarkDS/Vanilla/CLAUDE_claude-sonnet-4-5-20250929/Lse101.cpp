
#include <iostream>
#include <fstream>
#include <string>

void writeToFile(const std::string& filename, const std::string& content) {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << content;
        file.close();
    } else {
        std::cerr << "Error opening file: " << filename << std::endl;
    }
}

int main() {
    // Test case 1
    writeToFile("important_config", "important_config");
    
    // Test case 2
    writeToFile("test_file1.txt", "test content 1");
    
    // Test case 3
    writeToFile("test_file2.txt", "test content 2");
    
    // Test case 4
    writeToFile("config.txt", "configuration data");
    
    // Test case 5
    writeToFile("data.txt", "sample data");
    
    std::cout << "Files written successfully!" << std::endl;
    
    return 0;
}
