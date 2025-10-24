
#include <iostream>
#include <stdexcept>
#include <memory>
#include <vector>

class Task159 {
private:
    static const int BUFFER_SIZE = 100;
    
public:
    static std::unique_ptr<int[]> allocateBuffer(int size) {
        if (size <= 0 || size > 1000000) {
            throw std::invalid_argument("Invalid buffer size");
        }
        return std::make_unique<int[]>(size);
    }
    
    static void initializeBuffer(int* buffer, int size) {
        if (buffer == nullptr) {
            throw std::invalid_argument("Buffer cannot be null");
        }
        for (int i = 0; i < size; i++) {
            buffer[i] = i * 10;
        }
    }
    
    static int readFromBuffer(const int* buffer, int bufferSize, int index) {
        if (buffer == nullptr) {
            throw std::invalid_argument("Buffer cannot be null");
        }
        if (index < 0 || index >= bufferSize) {
            throw std::out_of_range("Index out of bounds: " + std::to_string(index));
        }
        return buffer[index];
    }
};

int main() {
    std::cout << "=== Secure Memory Buffer Access Test Cases ===\\n\\n";
    
    // Test Case 1: Normal valid access
    try {
        auto buffer1 = Task159::allocateBuffer(100);
        Task159::initializeBuffer(buffer1.get(), 100);
        int result = Task159::readFromBuffer(buffer1.get(), 100, 50);
        std::cout << "Test 1 - Valid access at index 50: " << result << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 1 - Error: " << e.what() << std::endl;
    }
    
    // Test Case 2: Access at boundary (first element)
    try {
        auto buffer2 = Task159::allocateBuffer(100);
        Task159::initializeBuffer(buffer2.get(), 100);
        int result = Task159::readFromBuffer(buffer2.get(), 100, 0);
        std::cout << "Test 2 - Valid access at index 0: " << result << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 2 - Error: " << e.what() << std::endl;
    }
    
    // Test Case 3: Access at boundary (last element)
    try {
        auto buffer3 = Task159::allocateBuffer(100);
        Task159::initializeBuffer(buffer3.get(), 100);
        int result = Task159::readFromBuffer(buffer3.get(), 100, 99);
        std::cout << "Test 3 - Valid access at index 99: " << result << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 3 - Error: " << e.what() << std::endl;
    }
    
    // Test Case 4: Invalid access (negative index)
    try {
        auto buffer4 = Task159::allocateBuffer(100);
        Task159::initializeBuffer(buffer4.get(), 100);
        int result = Task159::readFromBuffer(buffer4.get(), 100, -5);
        std::cout << "Test 4 - Access at index -5: " << result << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 4 - Error (Expected): " << e.what() << std::endl;
    }
    
    // Test Case 5: Invalid access (out of bounds)
    try {
        auto buffer5 = Task159::allocateBuffer(100);
        Task159::initializeBuffer(buffer5.get(), 100);
        int result = Task159::readFromBuffer(buffer5.get(), 100, 150);
        std::cout << "Test 5 - Access at index 150: " << result << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 5 - Error (Expected): " << e.what() << std::endl;
    }
    
    return 0;
}
