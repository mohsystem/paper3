#include <iostream>
#include <vector>
#include <optional>
#include <stdexcept>
#include <limits>

class Task149 {
public:
    class IntQueue {
    public:
        IntQueue(size_t initial_capacity = 16)
            : data_(initial_capacity > 0 ? initial_capacity : 16),
              head_(0), tail_(0), size_(0) {}

        bool enqueue(int value) {
            ensure_capacity_for_one_more();
            data_[tail_] = value;
            tail_ = (tail_ + 1) % data_.size();
            ++size_;
            return true;
        }

        std::optional<int> dequeue() {
            if (size_ == 0) {
                return std::nullopt;
            }
            int val = data_[head_];
            data_[head_] = 0;
            head_ = (head_ + 1) % data_.size();
            --size_;
            return val;
        }

        std::optional<int> peek() const {
            if (size_ == 0) return std::nullopt;
            return data_[head_];
        }

        size_t size() const { return size_; }
        bool is_empty() const { return size_ == 0; }

    private:
        void ensure_capacity_for_one_more() {
            if (size_ < data_.size()) return;
            size_t current = data_.size();
            size_t max_cap = static_cast<size_t>(1) << 30;
            size_t proposed = current * 2;
            if (proposed < current || proposed > max_cap) {
                if (current == max_cap) {
                    throw std::runtime_error("Queue too large.");
                }
                proposed = max_cap;
            }
            std::vector<int> new_data(proposed, 0);
            if (head_ <= tail_) {
                std::copy(data_.begin() + head_, data_.begin() + tail_, new_data.begin());
            } else {
                size_t first_part = data_.size() - head_;
                std::copy(data_.begin() + head_, data_.end(), new_data.begin());
                std::copy(data_.begin(), data_.begin() + tail_, new_data.begin() + first_part);
            }
            data_.swap(new_data);
            head_ = 0;
            tail_ = size_;
        }

        std::vector<int> data_;
        size_t head_;
        size_t tail_;
        size_t size_;
    };
};

int main() {
    using IntQueue = Task149::IntQueue;

    IntQueue q;

    // Test 1: Enqueue and Peek
    q.enqueue(1);
    q.enqueue(2);
    q.enqueue(3);
    std::cout << "Test1 Peek: " << (q.peek().has_value() ? std::to_string(*q.peek()) : "null") << "\n";
    auto d1 = q.dequeue();
    std::cout << "Test1 Dequeue: " << (d1.has_value() ? std::to_string(*d1) : "null") << "\n";

    // Test 2: Dequeue remaining and attempt extra dequeue
    auto d2 = q.dequeue();
    std::cout << "Test2 Dequeue: " << (d2.has_value() ? std::to_string(*d2) : "null") << "\n";
    auto d3 = q.dequeue();
    std::cout << "Test2 Dequeue: " << (d3.has_value() ? std::to_string(*d3) : "null") << "\n";
    auto d4 = q.dequeue();
    std::cout << "Test2 Dequeue Empty: " << (d4.has_value() ? std::to_string(*d4) : "null") << "\n";

    // Test 3: Interleaved operations
    q.enqueue(10);
    std::cout << "Test3 Peek: " << (q.peek().has_value() ? std::to_string(*q.peek()) : "null") << "\n";
    q.enqueue(20);
    auto d5 = q.dequeue();
    std::cout << "Test3 Dequeue: " << (d5.has_value() ? std::to_string(*d5) : "null") << "\n";
    std::cout << "Test3 Peek: " << (q.peek().has_value() ? std::to_string(*q.peek()) : "null") << "\n";

    // Test 4: Stress growth
    for (int i = 0; i < 100; ++i) q.enqueue(i);
    std::cout << "Test4 Size after 100 enqueues: " << q.size() << "\n";
    std::cout << "Test4 Peek: " << (q.peek().has_value() ? std::to_string(*q.peek()) : "null") << "\n";
    for (int i = 0; i < 5; ++i) {
        auto dx = q.dequeue();
        std::cout << "Test4 Dequeue: " << (dx.has_value() ? std::to_string(*dx) : "null") << "\n";
    }

    // Test 5: New queue, empty checks
    IntQueue q2;
    std::cout << "Test5 IsEmpty: " << (q2.is_empty() ? "true" : "false") << "\n";
    std::cout << "Test5 Peek Empty: " << (q2.peek().has_value() ? std::to_string(*q2.peek()) : "null") << "\n";
    q2.enqueue(99);
    auto d6 = q2.dequeue();
    std::cout << "Test5 Dequeue: " << (d6.has_value() ? std::to_string(*d6) : "null") << "\n";
    auto d7 = q2.dequeue();
    std::cout << "Test5 Dequeue Empty: " << (d7.has_value() ? std::to_string(*d7) : "null") << "\n";

    return 0;
}