#include <iostream>
#include <vector>
#include <stdexcept>

class IntStack {
private:
    std::vector<int> data;

public:
    IntStack() = default;

    void push(int value) {
        data.push_back(value);
    }

    int pop() {
        if (data.empty()) {
            throw std::out_of_range("Stack underflow");
        }
        int v = data.back();
        data.pop_back();
        return v;
    }

    int peek() const {
        if (data.empty()) {
            throw std::out_of_range("Stack underflow");
        }
        return data.back();
    }

    std::size_t size() const {
        return data.size();
    }
};

int main() {
    IntStack s;
    try {
        s.push(1); s.push(2); s.push(3);
        std::cout << "Test1 Peek: " << s.peek() << "\n";
    } catch (const std::exception& e) {
        std::cout << "Test1 Exception: " << e.what() << "\n";
    }
    try {
        std::cout << "Test2 Pop: " << s.pop() << "\n";
    } catch (const std::exception& e) {
        std::cout << "Test2 Exception: " << e.what() << "\n";
    }
    try {
        std::cout << "Test3 Pop: " << s.pop() << "\n";
    } catch (const std::exception& e) {
        std::cout << "Test3 Exception: " << e.what() << "\n";
    }
    try {
        s.push(4);
        std::cout << "Test4 Pop: " << s.pop() << "\n";
    } catch (const std::exception& e) {
        std::cout << "Test4 Exception: " << e.what() << "\n";
    }
    try {
        std::cout << "Test5 Pop: " << s.pop() << "\n"; // should pop 1
        std::cout << "Test5 Pop: " << s.pop() << "\n"; // underflow
    } catch (const std::exception& e) {
        std::cout << "Test5 Exception: " << e.what() << "\n";
    }
    return 0;
}