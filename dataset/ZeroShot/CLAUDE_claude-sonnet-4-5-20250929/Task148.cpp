
#include <iostream>
#include <stdexcept>
using namespace std;

class Stack {
private:
    struct Node {
        int data;
        Node* next;
        Node(int val) : data(val), next(nullptr) {}
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
            throw runtime_error("Cannot pop from empty stack");
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
            throw runtime_error("Cannot peek empty stack");
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
    // Test Case 1: Basic push and pop operations
    cout << "Test Case 1: Basic push and pop" << endl;
    Stack stack1;
    stack1.push(10);
    stack1.push(20);
    stack1.push(30);
    cout << "Peek: " << stack1.peek() << endl;
    cout << "Pop: " << stack1.pop() << endl;
    cout << "Pop: " << stack1.pop() << endl;
    cout << "Size: " << stack1.getSize() << endl;
    cout << endl;
    
    // Test Case 2: Push multiple elements and peek
    cout << "Test Case 2: Multiple pushes and peek" << endl;
    Stack stack2;
    for (int i = 1; i <= 5; i++) {
        stack2.push(i * 10);
    }
    cout << "Peek: " << stack2.peek() << endl;
    cout << "Size: " << stack2.getSize() << endl;
    cout << endl;
    
    // Test Case 3: Pop all elements
    cout << "Test Case 3: Pop all elements" << endl;
    Stack stack3;
    stack3.push(100);
    stack3.push(200);
    stack3.push(300);
    while (!stack3.isEmpty()) {
        cout << "Popped: " << stack3.pop() << endl;
    }
    cout << "Is empty: " << (stack3.isEmpty() ? "true" : "false") << endl;
    cout << endl;
    
    // Test Case 4: Mixed operations
    cout << "Test Case 4: Mixed operations" << endl;
    Stack stack4;
    stack4.push(5);
    stack4.push(15);
    cout << "Peek: " << stack4.peek() << endl;
    stack4.pop();
    stack4.push(25);
    stack4.push(35);
    cout << "Peek: " << stack4.peek() << endl;
    cout << "Size: " << stack4.getSize() << endl;
    cout << endl;
    
    // Test Case 5: Empty stack exception handling
    cout << "Test Case 5: Empty stack exception" << endl;
    Stack stack5;
    try {
        stack5.pop();
    } catch (const runtime_error& e) {
        cout << "Exception caught: " << e.what() << endl;
    }
    try {
        stack5.peek();
    } catch (const runtime_error& e) {
        cout << "Exception caught: " << e.what() << endl;
    }
    
    return 0;
}
