
#include <iostream>
#include <vector>
#include <stdexcept>

class Task149 {
private:
    std::vector<int> queue;
    
public:
    Task149() {}
    
    void enqueue(int value) {
        queue.push_back(value);
    }
    
    int dequeue() {
        if (queue.empty()) {
            throw std::runtime_error("Queue is empty");
        }
        int value = queue[0];
        queue.erase(queue.begin());
        return value;
    }
    
    int peek() {
        if (queue.empty()) {
            throw std::runtime_error("Queue is empty");
        }
        return queue[0];
    }
    
    bool isEmpty() {
        return queue.empty();
    }
    
    int size() {
        return queue.size();
    }
};

int main() {
    // Test Case 1: Basic enqueue and dequeue
    std::cout << "Test Case 1: Basic enqueue and dequeue" << std::endl;
    Task149 q1;
    q1.enqueue(10);
    q1.enqueue(20);
    q1.enqueue(30);
    std::cout << "Dequeue: " << q1.dequeue() << std::endl; // 10
    std::cout << "Dequeue: " << q1.dequeue() << std::endl; // 20
    std::cout << std::endl;
    
    // Test Case 2: Peek operation
    std::cout << "Test Case 2: Peek operation" << std::endl;
    Task149 q2;
    q2.enqueue(5);
    q2.enqueue(15);
    std::cout << "Peek: " << q2.peek() << std::endl; // 5
    std::cout << "Peek: " << q2.peek() << std::endl; // 5
    std::cout << "Dequeue: " << q2.dequeue() << std::endl; // 5
    std::cout << "Peek: " << q2.peek() << std::endl; // 15
    std::cout << std::endl;
    
    // Test Case 3: Empty queue operations
    std::cout << "Test Case 3: Empty queue operations" << std::endl;
    Task149 q3;
    try {
        std::cout << "Dequeue from empty: " << q3.dequeue() << std::endl;
    } catch (const std::runtime_error& e) {
        std::cout << "Dequeue from empty: " << e.what() << std::endl;
    }
    try {
        std::cout << "Peek from empty: " << q3.peek() << std::endl;
    } catch (const std::runtime_error& e) {
        std::cout << "Peek from empty: " << e.what() << std::endl;
    }
    q3.enqueue(100);
    std::cout << "Dequeue: " << q3.dequeue() << std::endl; // 100
    try {
        std::cout << "Dequeue from empty: " << q3.dequeue() << std::endl;
    } catch (const std::runtime_error& e) {
        std::cout << "Dequeue from empty: " << e.what() << std::endl;
    }
    std::cout << std::endl;
    
    // Test Case 4: Multiple operations
    std::cout << "Test Case 4: Multiple operations" << std::endl;
    Task149 q4;
    q4.enqueue(1);
    q4.enqueue(2);
    std::cout << "Dequeue: " << q4.dequeue() << std::endl; // 1
    q4.enqueue(3);
    q4.enqueue(4);
    std::cout << "Peek: " << q4.peek() << std::endl; // 2
    std::cout << "Dequeue: " << q4.dequeue() << std::endl; // 2
    std::cout << "Dequeue: " << q4.dequeue() << std::endl; // 3
    std::cout << std::endl;
    
    // Test Case 5: Size check
    std::cout << "Test Case 5: Size check" << std::endl;
    Task149 q5;
    std::cout << "Size: " << q5.size() << std::endl; // 0
    q5.enqueue(50);
    q5.enqueue(60);
    q5.enqueue(70);
    std::cout << "Size: " << q5.size() << std::endl; // 3
    q5.dequeue();
    std::cout << "Size after dequeue: " << q5.size() << std::endl; // 2
    std::cout << "Is empty: " << (q5.isEmpty() ? "true" : "false") << std::endl; // false
    
    return 0;
}
