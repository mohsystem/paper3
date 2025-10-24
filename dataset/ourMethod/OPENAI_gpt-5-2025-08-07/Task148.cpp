#include <iostream>
#include <vector>
#include <stdexcept>
#include <cstddef>

struct Result {
    bool ok;
    int value;
};

class IntStack {
public:
    explicit IntStack(std::size_t capacity) : data_(capacity, 0), top_(0) {
        if (capacity == 0 || capacity > 1000000u) {
            throw std::invalid_argument("Invalid capacity");
        }
    }

    bool push(int value) {
        if (top_ >= data_.size()) {
            return false;
        }
        data_[top_++] = value;
        return true;
    }

    Result pop() {
        if (top_ == 0) {
            return {false, 0};
        }
        int v = data_[--top_];
        return {true, v};
    }

    Result peek() const {
        if (top_ == 0) {
            return {false, 0};
        }
        return {true, data_[top_ - 1]};
    }

    std::size_t size() const { return top_; }
    bool isEmpty() const { return top_ == 0; }
    std::size_t capacity() const { return data_.size(); }

private:
    std::vector<int> data_;
    std::size_t top_;
};

int main() {
    try {
        // Test 1: Basic push/pop/peek
        IntStack s1(3);
        std::cout << "T1 push 10: " << (s1.push(10) ? "true" : "false") << "\n";
        std::cout << "T1 push 20: " << (s1.push(20) ? "true" : "false") << "\n";
        Result r = s1.peek();
        std::cout << "T1 peek: " << (r.ok ? std::to_string(r.value) : std::string("None")) << "\n";
        r = s1.pop();
        std::cout << "T1 pop: " << (r.ok ? std::to_string(r.value) : std::string("None")) << "\n";
        r = s1.pop();
        std::cout << "T1 pop: " << (r.ok ? std::to_string(r.value) : std::string("None")) << "\n";
        r = s1.pop();
        std::cout << "T1 pop empty: " << (r.ok ? std::to_string(r.value) : std::string("None")) << "\n";

        // Test 2: Overflow behavior
        IntStack s2(1);
        std::cout << "T2 push 5: " << (s2.push(5) ? "true" : "false") << "\n";
        std::cout << "T2 push 6 overflow: " << (s2.push(6) ? "true" : "false") << "\n";
        r = s2.peek();
        std::cout << "T2 peek: " << (r.ok ? std::to_string(r.value) : std::string("None")) << "\n";
        std::cout << "T2 size: " << s2.size() << "\n";

        // Test 3: Empty stack operations
        IntStack s3(2);
        r = s3.peek();
        std::cout << "T3 empty peek: " << (r.ok ? std::to_string(r.value) : std::string("None")) << "\n";
        r = s3.pop();
        std::cout << "T3 empty pop: " << (r.ok ? std::to_string(r.value) : std::string("None")) << "\n";
        std::cout << "T3 isEmpty: " << (s3.isEmpty() ? "true" : "false") << "\n";

        // Test 4: Fill and partial pop
        IntStack s4(4);
        std::cout << "T4 push 1: " << (s4.push(1) ? "true" : "false") << "\n";
        std::cout << "T4 push 2: " << (s4.push(2) ? "true" : "false") << "\n";
        std::cout << "T4 push 3: " << (s4.push(3) ? "true" : "false") << "\n";
        std::cout << "T4 size: " << s4.size() << "\n";
        r = s4.pop();
        std::cout << "T4 pop: " << (r.ok ? std::to_string(r.value) : std::string("None")) << "\n";
        r = s4.peek();
        std::cout << "T4 peek: " << (r.ok ? std::to_string(r.value) : std::string("None")) << "\n";

        // Test 5: Mixed operations
        IntStack s5(5);
        std::cout << "T5 push 7: " << (s5.push(7) ? "true" : "false") << "\n";
        std::cout << "T5 push 8: " << (s5.push(8) ? "true" : "false") << "\n";
        std::cout << "T5 push 9: " << (s5.push(9) ? "true" : "false") << "\n";
        r = s5.peek();
        std::cout << "T5 peek: " << (r.ok ? std::to_string(r.value) : std::string("None")) << "\n";
        r = s5.pop();
        std::cout << "T5 pop: " << (r.ok ? std::to_string(r.value) : std::string("None")) << "\n";
        std::cout << "T5 push 10: " << (s5.push(10) ? "true" : "false") << "\n";
        std::cout << "T5 size: " << s5.size() << "\n";
    } catch (const std::exception& ex) {
        std::cout << "Error: " << ex.what() << "\n";
    }
    return 0;
}