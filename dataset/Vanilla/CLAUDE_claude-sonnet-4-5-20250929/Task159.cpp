
#include <iostream>
#include <cstdlib>

class Task159 {
private:
    int* buffer;
    int bufferSize;
    
public:
    Task159() : buffer(nullptr), bufferSize(0) {}
    
    ~Task159() {
        deallocateBuffer();
    }
    
    void initializeBuffer(int size, int* initialData = nullptr, int dataSize = 0) {
        deallocateBuffer();
        bufferSize = size;
        buffer = new int[size];
        
        for (int i = 0; i < size; i++) {
            buffer[i] = 0;
        }
        
        if (initialData != nullptr) {
            for (int i = 0; i < size && i < dataSize; i++) {
                buffer[i] = initialData[i];
            }
        }
    }
    
    bool readFromBuffer(int index, int& result) {
        if (buffer == nullptr) {
            std::cout << "Error: Buffer not initialized" << std::endl;
            return false;
        }
        if (index < 0 || index >= bufferSize) {
            std::cout << "Error: Index out of bounds" << std::endl;
            return false;
        }
        result = buffer[index];
        return true;
    }
    
    void deallocateBuffer() {
        if (buffer != nullptr) {
            delete[] buffer;
            buffer = nullptr;
        }
        bufferSize = 0;
    }
};

int main() {
    // Test case 1: Basic read operation
    std::cout << "Test Case 1: Basic read operation" << std::endl;
    Task159 task1;
    int data1[] = {10, 20, 30, 40, 50};
    task1.initializeBuffer(5, data1, 5);
    int result;
    if (task1.readFromBuffer(2, result)) {
        std::cout << "Read from index 2: " << result << std::endl;
    }
    task1.deallocateBuffer();
    std::cout << std::endl;
    
    // Test case 2: Read from first index
    std::cout << "Test Case 2: Read from first index" << std::endl;
    Task159 task2;
    int data2[] = {100, 200, 300};
    task2.initializeBuffer(3, data2, 3);
    if (task2.readFromBuffer(0, result)) {
        std::cout << "Read from index 0: " << result << std::endl;
    }
    task2.deallocateBuffer();
    std::cout << std::endl;
    
    // Test case 3: Read from last index
    std::cout << "Test Case 3: Read from last index" << std::endl;
    Task159 task3;
    int data3[] = {5, 15, 25, 35, 45, 55};
    task3.initializeBuffer(6, data3, 6);
    if (task3.readFromBuffer(5, result)) {
        std::cout << "Read from index 5: " << result << std::endl;
    }
    task3.deallocateBuffer();
    std::cout << std::endl;
    
    // Test case 4: Invalid index (negative)
    std::cout << "Test Case 4: Invalid index (negative)" << std::endl;
    Task159 task4;
    int data4[] = {1, 2, 3, 4};
    task4.initializeBuffer(4, data4, 4);
    if (!task4.readFromBuffer(-1, result)) {
        std::cout << "Result: Failed to read" << std::endl;
    }
    task4.deallocateBuffer();
    std::cout << std::endl;
    
    // Test case 5: Invalid index (out of bounds)
    std::cout << "Test Case 5: Invalid index (out of bounds)" << std::endl;
    Task159 task5;
    int data5[] = {7, 14, 21, 28};
    task5.initializeBuffer(4, data5, 4);
    if (!task5.readFromBuffer(10, result)) {
        std::cout << "Result: Failed to read" << std::endl;
    }
    task5.deallocateBuffer();
    std::cout << std::endl;
    
    return 0;
}
