
#include <iostream>
#include <stdexcept>

class Task149 {
private:
    struct Node {
        int data;
        Node* next;
        
        Node(int data) : data(data), next(nullptr) {}
    };
    
    Node* front;
    Node* rear;
    int size;
    
public:
    Task149() : front(nullptr), rear(nullptr), size(0) {}
    
    ~Task149() {
        while (!isEmpty()) {
            dequeue();
        }
    }
    
    void enqueue(int data) {
        Node* newNode = new Node(data);
        if (rear == nullptr) {
            front = rear = newNode;
        } else {
            rear->next = newNode;
            rear = newNode;
        }
        size++;
    }
    
    int dequeue() {
        if (isEmpty()) {
            throw std::runtime_error("Queue is empty");
        }
        int data = front->data;
        Node* temp = front;
        front = front->next;
        if (front == nullptr) {
            rear = nullptr;
        }
        delete temp;
        size--;
        return data;
    }
    
    int peek() {
        if (isEmpty()) {
            throw std::runtime_error("Queue is empty");
        }
        return front->data;
    }
    
    bool isEmpty() {
        return front == nullptr;
    }
    
    int getSize() {
        return size;
    }
};

int main() {
    // Test case 1: Basic enqueue and dequeue
    std::cout << "Test Case 1: Basic enqueue and dequeue" << std::endl;
    Task149 queue1;
    queue1.enqueue(10);
    queue1.enqueue(20);
    queue1.enqueue(30);
    std::cout << "Dequeue: " << queue1.dequeue() << std::endl;
    std::cout << "Peek: " << queue1.peek() << std::endl;
    std::cout << "Size: " << queue1.getSize() << std::endl;
    std::cout << std::endl;
    
    // Test case 2: Enqueue and peek without dequeue
    std::cout << "Test Case 2: Enqueue and peek" << std::endl;
    Task149 queue2;
    queue2.enqueue(5);
    queue2.enqueue(15);
    std::cout << "Peek: " << queue2.peek() << std::endl;
    std::cout << "Peek again: " << queue2.peek() << std::endl;
    std::cout << "Size: " << queue2.getSize() << std::endl;
    std::cout << std::endl;
    
    // Test case 3: Multiple operations
    std::cout << "Test Case 3: Multiple operations" << std::endl;
    Task149 queue3;
    queue3.enqueue(1);
    queue3.enqueue(2);
    std::cout << "Dequeue: " << queue3.dequeue() << std::endl;
    queue3.enqueue(3);
    queue3.enqueue(4);
    std::cout << "Dequeue: " << queue3.dequeue() << std::endl;
    std::cout << "Peek: " << queue3.peek() << std::endl;
    std::cout << "Size: " << queue3.getSize() << std::endl;
    std::cout << std::endl;
    
    // Test case 4: Empty queue
    std::cout << "Test Case 4: Empty queue check" << std::endl;
    Task149 queue4;
    std::cout << "Is empty: " << (queue4.isEmpty() ? "true" : "false") << std::endl;
    queue4.enqueue(100);
    std::cout << "Is empty: " << (queue4.isEmpty() ? "true" : "false") << std::endl;
    queue4.dequeue();
    std::cout << "Is empty after dequeue: " << (queue4.isEmpty() ? "true" : "false") << std::endl;
    std::cout << std::endl;
    
    // Test case 5: Large number of operations
    std::cout << "Test Case 5: Large number of operations" << std::endl;
    Task149 queue5;
    for (int i = 1; i <= 5; i++) {
        queue5.enqueue(i * 10);
    }
    std::cout << "Size: " << queue5.getSize() << std::endl;
    std::cout << "Dequeue: " << queue5.dequeue() << std::endl;
    std::cout << "Dequeue: " << queue5.dequeue() << std::endl;
    std::cout << "Peek: " << queue5.peek() << std::endl;
    std::cout << "Size: " << queue5.getSize() << std::endl;
    
    return 0;
}
