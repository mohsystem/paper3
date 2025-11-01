
#include <iostream>
#include <cstring>
#include <string>

class Lse80 {
public:
    static int secureReadInput(char* buffer, int maxSize) {
        if (buffer == nullptr || maxSize <= 0) {
            return -1;
        }
        
        std::string input;
        if (!std::getline(std::cin, input)) {
            return 0;
        }
        
        // Limit input to maxSize - 1 to leave room for null terminator
        int charsToRead = std::min((int)input.length(), maxSize - 1);
        
        // Copy characters securely
        std::strncpy(buffer, input.c_str(), charsToRead);
        
        // Ensure null termination
        buffer[charsToRead] = '\\0';
        
        return charsToRead;
    }
};

int main() {
    std::cout << "Test Case 1: Short input" << std::endl;
    char buffer1[1024];
    std::cout << "Enter text: ";
    int chars1 = Lse80::secureReadInput(buffer1, 1024);
    std::cout << "Characters read: " << chars1 << std::endl;
    
    std::cout << "\\nTest Case 2: Medium input" << std::endl;
    char buffer2[1024];
    std::cout << "Enter text: ";
    int chars2 = Lse80::secureReadInput(buffer2, 1024);
    std::cout << "Characters read: " << chars2 << std::endl;
    
    std::cout << "\\nTest Case 3: Long input (near limit)" << std::endl;
    char buffer3[1024];
    std::cout << "Enter text: ";
    int chars3 = Lse80::secureReadInput(buffer3, 1024);
    std::cout << "Characters read: " << chars3 << std::endl;
    
    std::cout << "\\nTest Case 4: Empty input" << std::endl;
    char buffer4[1024];
    std::cout << "Enter text: ";
    int chars4 = Lse80::secureReadInput(buffer4, 1024);
    std::cout << "Characters read: " << chars4 << std::endl;
    
    std::cout << "\\nTest Case 5: Input exceeding buffer size" << std::endl;
    char buffer5[1024];
    std::cout << "Enter text: ";
    int chars5 = Lse80::secureReadInput(buffer5, 1024);
    std::cout << "Characters read: " << chars5 << std::endl;
    
    return 0;
}
