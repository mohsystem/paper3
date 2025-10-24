#include <iostream>
#include <climits>

class Stack {
private:
    int *arr;
    int top;
    int capacity;

public:
    Stack(int size) {
        capacity = size;
        arr = new int[capacity];
        top = -1;
    }

    ~Stack() {
        delete[] arr;
    }

    void push(int x) {
        if (isFull()) {
            std::cout << "Stack Overflow" << std::endl;
            return;
        }
        std::cout << "Pushing " << x << std::endl;
        arr[++top] = x;
    }

    int pop() {
        if (isEmpty()) {
            std::cout << "Stack Underflow" << std::endl;
            return INT_MIN;
        }
        int item = arr[top--];
        std::cout << "Popping " << item << std::endl;
        return item;
    }

    int peek() {
        if (isEmpty()) {
            std::cout << "Stack is empty" << std::endl;
            return INT_MIN;
        }
        return arr[top];
    }

    bool isFull() {
        return top == capacity - 1;
    }

    bool isEmpty() {
        return top == -1;
    }
};

int main() {
    std::cout << "--- CPP Stack Test ---" << std::endl;
    Stack stack(3);

    // Test Case 1: Push elements
    stack.push(10);
    stack.push(20);
    stack.push(30);

    // Test Case 2: Peek at the top element
    std::cout << "Top element is: " << stack.peek() << std::endl;

    // Test Case 3: Pop an element
    stack.pop();
    std::cout << "Top element after pop is: " << stack.peek() << std::endl;

    // Test Case 4: Push to a full stack (Overflow)
    stack.push(40);
    stack.push(50);

    // Test Case 5: Pop all elements and check for underflow
    stack.pop();
    stack.pop();
    stack.pop();
    stack.pop();

    return 0;
}