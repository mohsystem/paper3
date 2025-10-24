#include <iostream>
#include <vector>
#include <stdexcept>

class Stack {
private:
    std::vector<int> items;

public:
    /**
     * Pushes an item onto the top of this stack.
     * @param item The item to be pushed.
     */
    void push(int item) {
        items.push_back(item);
    }

    /**
     * Removes the item at the top of this stack and returns it.
     * @return The item at the top of the stack.
     * @throws std::out_of_range if the stack is empty.
     */
    int pop() {
        if (isEmpty()) {
            throw std::out_of_range("pop from an empty stack");
        }
        int top_item = items.back();
        items.pop_back();
        return top_item;
    }

    /**
     * Looks at the item at the top of this stack without removing it.
     * @return The item at the top of the stack.
     * @throws std::out_of_range if the stack is empty.
     */
    int peek() {
        if (isEmpty()) {
            throw std::out_of_range("peek from an empty stack");
        }
        return items.back();
    }

    /**
     * Tests if this stack is empty.
     * @return True if the stack is empty, false otherwise.
     */
    bool isEmpty() const {
        return items.empty();
    }

    /**
     * Returns the number of items in this stack.
     * @return The number of items in the stack.
     */
    size_t size() const {
        return items.size();
    }
};

void run_tests() {
    // Test Case 1: Push and Peek
    std::cout << "--- Test Case 1: Push and Peek ---" << std::endl;
    Stack stack1;
    stack1.push(10);
    stack1.push(20);
    stack1.push(30);
    std::cout << "Pushed 10, 20, 30. Top element (peek): " << stack1.peek() << std::endl;
    std::cout << "Stack size: " << stack1.size() << std::endl;

    // Test Case 2: Push and Pop
    std::cout << "\n--- Test Case 2: Push and Pop ---" << std::endl;
    Stack stack2;
    stack2.push(10);
    stack2.push(20);
    stack2.push(30);
    std::cout << "Popped element: " << stack2.pop() << std::endl;
    std::cout << "Top element after pop (peek): " << stack2.peek() << std::endl;
    std::cout << "Stack size: " << stack2.size() << std::endl;

    // Test Case 3: Mixed Operations
    std::cout << "\n--- Test Case 3: Mixed Operations ---" << std::endl;
    Stack stack3;
    stack3.push(10);
    stack3.push(20);
    std::cout << "Popped element: " << stack3.pop() << std::endl;
    stack3.push(30);
    std::cout << "Top element (peek): " << stack3.peek() << std::endl;
    std::cout << "Stack size: " << stack3.size() << std::endl;

    // Test Case 4: Pop from empty stack
    std::cout << "\n--- Test Case 4: Pop from empty stack ---" << std::endl;
    Stack stack4;
    try {
        stack4.pop();
    } catch (const std::out_of_range& e) {
        std::cout << "Successfully caught expected exception: " << e.what() << std::endl;
    }

    // Test Case 5: Peek from empty stack
    std::cout << "\n--- Test Case 5: Peek from empty stack ---" << std::endl;
    Stack stack5;
    try {
        stack5.peek();
    } catch (const std::out_of_range& e) {
        std::cout << "Successfully caught expected exception: " << e.what() << std::endl;
    }
}

int main() {
    run_tests();
    return 0;
}