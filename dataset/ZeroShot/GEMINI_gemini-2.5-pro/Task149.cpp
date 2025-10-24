#include <iostream>
#include <stdexcept>

class Task149 {
private:
    struct Node {
        int data;
        Node* next;

        Node(int d) : data(d), next(nullptr) {}
    };

    Node* front;
    Node* rear;

public:
    Task149() : front(nullptr), rear(nullptr) {}

    // Destructor to free all allocated memory
    ~Task149() {
        while (!isEmpty()) {
            dequeue();
        }
    }

    bool isEmpty() const {
        return front == nullptr;
    }

    void enqueue(int data) {
        Node* newNode = new Node(data);
        if (isEmpty()) {
            front = newNode;
        } else {
            rear->next = newNode;
        }
        rear = newNode;
    }

    int dequeue() {
        if (isEmpty()) {
            throw std::runtime_error("Queue is empty. Cannot dequeue.");
        }
        Node* temp = front;
        int data = temp->data;
        front = front->next;
        if (front == nullptr) {
            rear = nullptr;
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

int main() {
    Task149 queue;

    // Test Case 1: Enqueue elements
    std::cout << "Test Case 1: Enqueue 10, 20, 30" << std::endl;
    queue.enqueue(10);
    queue.enqueue(20);
    queue.enqueue(30);
    std::cout << "Enqueue successful." << std::endl;
    std::cout << "--------------------" << std::endl;

    // Test Case 2: Peek at the front element
    std::cout << "Test Case 2: Peek front element" << std::endl;
    std::cout << "Front element is: " << queue.peek() << std::endl; // Expected: 10
    std::cout << "--------------------" << std::endl;

    // Test Case 3: Dequeue an element
    std::cout << "Test Case 3: Dequeue an element" << std::endl;
    std::cout << "Dequeued element: " << queue.dequeue() << std::endl; // Expected: 10
    std::cout << "Front element after dequeue is: " << queue.peek() << std::endl; // Expected: 20
    std::cout << "--------------------" << std::endl;

    // Test Case 4: Dequeue all elements
    std::cout << "Test Case 4: Dequeue all elements" << std::endl;
    std::cout << "Dequeued: " << queue.dequeue() << std::endl; // Expected: 20
    std::cout << "Dequeued: " << queue.dequeue() << std::endl; // Expected: 30
    std::cout << "Is queue empty? " << (queue.isEmpty() ? "true" : "false") << std::endl; // Expected: true
    std::cout << "--------------------" << std::endl;

    // Test Case 5: Operations on an empty queue (handle exceptions)
    std::cout << "Test Case 5: Operations on empty queue" << std::endl;
    try {
        std::cout << "Attempting to peek..." << std::endl;
        queue.peek();
    } catch (const std::runtime_error& e) {
        std::cerr << "Caught expected exception: " << e.what() << std::endl;
    }
    try {
        std::cout << "Attempting to dequeue..." << std::endl;
        queue.dequeue();
    } catch (const std::runtime_error& e) {
        std::cerr << "Caught expected exception: " << e.what() << std::endl;
    }
    std::cout << "--------------------" << std::endl;

    return 0;
}