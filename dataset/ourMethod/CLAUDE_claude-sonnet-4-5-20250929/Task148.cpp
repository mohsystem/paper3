
#include <iostream>
#include <vector>
#include <stdexcept>
#include <limits>
#include <cstdint>

// Stack implementation with bounds checking and input validation
template<typename T>
class Stack {
private:
    std::vector<T> data;
    // Maximum size to prevent excessive memory usage
    static const size_t MAX_SIZE = 1000000;

public:
    Stack() {
        // Reserve initial capacity to reduce reallocations
        data.reserve(16);
    }

    // Push operation with overflow protection
    void push(const T& value) {
        // Check if adding element would exceed maximum size
        if (data.size() >= MAX_SIZE) {
            throw std::overflow_error("Stack overflow: maximum size reached");
        }
        data.push_back(value);
    }

    // Pop operation with underflow protection
    T pop() {
        // Validate stack is not empty before popping
        if (data.empty()) {
            throw std::underflow_error("Stack underflow: cannot pop from empty stack");
        }
        T value = data.back();
        data.pop_back();
        return value;
    }

    // Peek operation with bounds checking
    T peek() const {
        // Validate stack is not empty before peeking
        if (data.empty()) {
            throw std::underflow_error("Stack underflow: cannot peek empty stack");
        }
        return data.back();
    }

    // Check if stack is empty
    bool isEmpty() const {
        return data.empty();
    }

    // Get current size with overflow protection
    size_t size() const {
        return data.size();
    }
};

int main() {
    try {
        // Test case 1: Basic push and pop operations
        std::cout << "Test 1: Basic operations" << std::endl;
        Stack<int> stack1;
        stack1.push(10);
        stack1.push(20);
        stack1.push(30);
        std::cout << "Peek: " << stack1.peek() << std::endl;
        std::cout << "Pop: " << stack1.pop() << std::endl;
        std::cout << "Pop: " << stack1.pop() << std::endl;
        std::cout << "Size: " << stack1.size() << std::endl;
        std::cout << std::endl;

        // Test case 2: Empty stack operations
        std::cout << "Test 2: Empty stack check" << std::endl;
        Stack<int> stack2;
        std::cout << "Is empty: " << (stack2.isEmpty() ? "true" : "false") << std::endl;
        stack2.push(5);
        std::cout << "Is empty after push: " << (stack2.isEmpty() ? "true" : "false") << std::endl;
        stack2.pop();
        std::cout << "Is empty after pop: " << (stack2.isEmpty() ? "true" : "false") << std::endl;
        std::cout << std::endl;

        // Test case 3: Underflow protection
        std::cout << "Test 3: Underflow protection" << std::endl;
        Stack<int> stack3;
        try {
            stack3.pop(); // Should throw exception
        } catch (const std::underflow_error& e) {
            std::cout << "Caught exception: " << e.what() << std::endl;
        }
        std::cout << std::endl;

        // Test case 4: String stack
        std::cout << "Test 4: String stack" << std::endl;
        Stack<std::string> stack4;
        stack4.push("Hello");
        stack4.push("World");
        stack4.push("Stack");
        std::cout << "Peek: " << stack4.peek() << std::endl;
        std::cout << "Pop: " << stack4.pop() << std::endl;
        std::cout << "Peek: " << stack4.peek() << std::endl;
        std::cout << std::endl;

        // Test case 5: Multiple operations
        std::cout << "Test 5: Multiple operations" << std::endl;
        Stack<int> stack5;
        for (int i = 1; i <= 5; ++i) {
            stack5.push(i * 10);
        }
        std::cout << "Size after pushes: " << stack5.size() << std::endl;
        while (!stack5.isEmpty()) {
            std::cout << "Pop: " << stack5.pop() << std::endl;
        }
        std::cout << "Final size: " << stack5.size() << std::endl;

    } catch (const std::exception& e) {
        // Catch any unexpected exceptions with safe error reporting
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
