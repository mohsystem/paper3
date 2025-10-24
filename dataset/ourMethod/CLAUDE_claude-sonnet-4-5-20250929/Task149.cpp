
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <limits>

// Queue data structure using a dynamic array
// Security: All operations validate bounds and check for overflow/underflow
class Queue {
private:
    int* data;           // Dynamic array to store queue elements
    size_t capacity;     // Maximum capacity of the queue
    size_t frontIndex;   // Index of the front element
    size_t rearIndex;    // Index where next element will be inserted
    size_t count;        // Current number of elements
    
    static const size_t MAX_CAPACITY = 1000000; // Prevent excessive memory usage
    
public:
    // Constructor: Initialize queue with given capacity
    // Security: Validate capacity to prevent integer overflow and excessive allocation
    explicit Queue(size_t initialCapacity = 10) : data(nullptr), capacity(0), frontIndex(0), rearIndex(0), count(0) {
        // Validate capacity is within safe bounds
        if (initialCapacity == 0 || initialCapacity > MAX_CAPACITY) {
            std::cerr << "Invalid capacity. Using default (10)." << std::endl;
            initialCapacity = 10;
        }
        
        capacity = initialCapacity;
        // Security: Check malloc return for NULL
        data = static_cast<int*>(std::malloc(capacity * sizeof(int)));
        if (data == nullptr) {
            std::cerr << "Memory allocation failed" << std::endl;
            capacity = 0;
            return;
        }
        // Security: Initialize allocated memory
        std::memset(data, 0, capacity * sizeof(int));
    }
    
    // Destructor: Free allocated memory
    ~Queue() {
        if (data != nullptr) {
            // Security: Clear sensitive data before freeing (if queue contained sensitive data)
            std::memset(data, 0, capacity * sizeof(int));
            std::free(data);
            data = nullptr;
        }
    }
    
    // Enqueue: Add element to rear of queue
    // Security: Validate queue state and check bounds before insertion
    bool enqueue(int value) {
        // Check if queue was properly initialized
        if (data == nullptr) {
            std::cerr << "Queue not initialized" << std::endl;
            return false;
        }
        
        // Check if queue is full
        if (count >= capacity) {
            std::cerr << "Queue is full" << std::endl;
            return false;
        }
        
        // Security: Bounds check - ensure rearIndex is within valid range
        if (rearIndex >= capacity) {
            std::cerr << "Invalid rear index" << std::endl;
            return false;
        }
        
        // Insert element at rear
        data[rearIndex] = value;
        rearIndex = (rearIndex + 1) % capacity; // Circular increment
        count++;
        
        return true;
    }
    
    // Dequeue: Remove and return element from front of queue
    // Security: Validate queue state and check for underflow
    bool dequeue(int* result) {
        // Validate output parameter
        if (result == nullptr) {
            std::cerr << "Invalid result pointer" << std::endl;
            return false;
        }
        
        // Check if queue was properly initialized
        if (data == nullptr) {
            std::cerr << "Queue not initialized" << std::endl;
            return false;
        }
        
        // Check if queue is empty
        if (count == 0) {
            std::cerr << "Queue is empty" << std::endl;
            return false;
        }
        
        // Security: Bounds check - ensure frontIndex is within valid range
        if (frontIndex >= capacity) {
            std::cerr << "Invalid front index" << std::endl;
            return false;
        }
        
        // Remove element from front
        *result = data[frontIndex];
        data[frontIndex] = 0; // Clear the slot
        frontIndex = (frontIndex + 1) % capacity; // Circular increment
        count--;
        
        return true;
    }
    
    // Peek: Return element at front without removing it
    // Security: Validate queue state and check for underflow
    bool peek(int* result) const {
        // Validate output parameter
        if (result == nullptr) {
            std::cerr << "Invalid result pointer" << std::endl;
            return false;
        }
        
        // Check if queue was properly initialized
        if (data == nullptr) {
            std::cerr << "Queue not initialized" << std::endl;
            return false;
        }
        
        // Check if queue is empty
        if (count == 0) {
            std::cerr << "Queue is empty" << std::endl;
            return false;
        }
        
        // Security: Bounds check - ensure frontIndex is within valid range
        if (frontIndex >= capacity) {
            std::cerr << "Invalid front index" << std::endl;
            return false;
        }
        
        // Return front element without removing
        *result = data[frontIndex];
        return true;
    }
    
    // Check if queue is empty
    bool isEmpty() const {
        return count == 0;
    }
    
    // Get current size of queue
    size_t size() const {
        return count;
    }
};

// Test cases
int main() {
    std::cout << "=== Queue Implementation Test Cases ===" << std::endl;
    
    // Test Case 1: Basic enqueue and dequeue operations
    std::cout << "\\nTest Case 1: Basic operations" << std::endl;
    Queue q1(5);
    q1.enqueue(10);
    q1.enqueue(20);
    q1.enqueue(30);
    int value = 0;
    if (q1.dequeue(&value)) {
        std::cout << "Dequeued: " << value << std::endl; // Expected: 10
    }
    if (q1.peek(&value)) {
        std::cout << "Peek: " << value << std::endl; // Expected: 20
    }
    
    // Test Case 2: Queue overflow handling
    std::cout << "\\nTest Case 2: Overflow handling" << std::endl;
    Queue q2(3);
    q2.enqueue(1);
    q2.enqueue(2);
    q2.enqueue(3);
    if (!q2.enqueue(4)) { // Should fail - queue is full
        std::cout << "Correctly prevented overflow" << std::endl;
    }
    
    // Test Case 3: Queue underflow handling
    std::cout << "\\nTest Case 3: Underflow handling" << std::endl;
    Queue q3(3);
    if (!q3.dequeue(&value)) { // Should fail - queue is empty
        std::cout << "Correctly prevented underflow on dequeue" << std::endl;
    }
    if (!q3.peek(&value)) { // Should fail - queue is empty
        std::cout << "Correctly prevented underflow on peek" << std::endl;
    }
    
    // Test Case 4: Circular buffer behavior
    std::cout << "\\nTest Case 4: Circular buffer" << std::endl;
    Queue q4(3);
    q4.enqueue(100);
    q4.enqueue(200);
    q4.dequeue(&value);
    q4.enqueue(300);
    q4.enqueue(400);
    while (!q4.isEmpty()) {
        if (q4.dequeue(&value)) {
            std::cout << "Dequeued: " << value << std::endl; // Expected: 200, 300, 400
        }
    }
    
    // Test Case 5: Multiple operations sequence
    std::cout << "\\nTest Case 5: Multiple operations" << std::endl;
    Queue q5(10);
    for (int i = 1; i <= 5; i++) {
        q5.enqueue(i * 10);
    }
    std::cout << "Queue size: " << q5.size() << std::endl; // Expected: 5
    q5.peek(&value);
    std::cout << "Front element: " << value << std::endl; // Expected: 10
    q5.dequeue(&value);
    q5.dequeue(&value);
    std::cout << "After 2 dequeues, size: " << q5.size() << std::endl; // Expected: 3
    q5.peek(&value);
    std::cout << "New front element: " << value << std::endl; // Expected: 30
    
    return 0;
}
