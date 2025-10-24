#include <iostream>
#include <stdexcept>

// Node structure for the linked list
struct Node {
    int data;
    Node* next;
    Node(int d) : data(d), next(nullptr) {}
};

class Queue {
private:
    Node *front, *rear;

public:
    Queue() : front(nullptr), rear(nullptr) {}

    // Destructor to free all allocated memory
    ~Queue() {
        while (front != nullptr) {
            Node* temp = front;
            front = front->next;
            delete temp;
        }
    }

    // Method to add an item to the queue.
    void enqueue(int item) {
        Node* newNode = new Node(item);
        // If queue is empty, then new node is front and rear both
        if (rear == nullptr) {
            front = rear = newNode;
            return;
        }
        // Add the new node at the end of queue and change rear
        rear->next = newNode;
        rear = newNode;
    }

    // Method to remove an item from queue.
    int dequeue() {
        // If queue is empty, throw an exception
        if (front == nullptr) {
            throw std::runtime_error("Queue is empty. Cannot dequeue.");
        }
        // Store previous front and move front one node ahead
        Node* temp = front;
        int item = temp->data;
        front = front->next;
        // If front becomes NULL, then change rear also as NULL
        if (front == nullptr) {
            rear = nullptr;
        }
        delete temp; // Free the old front node
        return item;
    }

    // Method to get the front of queue.
    int peek() {
        // If queue is empty, throw an exception
        if (front == nullptr) {
            throw std::runtime_error("Queue is empty. Cannot peek.");
        }
        return front->data;
    }

    // Method to check if the queue is empty.
    bool isEmpty() {
        return front == nullptr;
    }
};

int main() {
    std::cout << "C++ Queue Implementation Test" << std::endl;
    Queue queue;

    // Test Case 1: Enqueue elements
    std::cout << "Test Case 1: Enqueue 10, 20, 30" << std::endl;
    queue.enqueue(10);
    queue.enqueue(20);
    queue.enqueue(30);
    std::cout << "Front element is: " << queue.peek() << std::endl;

    // Test Case 2: Dequeue an element
    std::cout << "\nTest Case 2: Dequeue" << std::endl;
    std::cout << "Dequeued element: " << queue.dequeue() << std::endl;
    std::cout << "Front element is now: " << queue.peek() << std::endl;

    // Test Case 3: Enqueue another element
    std::cout << "\nTest Case 3: Enqueue 40" << std::endl;
    queue.enqueue(40);
    std::cout << "Front element is: " << queue.peek() << std::endl;

    // Test Case 4: Dequeue all elements
    std::cout << "\nTest Case 4: Dequeue all elements" << std::endl;
    std::cout << "Dequeued element: " << queue.dequeue() << std::endl; // 20
    std::cout << "Dequeued element: " << queue.dequeue() << std::endl; // 30
    std::cout << "Dequeued element: " << queue.dequeue() << std::endl; // 40
    std::cout << "Is queue empty? " << (queue.isEmpty() ? "true" : "false") << std::endl;

    // Test Case 5: Underflow condition
    std::cout << "\nTest Case 5: Underflow condition" << std::endl;
    try {
        queue.peek();
    } catch (const std::runtime_error& e) {
        std::cerr << "Caught expected exception on peek: " << e.what() << std::endl;
    }
    try {
        queue.dequeue();
    } catch (const std::runtime_error& e) {
        std::cerr << "Caught expected exception on dequeue: " << e.what() << std::endl;
    }

    return 0;
}