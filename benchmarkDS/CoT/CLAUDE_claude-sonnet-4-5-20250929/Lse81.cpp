
#include <iostream>
#include <cstring>
#include <algorithm>

const int BUFFER_SIZE = 1024;
const int READ_SIZE = 10;

char* allocateBuffer() {
    char* buffer = new char[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    return buffer;
}

int readCharacters(char* buffer) {
    if (buffer == nullptr) {
        return -1;
    }
    
    std::string input;
    std::getline(std::cin, input);
    
    int charsToRead = std::min(READ_SIZE, static_cast<int>(input.length()));
    charsToRead = std::min(charsToRead, BUFFER_SIZE - 1);
    
    for (int i = 0; i < charsToRead; i++) {
        buffer[i] = input[i];
    }
    buffer[charsToRead] = '\\0';
    
    return charsToRead;
}

int main() {
    // Test case 1: Normal input
    std::cout << "Test 1 - Enter 10 characters:" << std::endl;
    char* buf1 = allocateBuffer();
    int read1 = readCharacters(buf1);
    std::cout << "Characters read: " << read1 << std::endl;
    delete[] buf1;
    
    // Test case 2: Less than 10 characters
    std::cout << "\\nTest 2 - Enter less than 10 characters:" << std::endl;
    char* buf2 = allocateBuffer();
    int read2 = readCharacters(buf2);
    std::cout << "Characters read: " << read2 << std::endl;
    delete[] buf2;
    
    // Test case 3: More than 10 characters
    std::cout << "\\nTest 3 - Enter more than 10 characters:" << std::endl;
    char* buf3 = allocateBuffer();
    int read3 = readCharacters(buf3);
    std::cout << "Characters read: " << read3 << std::endl;
    delete[] buf3;
    
    // Test case 4: Empty input
    std::cout << "\\nTest 4 - Press Enter without input:" << std::endl;
    char* buf4 = allocateBuffer();
    int read4 = readCharacters(buf4);
    std::cout << "Characters read: " << read4 << std::endl;
    delete[] buf4;
    
    // Test case 5: Special characters
    std::cout << "\\nTest 5 - Enter special characters:" << std::endl;
    char* buf5 = allocateBuffer();
    int read5 = readCharacters(buf5);
    std::cout << "Characters read: " << read5 << std::endl;
    delete[] buf5;
    
    return 0;
}
