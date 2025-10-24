#include <iostream>
#include <vector>
#include <string>

struct DequeueResult {
    bool success;
    int value;
};

class IntQueue {
public:
    explicit IntQueue(int capacity)
        : capacity_(capacity), buffer_(), head_(0), tail_(0), size_(0) {
        const int MAX_CAPACITY = 1000000;
        if (capacity < 1 || capacity > MAX_CAPACITY) {
            throw std::invalid_argument("Capacity must be between 1 and 1000000");
        }
        buffer_.assign(static_cast<size_t>(capacity), 0);
    }

    int size() const noexcept { return size_; }
    int capacity() const noexcept { return capacity_; }
    bool isEmpty() const noexcept { return size_ == 0; }
    bool isFull() const noexcept { return size_ == capacity_; }

    bool enqueue(int value) noexcept {
        if (isFull()) {
            return false;
        }
        buffer_[static_cast<size_t>(tail_)] = value;
        tail_ = (tail_ + 1) % capacity_;
        ++size_;
        return true;
    }

    DequeueResult dequeue() noexcept {
        if (isEmpty()) {
            return {false, 0};
        }
        int v = buffer_[static_cast<size_t>(head_)];
        head_ = (head_ + 1) % capacity_;
        --size_;
        return {true, v};
    }

    DequeueResult peek() const noexcept {
        if (isEmpty()) {
            return {false, 0};
        }
        return {true, buffer_[static_cast<size_t>(head_)]};
    }

    std::string toString() const {
        std::string s = "IntQueue{size=" + std::to_string(size_) +
                        ", capacity=" + std::to_string(capacity_) + ", data=[";
        for (int i = 0; i < size_; ++i) {
            int idx = (head_ + i) % capacity_;
            s += std::to_string(buffer_[static_cast<size_t>(idx)]);
            if (i + 1 < size_) s += ", ";
        }
        s += "]}";
        return s;
    }

private:
    int capacity_;
    std::vector<int> buffer_;
    int head_;
    int tail_;
    int size_;
};

static void test1() {
    std::cout << "TEST 1: Basic enqueue/dequeue\n";
    IntQueue q(3);
    std::cout << "enqueue 1 -> " << (q.enqueue(1) ? "true" : "false") << "\n";
    std::cout << "enqueue 2 -> " << (q.enqueue(2) ? "true" : "false") << "\n";
    std::cout << "enqueue 3 -> " << (q.enqueue(3) ? "true" : "false") << "\n";
    std::cout << "queue: " << q.toString() << "\n";
    DequeueResult r;
    r = q.peek();
    std::cout << "peek -> " << (r.success ? ("Result{success=true, value=" + std::to_string(r.value) + "}") : "Result{success=false}") << "\n";
    r = q.dequeue();
    std::cout << "dequeue -> " << (r.success ? ("Result{success=true, value=" + std::to_string(r.value) + "}") : "Result{success=false}") << "\n";
    r = q.dequeue();
    std::cout << "dequeue -> " << (r.success ? ("Result{success=true, value=" + std::to_string(r.value) + "}") : "Result{success=false}") << "\n";
    r = q.dequeue();
    std::cout << "dequeue -> " << (r.success ? ("Result{success=true, value=" + std::to_string(r.value) + "}") : "Result{success=false}") << "\n";
    r = q.dequeue();
    std::cout << "dequeue (empty) -> " << (r.success ? ("Result{success=true, value=" + std::to_string(r.value) + "}") : "Result{success=false}") << "\n";
    std::cout << "queue: " << q.toString() << "\n";
}

static void test2() {
    std::cout << "TEST 2: Overflow handling\n";
    IntQueue q(2);
    std::cout << "enqueue 10 -> " << (q.enqueue(10) ? "true" : "false") << "\n";
    std::cout << "enqueue 20 -> " << (q.enqueue(20) ? "true" : "false") << "\n";
    std::cout << "enqueue 30 (should fail) -> " << (q.enqueue(30) ? "true" : "false") << "\n";
    DequeueResult r = q.peek();
    std::cout << "peek -> " << (r.success ? ("Result{success=true, value=" + std::to_string(r.value) + "}") : "Result{success=false}") << "\n";
    std::cout << "queue: " << q.toString() << "\n";
}

