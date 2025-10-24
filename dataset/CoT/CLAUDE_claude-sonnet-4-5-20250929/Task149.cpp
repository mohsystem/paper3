
#include <iostream>
#include <stdexcept>
#include <string>

template <typename T>
class Queue {
private:
    struct Node {
        T data;
        Node* next;
        
        Node(const T& data) : data(data), next(nullptr) {}
    };
    
    Node* front;
    Node* rear;
    int size;

public:
    Queue() : front(nullptr), rear(nullptr), size(0) {}
    
    ~Queue() {
        while (!isEmpty()) {
            dequeue();
        }
    }
    
    void enqueue(const T& item) {
        Node* newNode = new Node(item);
        
        if (isEmpty()) {
            front = newNode;
            rear = newNode;
        } else {
            rear->next = newNode;
            rear = newNode;
        }
        size++;
    }
    
    T dequeue() {
        if (isEmpty()) {
            throw std::underflow_error("Queue is empty");
        }
        
        Node* temp = front;
        T data = front->data;
        front = front->next;
        
        if (front == nullptr) {
            rear = nullptr;
        }
        
        delete temp;
        size--;
        return data;
    }
    
    T peek() const {
        if (isEmpty()) {
            throw std::underflow_error("Queue is empty");
        }
        return front->data;
    }
    
    bool isEmpty() const {
        return front == nullptr;
    }
    
    int getSize() const {
        return size;
    }
};

int main() {
    // Test Case 1: Basic enqueue and dequeue operations
    std::cout << "Test Case 1: Basic operations" << std::endl;
    Queue<int> queue1;
    queue1.enqueue(10);
    queue1.enqueue(20);
    queue1.enqueue(30);
    std::cout << "Peek: " << queue1.peek() << std::endl;
    std::cout << "Dequeue: " << queue1.dequeue() << std::endl;
    std::cout << "Peek after dequeue: " << queue1.peek() << std::endl;
    std::cout << std::endl;
    
    // Test Case 2: String queue
    std::cout << "Test Case 2: String queue" << std::endl;
    Queue<std::string> queue2;
    queue2.enqueue("Hello");
    queue2.enqueue("World");
    queue2.enqueue("C++");
    std::cout << "Size: " << queue2.getSize() << std::endl;
    std::cout << "Dequeue: " << queue2.dequeue() << std::endl;
    std::cout << "Dequeue: " << queue2.dequeue() << std::endl;
    std::cout << std::endl;
    
    // Test Case 3: Empty queue check
    std::cout << "Test Case 3: Empty queue operations" << std::endl;
    Queue<int> queue3;
    std::cout << "Is empty: " << (queue3.isEmpty() ? "true" : "false") << std::endl;
    queue3.enqueue(100);
    std::cout << "Is empty after enqueue: " << (queue3.isEmpty() ? "true" : "false") << std::endl;
    queue3.dequeue();
    std::cout << "Is empty after dequeue: " << (queue3.isEmpty() ? "true" : "false") << std::endl;
    std::cout << std::endl;
    
    // Test Case 4: Multiple operations
    std::cout << "Test Case 4: Multiple operations" << std::endl;
    Queue<int> queue4;
    for (int i = 1; i <= 5; i++) {
        queue4.enqueue(i * 10);
    }
    std::cout << "Size: " << queue4.getSize() << std::endl;
    while (!queue4.isEmpty()) {
        std::cout << "Dequeue: " << queue4.dequeue() << std::endl;
    }
    std::cout << std::endl;
    
    // Test Case 5: Error handling
    std::cout << "Test Case 5: Error handling" << std::endl;
    Queue<int> queue5;
    try {
        queue5.dequeue();
    } catch (const std::underflow_error& e) {
        std::cout << "Caught exception: " << e.what() << std::endl;
    }
    
    return 0;
}
