
#include <iostream>
#include <memory>
#include <stdexcept>

class Task159 {
private:
    static const int BUFFER_SIZE = 100;
    std::unique_ptr<int[]> buffer;
    int size;
    
public:
    Task159() : size(BUFFER_SIZE) {
        buffer = std::make_unique<int[]>(BUFFER_SIZE);
        // Initialize buffer with sample data
        for (int i = 0; i < BUFFER_SIZE; i++) {
            buffer[i] = i * 10;
        }
    }
    
    bool readFromBuffer(int index, int& value) {
        // Secure bounds checking
        if (index < 0 || index >= size) {
            std::cout << "Error: Index out of bounds. Valid range: 0-" 
                      << (size - 1) << std::endl;
            return false;
        }
        value = buffer[index];
        return true;
    }
    
    int getBufferSize() const {
        return size;
    }
};

int main() {
    Task159 task;
    int value;
    
    std::cout << "=== Test Cases ===" << std::endl << std::endl;
    
    // Test case 1: Valid index at start
    std::cout << "Test 1 - Valid index (0):" << std::endl;
    if (task.readFromBuffer(0, value)) {
        std::cout << "Value at index 0: " << value << std::endl;
    }
    std::cout << std::endl;
    
    // Test case 2: Valid index in middle
    std::cout << "Test 2 - Valid index (50):" << std::endl;
    if (task.readFromBuffer(50, value)) {
        std::cout << "Value at index 50: " << value << std::endl;
    }
    std::cout << std::endl;
    
    // Test case 3: Valid index at end
    std::cout << "Test 3 - Valid index (99):" << std::endl;
    if (task.readFromBuffer(99, value)) {
        std::cout << "Value at index 99: " << value << std::endl;
    }
    std::cout << std::endl;
    
    // Test case 4: Invalid negative index
    std::cout << "Test 4 - Invalid negative index (-5):" << std::endl;
    task.readFromBuffer(-5, value);
    std::cout << std::endl;
    
    // Test case 5: Invalid index beyond bounds
    std::cout << "Test 5 - Invalid index beyond bounds (150):" << std::endl;
    task.readFromBuffer(150, value);
    std::cout << std::endl;
    
    return 0;
}
