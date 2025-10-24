#include <iostream>
#include <climits> // For INT_MIN

class Stack {
private:
    int* arr;
    int top;
    int capacity;

public:
    // Constructor to initialize stack
    Stack(int size) {
        if (size <= 0) {
            std::cerr << "Error: Stack size must be positive." << std::endl;
            exit(EXIT_FAILURE);
        }
        capacity = size;
        arr = new int[capacity];
        top = -1; // Stack is initially empty
    }

    // Destructor to free memory
    ~Stack() {
        delete[] arr;
    }

    // Check if the stack is full
    bool isFull() {
        return top == capacity - 1;
    }

    // Check if the stack is empty
    bool isEmpty() {
        return top == -1;
    }

    // Push an item onto the top of the stack
    void push(int value) {
        if (isFull()) {
            std::cerr << "Error: Stack overflow. Cannot push " << value << std::endl;
            return;
        }
        arr[++top] = value;
        std::cout << "Pushed " << value << " to stack." << std::endl;
    }

    // Pop an item from the top of the stack
    int pop() {
        if (isEmpty()) {
            std::cerr << "Error: Stack underflow. Cannot pop." << std::endl;
            return INT_MIN; // Return a sentinel value for error
        }
        int value = arr[top--];
        std::cout << "Popped " << value << " from stack." << std::endl;
        return value;
    }

    // Peek at the top item of the stack without removing it
    int peek() {
        if (isEmpty()) {
            std::cerr << "Error: Stack is empty. Cannot peek." << std::endl;
            return INT_MIN; // Return a sentinel value for error
        }
        return arr[top];
    }
};

int main() {
    std::cout << "Initializing a stack with capacity 5." << std::endl;
    Stack stack(5);

    // Test Case 1: Push items and peek
    std::cout << "\n--- Test Case 1: Push and Peek ---" << std::endl;
    stack.push(10);
    stack.push(20);
    stack.push(30);
    std::cout << "Top element is (peek): " << stack.peek() << std::endl;
    
    // Test Case 2: Pop all items
    std::cout << "\n--- Test Case 2: Pop All Items ---" << std::endl;
    stack.pop();
    stack.pop();
    stack.pop();

    // Test Case 3: Pop from an empty stack (underflow)
    std::cout << "\n--- Test Case 3: Pop from Empty Stack ---" << std::endl;
    std::cout << "Is stack empty? " << (stack.isEmpty() ? "Yes" : "No") << std::endl;
    stack.pop(); // This should cause an underflow error

    // Test Case 4: Push until the stack is full
    std::cout << "\n--- Test Case 4: Fill the Stack ---" << std::endl;
    stack.push(11);
    stack.push(22);
    stack.push(33);
    stack.push(44);
    stack.push(55);
    std::cout << "Is stack full? " << (stack.isFull() ? "Yes" : "No") << std::endl;

    // Test Case 5: Push to a full stack (overflow)
    std::cout << "\n--- Test Case 5: Push to Full Stack ---" << std::endl;
    stack.push(66); // This should cause an overflow error
    std::cout << "Top element is (peek): " << stack.peek() << std::endl;

    return 0;
}