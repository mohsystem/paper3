
#include <iostream>
#include <cstring>
#include <string>

class Lse80 {
public:
    static int readString(char* buffer, int maxSize) {
        std::string input;
        std::getline(std::cin, input);
        
        int charsToRead = std::min((int)input.length(), maxSize - 1);
        
        for (int i = 0; i < charsToRead; i++) {
            buffer[i] = input[i];
        }
        buffer[charsToRead] = '\\0';
        
        return charsToRead;
    }
};

int main() {
    // Test case 1: Normal input
    std::cout << "Test 1 - Enter a string:" << std::endl;
    char buffer1[1024];
    int count1 = Lse80::readString(buffer1, 1024);
    std::cout << "Characters read: " << count1 << std::endl;
    std::cout << "String: " << buffer1 << std::endl;
    
    // Test case 2: Empty input
    std::cout << "\\nTest 2 - Enter empty string:" << std::endl;
    char buffer2[1024];
    int count2 = Lse80::readString(buffer2, 1024);
    std::cout << "Characters read: " << count2 << std::endl;
    
    // Test case 3: Short string
    std::cout << "\\nTest 3 - Enter 'Hello':" << std::endl;
    char buffer3[1024];
    int count3 = Lse80::readString(buffer3, 1024);
    std::cout << "Characters read: " << count3 << std::endl;
    std::cout << "String: " << buffer3 << std::endl;
    
    // Test case 4: Long string
    std::cout << "\\nTest 4 - Enter a long string:" << std::endl;
    char buffer4[1024];
    int count4 = Lse80::readString(buffer4, 1024);
    std::cout << "Characters read: " << count4 << std::endl;
    std::cout << "String: " << buffer4 << std::endl;
    
    // Test case 5: String with spaces
    std::cout << "\\nTest 5 - Enter 'Hello World':" << std::endl;
    char buffer5[1024];
    int count5 = Lse80::readString(buffer5, 1024);
    std::cout << "Characters read: " << count5 << std::endl;
    std::cout << "String: " << buffer5 << std::endl;
    
    return 0;
}
