#include <iostream>
#include <queue>
#include <stdexcept>

class Queue {
private:
    std::queue<int> items;

public:
    // Enqueue: Add an element to the end of the queue
    void enqueue(int item) {
        items.push(item);
    }

    // Dequeue: Remove and return the element from the front of the queue
    int dequeue() {
        if (items.empty()) {
            throw std::runtime_error("Queue is empty");
        }
        int front_item = items.front();
        items.pop();
        return front_item;
    }

    // Peek: Return the element at the front of the queue without removing it
    int peek() {
        if (items.empty()) {
            throw std::runtime_error("Queue is empty");
        }
        return items.front();
    }

    // Check if the queue is empty
    bool isEmpty() {
        return items.empty();
    }
};

int main() {
    Queue q;

    std::cout << "--- Test Case 1: Enqueue and Peek ---" << std::endl;
    q.enqueue(10);
    q.enqueue(20);
    q.enqueue(30);
    std::cout << "Peek: " << q.peek() << std::endl; // Expected: 10

    std::cout << "\n--- Test Case 2: Dequeue and Peek ---" << std::endl;
    std::cout << "Dequeued: " << q.dequeue() << std::endl; // Expected: 10
    std::cout << "Peek: " << q.peek() << std::endl; // Expected: 20

    std::cout << "\n--- Test Case 3: Dequeue multiple ---" << std::endl;
    std::cout << "Dequeued: " << q.dequeue() << std::endl; // Expected: 20
    std::cout << "Dequeued: " << q.dequeue() << std::endl; // Expected: 30

    std::cout << "\n--- Test Case 4: Operations on empty queue ---" << std::endl;
    try {
        std::cout << "Peek: " << q.peek() << std::endl;
    } catch (const std::runtime_error& e) {
        std::cout << "Peek error: " << e.what() << std::endl; // Expected
    }
    try {
        std::cout << "Dequeued: " << q.dequeue() << std::endl;
    } catch (const std::runtime_error& e) {
        std::cout << "Dequeue error: " << e.what() << std::endl; // Expected
    }

    std::cout << "\n--- Test Case 5: Enqueue after empty ---" << std::endl;
    q.enqueue(40);
    q.enqueue(50);
    std::cout << "Peek: " << q.peek() << std::endl; // Expected: 40
    std::cout << "Dequeued: " << q.dequeue() << std::endl; // Expected: 40
    std::cout << "Peek: " << q.peek() << std::endl; // Expected: 50

    return 0;
}