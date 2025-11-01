
// C++ web server implementation using mongoose or similar library would be complex
// Here's a simplified concept using basic socket programming
// Note: This requires a proper HTTP server library for production use

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

class Lse46 {
public:
    static std::string img(const std::string& name) {
        std::string filePath = "images/" + name;
        std::ifstream file(filePath, std::ios::binary);
        
        if (!file.is_open()) {
            return "";
        }
        
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        file.close();
        return content;
    }
    
    static void runServer() {
        // Simplified server implementation
        std::cout << "Server concept - use proper HTTP library for production" << std::endl;
    }
};

int main() {
    // Test cases
    std::cout << "Testing img function with test cases:" << std::endl;
    
    std::string testCases[] = {"test1.jpg", "test2.png", "test3.gif", "test4.bmp", "test5.jpg"};
    
    for (int i = 0; i < 5; i++) {
        std::string result = Lse46::img(testCases[i]);
        if (!result.empty()) {
            std::cout << "Successfully read: " << testCases[i] << " (" << result.size() << " bytes)" << std::endl;
        } else {
            std::cout << "Failed to read: " << testCases[i] << std::endl;
        }
    }
    
    return 0;
}
