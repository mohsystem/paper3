#include <iostream>
#include <utility>

class IntQueue {
private:
    int* data;
    int head;
    int tail;
    int size;
    int capacity;
public:
    explicit IntQueue(int cap) : head(0), tail(0), size(0), capacity(cap) {
        data = new int[capacity];
    }
    ~IntQueue() {
        delete[] data;
    }

    bool enqueue(int x) {
        if (size == capacity) return false;
        data[tail] = x;
        tail = (tail + 1) % capacity;
        size++;
        return true;
    }

    std::pair<bool,int> dequeue() {
        if (size == 0) return {false, 0};
        int val = data[head];
        head = (head + 1) % capacity;
        size--;
        return {true, val};
    }

    std::pair<bool,int> peek() const {
        if (size == 0) return {false, 0};
        return {true, data[head]};
    }

    bool isEmpty() const { return size == 0; }
    bool isFull() const { return size == capacity; }
};

class Task149 {
public:
    static void runTests() {
        // Test 1: Basic enqueue, peek, and dequeue
        IntQueue q1(5);
        q1.enqueue(1);
        q1.enqueue(2);
        q1.enqueue(3);
        auto p1 = q1.peek();
        std::cout << "Test1 Peek: " << (p1.first ? std::to_string(p1.second) : std::string("null")) << "\n";
        auto d1 = q1.dequeue();
        std::cout << "Test1 Dequeue: " << (d1.first ? std::to_string(d1.second) : std::string("null")) << "\n";
        auto p1b = q1.peek();
        std::cout << "Test1 Peek after dequeue: " << (p1b.first ? std::to_string(p1b.second) : std::string("null")) << "\n";

        // Test 2: Peek on empty queue
        IntQueue q2(3);
        auto p2 = q2.peek();
        std::cout << "Test2 Peek on empty: " << (p2.first ? std::to_string(p2.second) : std::string("null")) << "\n";

        // Test 3: Dequeue on empty queue
        IntQueue q3(2);
        auto d3 = q3.dequeue();
        std::cout << "Test3 Dequeue on empty: " << (d3.first ? std::to_string(d3.second) : std::string("null")) << "\n";

        // Test 4: Fill to capacity and attempt extra enqueue
        IntQueue q4(3);
        std::cout << "Test4 Enqueue 1: " << (q4.enqueue(1) ? "true" : "false") << "\n";
        std::cout << "Test4 Enqueue 2: " << (q4.enqueue(2) ? "true" : "false") << "\n";
        std::cout << "Test4 Enqueue 3: " << (q4.enqueue(3) ? "true" : "false") << "\n";
        std::cout << "Test4 Enqueue 4 (should fail): " << (q4.enqueue(4) ? "true" : "false") << "\n";

        // Test 5: Wrap-around behavior
        IntQueue q5(3);
        q5.enqueue(10);
        q5.enqueue(20);
        q5.enqueue(30);
        auto d5a = q5.dequeue();
        std::cout << "Test5 Dequeue: " << (d5a.first ? std::to_string(d5a.second) : std::string("null")) << "\n";
        auto d5b = q5.dequeue();
        std::cout << "Test5 Dequeue: " << (d5b.first ? std::to_string(d5b.second) : std::string("null")) << "\n";
        std::cout << "Test5 Enqueue 40: " << (q5.enqueue(40) ? "true" : "false") << "\n";
        std::cout << "Test5 Enqueue 50: " << (q5.enqueue(50) ? "true" : "false") << "\n";
        auto d5c = q5.dequeue();
        std::cout << "Test5 Dequeue: " << (d5c.first ? std::to_string(d5c.second) : std::string("null")) << "\n";
        auto d5d = q5.dequeue();
        std::cout << "Test5 Dequeue: " << (d5d.first ? std::to_string(d5d.second) : std::string("null")) << "\n";
        auto d5e = q5.dequeue();
        std::cout << "Test5 Dequeue: " << (d5e.first ? std::to_string(d5e.second) : std::string("null")) << "\n";
        auto d5f = q5.dequeue();
        std::cout << "Test5 Dequeue on empty: " << (d5f.first ? std::to_string(d5f.second) : std::string("null")) << "\n";
    }
};

int main() {
    Task149::runTests();
    return 0;
}