static void test3() {
    std::cout << "TEST 3: Peek on empty\n";
    IntQueue q(5);
    DequeueResult r = q.peek();
    std::cout << "peek (empty) -> " << (r.success ? ("Result{success=true, value=" + std::to_string(r.value) + "}") : "Result{success=false}") << "\n";
    std::cout << "enqueue 7 -> " << (q.enqueue(7) ? "true" : "false") << "\n";
    r = q.peek();
    std::cout << "peek -> " << (r.success ? ("Result{success=true, value=" + std::to_string(r.value) + "}") : "Result{success=false}") << "\n";
    r = q.dequeue();
    std::cout << "dequeue -> " << (r.success ? ("Result{success=true, value=" + std::to_string(r.value) + "}") : "Result{success=false}") << "\n";
    r = q.peek();
    std::cout << "peek (empty) -> " << (r.success ? ("Result{success=true, value=" + std::to_string(r.value) + "}") : "Result{success=false}") << "\n";
}

static void test4() {
    std::cout << "TEST 4: Wrap-around behavior\n";
    IntQueue q(3);
    std::cout << "enqueue 1 -> " << (q.enqueue(1) ? "true" : "false") << "\n";
    std::cout << "enqueue 2 -> " << (q.enqueue(2) ? "true" : "false") << "\n";
    DequeueResult r = q.dequeue();
    std::cout << "dequeue -> " << (r.success ? ("Result{success=true, value=" + std::to_string(r.value) + "}") : "Result{success=false}") << "\n";
    std::cout << "enqueue 3 -> " << (q.enqueue(3) ? "true" : "false") << "\n";
    std::cout << "enqueue 4 -> " << (q.enqueue(4) ? "true" : "false") << "\n";
    std::cout << "isFull -> " << (q.isFull() ? "true" : "false") << "\n";
    std::cout << "queue: " << q.toString() << "\n";
    r = q.dequeue();
    std::cout << "dequeue -> " << (r.success ? ("Result{success=true, value=" + std::to_string(r.value) + "}") : "Result{success=false}") << "\n";
    r = q.dequeue();
    std::cout << "dequeue -> " << (r.success ? ("Result{success=true, value=" + std::to_string(r.value) + "}") : "Result{success=false}") << "\n";
    r = q.dequeue();
    std::cout << "dequeue -> " << (r.success ? ("Result{success=true, value=" + std::to_string(r.value) + "}") : "Result{success=false}") << "\n";
    std::cout << "isEmpty -> " << (q.isEmpty() ? "true" : "false") << "\n";
}

static void test5() {
    std::cout << "TEST 5: Capacity 1 edge case\n";
    IntQueue q(1);
    std::cout << "enqueue 42 -> " << (q.enqueue(42) ? "true" : "false") << "\n";
    std::cout << "enqueue 99 (should fail) -> " << (q.enqueue(99) ? "true" : "false") << "\n";
    DequeueResult r = q.peek();
    std::cout << "peek -> " << (r.success ? ("Result{success=true, value=" + std::to_string(r.value) + "}") : "Result{success=false}") << "\n";
    r = q.dequeue();
    std::cout << "dequeue -> " << (r.success ? ("Result{success=true, value=" + std::to_string(r.value) + "}") : "Result{success=false}") << "\n";
    r = q.dequeue();
    std::cout << "dequeue (empty) -> " << (r.success ? ("Result{success=true, value=" + std::to_string(r.value) + "}") : "Result{success=false}") << "\n";
}

int main() {
    try {
        test1();
        std::cout << "\n";
        test2();
        std::cout << "\n";
        test3();
        std::cout << "\n";
        test4();
        std::cout << "\n";
        test5();
    } catch (const std::exception& ex) {
        std::cout << "Error: " << ex.what() << "\n";
    }
    return 0;
}