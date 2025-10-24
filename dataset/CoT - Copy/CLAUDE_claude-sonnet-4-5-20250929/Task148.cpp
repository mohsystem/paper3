
#include <iostream>
#include <stdexcept>
#include <cstring>

class Task148 {
private:
    static const int DEFAULT_CAPACITY = 10;
    int* stackArray;
    int top;
    int capacity;

    void resize() {
        int newCapacity = capacity * 2;
        int* newArray = new int[newCapacity];
        std::memcpy(newArray, stackArray, capacity * sizeof(int));
        delete[] stackArray;
        stackArray = newArray;
        capacity = newCapacity;
    }

public:
    Task148(int cap = DEFAULT_CAPACITY) : top(-1), capacity(cap) {
        if (capacity <= 0) {
            throw std::invalid_argument("Capacity must be positive");
        }
        stackArray = new int[capacity];
    }

    ~Task148() {
        delete[] stackArray;
    }

    void push(int value) {
        if (isFull()) {
            resize();
        }
        stackArray[++top] = value;
    }

    int pop() {
        if (isEmpty()) {
            throw std::underflow_error("Cannot pop from empty stack");
        }
        return stackArray[top--];
    }

    int peek() {
        if (isEmpty()) {
            throw std::underflow_error("Cannot peek empty stack");
        }
        return stackArray[top];
    }

    bool isEmpty() const {
        return top == -1;
    }

    bool isFull() const {
        return top == capacity - 1;
    }

    int size() const {
        return top + 1;
    }
};

int main() {
    std::cout << "=== Test Case 1: Basic Push and Pop ===" << std::endl;
    Task148 stack1;
    stack1.push(10);
    stack1.push(20);
    stack1.push(30);
    std::cout << "Peek: " << stack1.peek() << std::endl;
    std::cout << "Pop: " << stack1.pop() << std::endl;
    std::cout << "Pop: " << stack1.pop() << std::endl;
    std::cout << "Size: " << stack1.size() << std::endl;

    std::cout << "\\n=== Test Case 2: Empty Stack Check ===" << std::endl;
    Task148 stack2;
    std::cout << "Is Empty: " << (stack2.isEmpty() ? "true" : "false") << std::endl;
    stack2.push(5);
    std::cout << "Is Empty: " << (stack2.isEmpty() ? "true" : "false") << std::endl;
    stack2.pop();
    std::cout << "Is Empty after pop: " << (stack2.isEmpty() ? "true" : "false") << std::endl;

    std::cout << "\\n=== Test Case 3: Multiple Operations ===" << std::endl;
    Task148 stack3;
    for (int i = 1; i <= 5; i++) {
        stack3.push(i * 10);
    }
    std::cout << "Stack size: " << stack3.size() << std::endl;
    std::cout << "Peek: " << stack3.peek() << std::endl;
    while (!stack3.isEmpty()) {
        std::cout << "Pop: " << stack3.pop() << std::endl;
    }

    std::cout << "\\n=== Test Case 4: Stack Resize ===" << std::endl;
    Task148 stack4(2);
    stack4.push(1);
    stack4.push(2);
    stack4.push(3);
    stack4.push(4);
    std::cout << "Size after resize: " << stack4.size() << std::endl;
    std::cout << "Peek: " << stack4.peek() << std::endl;

    std::cout << "\\n=== Test Case 5: Error Handling ===" << std::endl;
    Task148 stack5;
    try {
        stack5.pop();
    } catch (const std::underflow_error& e) {
        std::cout << "Caught exception: " << e.what() << std::endl;
    }
    try {
        stack5.peek();
    } catch (const std::underflow_error& e) {
        std::cout << "Caught exception: " << e.what() << std::endl;
    }

    return 0;
}
