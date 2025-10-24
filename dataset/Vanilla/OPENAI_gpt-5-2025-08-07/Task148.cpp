#include <bits/stdc++.h>
using namespace std;

class Stack {
    vector<int> data;
public:
    void push(int value) {
        data.push_back(value);
    }
    int pop() {
        if (data.empty()) throw runtime_error("Stack underflow");
        int v = data.back();
        data.pop_back();
        return v;
    }
    int peek() const {
        if (data.empty()) throw runtime_error("Stack is empty");
        return data.back();
    }
    bool isEmpty() const { return data.empty(); }
    int size() const { return (int)data.size(); }
};

int main() {
    Stack s;
    s.push(10);
    s.push(20);
    s.push(30);
    cout << s.peek() << "\n"; // Test 1
    cout << s.pop() << "\n";  // Test 2
    s.push(40);
    cout << s.peek() << "\n"; // Test 3
    cout << s.pop() << "\n";  // Test 4
    cout << s.pop() << "\n";  // Test 5
    return 0;
}