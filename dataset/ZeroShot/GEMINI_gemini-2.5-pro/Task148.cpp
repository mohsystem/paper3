#include <iostream>
#include <vector>
#include <stdexcept>

/**
 * @class Stack
 * @brief A secure Stack implementation for integers.
 * It uses std::vector, which dynamically resizes, preventing overflow.
 * It explicitly checks for underflow on pop and peek operations.
 */
class Stack {
private:
    std::vector<int> stack_vec;

public:
    /**
     * @brief Pushes an element onto the top of the stack.
     * @param item The integer element to be pushed.
     */
    void push(int item) {
        stack_vec.push_back(item);
    }

    /**
     * @brief Removes and returns the element at the top of the stack.
     * @return The element at the top of the stack.
     * @throws std::out_of_range if the stack is empty.
     */
    int pop() {
        if (isEmpty()) {
            // Secure: Prevent stack underflow.
            throw std::out_of_range("pop from an empty stack");
        }
        int top_item = stack_vec.back();
        stack_vec.pop_back();
        return top_item;
    }

    /**
     * @brief Returns the element at the top of the stack without removing it.
     * @return The element at the top of the stack.
     * @throws std::out_of_range if the stack is empty.
     */
    int peek() {
        if (isEmpty()) {
            // Secure: Prevent stack underflow.
            throw std::out_of_range("peek from an empty stack");
        }
        return stack_vec.back();
    }

    /**
     * @brief Tests if this stack is empty.
     * @return true if the stack is empty, false otherwise.
     */
    bool isEmpty() const {
        return stack_vec.empty();
    }
};

/**
 * @brief Main function with test cases for the Stack implementation.
 */
int main() {
    std::cout << "C++ Stack Test Cases:" << std::endl;
    
    // Test Case 1: Basic push and pop
    std::cout << "\n--- Test Case 1: Basic Operations ---" << std::endl;
    Stack s1;
    s1.push(10);
    s1.push(20);
    std::cout << "Popped: " << s1.pop() << std::endl; // 20
    std::cout << "Peeked: " << s1.peek() << std::endl; // 10
    std::cout << "Popped: " << s1.pop() << std::endl; // 10
    
    // Test Case 2: Popping from an empty stack
    std::cout << "\n--- Test Case 2: Pop from Empty Stack ---" << std::endl;
    Stack s2;
    try {
        s2.pop();
    } catch (const std::out_of_range& e) {
        std::cerr << "Caught expected exception: " << e.what() << std::endl;
    }

    // Test Case 3: Peeking from an empty stack
    std::cout << "\n--- Test Case 3: Peek from Empty Stack ---" << std::endl;
    Stack s3;
    try {
        s3.peek();
    } catch (const std::out_of_range& e) {
        std::cerr << "Caught expected exception: " << e.what() << std::endl;
    }

    // Test Case 4: A sequence of operations
    std::cout << "\n--- Test Case 4: Sequence of Operations ---" << std::endl;
    Stack s4;
    s4.push(1);
    s4.push(2);
    s4.push(3);
    std::cout << "Popped: " << s4.pop() << std::endl; // 3
    s4.push(4);
    std::cout << "Peeked: " << s4.peek() << std::endl; // 4
    std::cout << "Popped: " << s4.pop() << std::endl; // 4
    std::cout << "Popped: " << s4.pop() << std::endl; // 2

    // Test Case 5: Check isEmpty
    std::cout << "\n--- Test Case 5: isEmpty Check ---" << std::endl;
    Stack s5;
    std::cout << "Is stack empty? " << (s5.isEmpty() ? "true" : "false") << std::endl; // true
    s5.push(100);
    std::cout << "Is stack empty? " << (s5.isEmpty() ? "true" : "false") << std::endl; // false
    s5.pop();
    std::cout << "Is stack empty? " << (s5.isEmpty() ? "true" : "false") << std::endl; // true

    return 0;
}