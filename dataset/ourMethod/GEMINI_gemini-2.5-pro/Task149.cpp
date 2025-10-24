#include <iostream>
#include <stdexcept>

class Queue {
private:
    struct Node {
        int data;
        Node* next;

        Node(int d) : data(d), next(nullptr) {}
    };

    Node* front;
    Node* rear;

public:
    Queue() : front(nullptr), rear(nullptr) {}

    ~Queue() {
        while (!isEmpty()) {
            Node* temp = front;
            front = front->next;
            delete temp;
        }
    }

    // Disallow copy constructor and copy assignment to prevent shallow copies
    // and potential double-free errors.
    Queue(const Queue&) = delete;
    Queue& operator=(const Queue&) = delete;

    bool isEmpty() const {
        return front == nullptr;
    }

    void enqueue(int data) {
        Node* newNode = new Node(data);
        if (isEmpty()) {
            front = newNode;
            rear = newNode;
        } else {
            rear->next = newNode;
            rear = newNode;
        }
    }

    int dequeue() {
        if (isEmpty()) {
            throw std::runtime_error("Queue is empty. Cannot dequeue.");
        }
        Node* temp = front;
        int data = temp->data;
        front = front->next;
        if (front == nullptr) {
            rear = nullptr; // The queue is now empty
        }
        delete temp;
        return data;
    }

    int peek() const {
        if (isEmpty()) {
            throw std::runtime_error("Queue is empty. Cannot peek.");
        }
        return front->data;
    }
};

void run_tests() {
    std::cout << "--- C++ Queue Test Cases ---" << std::endl;

    // Test Case 1: Basic enqueue and dequeue
    std::cout << "\n--- Test Case 1: Basic Operations ---" << std::endl;
    Queue q1;
    q1.enqueue(10);
    q1.enqueue(20);
    std::cout << "Dequeued: " << q1.dequeue() << std::endl; // Expected: 10
    std::cout << "Peek: " << q1.peek() << std::endl;       // Expected: 20
    std::cout << "Dequeued: " << q1.dequeue() << std::endl; // Expected: 20
    std::cout << "Is empty: " << (q1.isEmpty() ? "true" : "false") << std::endl; // Expected: true

    // Test Case 2: Dequeue from an empty queue
    std::cout << "\n--- Test Case 2: Dequeue from Empty Queue ---" << std::endl;
    Queue q2;
    try {
        q2.dequeue();
    } catch (const std::runtime_error& e) {
        std::cout << "Caught expected exception: " << e.what() << std::endl;
    }

    // Test Case 3: Peek at an empty queue
    std::cout << "\n--- Test Case 3: Peek at Empty Queue ---" << std::endl;
    try {
        q2.peek();
    } catch (const std::runtime_error& e) {
        std::cout << "Caught expected exception: " << e.what() << std::endl;
    }

    // Test Case 4: Enqueue after emptying the queue
    std::cout << "\n--- Test Case 4: Enqueue after Emptying ---" << std::endl;
    Queue q4;
    q4.enqueue(1);
    q4.enqueue(2);
    q4.dequeue();
    q4.dequeue();
    q4.enqueue(3);
    std::cout << "Peek: " << q4.peek() << std::endl;       // Expected: 3
    std::cout << "Is empty: " << (q4.isEmpty() ? "true" : "false") << std::endl; // Expected: false

    // Test Case 5: Multiple operations
    std::cout << "\n--- Test Case 5: Multiple Operations ---" << std::endl;
    Queue q5;
    q5.enqueue(100);
    q5.enqueue(200);
    q5.enqueue(300);
    std::cout << "Dequeued: " << q5.dequeue() << std::endl; // Expected: 100
    q5.enqueue(400);
    std::cout << "Peek: " << q5.peek() << std::endl;       // Expected: 200
    std::cout << "Dequeued: " << q5.dequeue() << std::endl; // Expected: 200
    std::cout << "Dequeued: " << q5.dequeue() << std::endl; // Expected: 300
    std::cout << "Peek: " << q5.peek() << std::endl;       // Expected: 400
}

int main() {
    run_tests();
    return 0;
}