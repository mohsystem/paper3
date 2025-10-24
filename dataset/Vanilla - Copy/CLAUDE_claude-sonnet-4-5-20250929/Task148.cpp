
#include <iostream>
#include <stdexcept>

class Stack {
private:
    struct Node {
        int data;
        Node* next;
        
        Node(int data) : data(data), next(nullptr) {}
    };
    
    Node* top;
    int size;
    
public:
    Stack() : top(nullptr), size(0) {}
    
    ~Stack() {
        while (!isEmpty()) {
            pop();
        }
    }
    
    void push(int data) {
        Node* newNode = new Node(data);
        newNode->next = top;
        top = newNode;
        size++;
    }
    
    int pop() {
        if (isEmpty()) {
            throw std::runtime_error("Pop from empty stack");
        }
        int data = top->data;
        Node* temp = top;
        top = top->next;
        delete temp;
        size--;
        return data;
    }
    
    int peek() {
        if (isEmpty()) {
            throw std::runtime_error("Peek from empty stack");
        }
        return top->data;
    }
    
    bool isEmpty() {
        return top == nullptr;
    }
    
    int getSize() {
        return size;
    }
};

int main() {
    // Test Case 1: Basic push and peek
    std::cout << "Test Case 1: Basic push and peek" << std::endl;
    Stack stack1;
    stack1.push(10);
    stack1.push(20);
    stack1.push(30);
    std::cout << "Peek: " << stack1.peek() << std::endl;
    std::cout << "Size: " << stack1.getSize() << std::endl;
    std::cout << std::endl;
    
    // Test Case 2: Pop operations
    std::cout << "Test Case 2: Pop operations" << std::endl;
    Stack stack2;
    stack2.push(5);
    stack2.push(15);
    stack2.push(25);
    std::cout << "Pop: " << stack2.pop() << std::endl;
    std::cout << "Pop: " << stack2.pop() << std::endl;
    std::cout << "Peek: " << stack2.peek() << std::endl;
    std::cout << "Size: " << stack2.getSize() << std::endl;
    std::cout << std::endl;
    
    // Test Case 3: Push and pop multiple elements
    std::cout << "Test Case 3: Push and pop multiple elements" << std::endl;
    Stack stack3;
    for (int i = 1; i <= 5; i++) {
        stack3.push(i * 10);
    }
    std::cout << "Peek: " << stack3.peek() << std::endl;
    while (!stack3.isEmpty()) {
        std::cout << "Pop: " << stack3.pop() << std::endl;
    }
    std::cout << "Is Empty: " << (stack3.isEmpty() ? "true" : "false") << std::endl;
    std::cout << std::endl;
    
    // Test Case 4: Single element operations
    std::cout << "Test Case 4: Single element operations" << std::endl;
    Stack stack4;
    stack4.push(100);
    std::cout << "Peek: " << stack4.peek() << std::endl;
    std::cout << "Pop: " << stack4.pop() << std::endl;
    std::cout << "Is Empty: " << (stack4.isEmpty() ? "true" : "false") << std::endl;
    std::cout << std::endl;
    
    // Test Case 5: Mixed operations
    std::cout << "Test Case 5: Mixed operations" << std::endl;
    Stack stack5;
    stack5.push(7);
    stack5.push(14);
    std::cout << "Peek: " << stack5.peek() << std::endl;
    stack5.push(21);
    std::cout << "Pop: " << stack5.pop() << std::endl;
    stack5.push(28);
    std::cout << "Peek: " << stack5.peek() << std::endl;
    std::cout << "Size: " << stack5.getSize() << std::endl;
    
    return 0;
}
