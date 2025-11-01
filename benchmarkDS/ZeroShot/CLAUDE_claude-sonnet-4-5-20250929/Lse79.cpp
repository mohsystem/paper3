
#include <iostream>
#include <cstring>
#include <cstdlib>

class MemoryBuffer {
private:
    char* buffer;
    size_t size;
    
public:
    MemoryBuffer(size_t bufferSize) : size(bufferSize) {
        buffer = new char[size];
        memset(buffer, 0, size);
    }
    
    ~MemoryBuffer() {
        delete[] buffer;
    }
    
    void readInput(size_t maxChars) {
        size_t charsToRead = (maxChars < size - 1) ? maxChars : size - 1;
        
        if (std::cin.read(buffer, charsToRead)) {
            buffer[std::cin.gcount()] = '\\0';
        } else {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\\n');
        }
    }
    
    const char* getContent() const {
        return buffer;
    }
};

void allocateAndReadBuffers() {
    MemoryBuffer buf(1024);
    buf.readInput(100);
    
    MemoryBuffer buf1(1024);
    buf1.readInput(100);
    
    std::cout << "Buffer 1: " << buf.getContent() << std::endl;
    std::cout << "Buffer 2: " << buf1.getContent() << std::endl;
}

int main() {
    std::cout << "Test cases for memory allocation and input reading:" << std::endl;
    
    try {
        std::cout << "\\nTest Case 1: Normal execution" << std::endl;
        allocateAndReadBuffers();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    return 0;
}
