
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <cstdio>

void displayFileContents(const std::string& filename) {
    std::string command;
    
    #ifdef _WIN32
        command = "type \\"" + filename + "\\"";
    #else
        command = "cat \\"" + filename + "\\"";
    #endif
    
    int result = system(command.c_str());
    
    if (result != 0) {
        std::cerr << "Error executing command" << std::endl;
    }
}

void createTestFile(const std::string& filename, const std::string& content) {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << content;
        file.close();
    }
}

int main() {
    // Create test files
    std::string testFiles[] = {"test1.txt", "test2.txt", "test3.txt", "test4.txt", "test5.txt"};
    std::string testContents[] = {
        "Hello World!",
        "This is test file 2.\\nWith multiple lines.",
        "Test file 3 contents",
        "Line 1\\nLine 2\\nLine 3",
        "Final test file"
    };
    
    // Create test files
    for (int i = 0; i < 5; i++) {
        createTestFile(testFiles[i], testContents[i]);
    }
    
    // Test cases
    for (int i = 0; i < 5; i++) {
        std::cout << "\\n--- Test Case " << (i + 1) << ": " << testFiles[i] << " ---" << std::endl;
        displayFileContents(testFiles[i]);
    }
    
    // Clean up test files
    for (int i = 0; i < 5; i++) {
        remove(testFiles[i].c_str());
    }
    
    return 0;
}
