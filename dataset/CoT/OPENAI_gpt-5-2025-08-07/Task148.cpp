#include <iostream>
#include <vector>
#include <new>

struct Result {
    bool ok;
    int value;
};

class SafeStack {
private:
    std::vector<int> data;
public:
    SafeStack() { data.reserve(16); }

    // Push returns true on success
    bool push(int v) noexcept {
        try {
            data.push_back(v);
            return true;
        } catch (const std::bad_alloc&) {
            return false;
        }
    }

    // Pop returns {false, 0} when empty
    Result pop() noexcept {
        if (data.empty()) return {false, 0};
        int v = data.back();
        data.pop_back();
        return {true, v};
    }

    // Peek returns {false, 0} when empty
    Result peek() const noexcept {
        if (data.empty()) return {false, 0};
        return {true, data.back()};
    }

    bool isEmpty() const noexcept { return data.empty(); }
    std::size_t size() const noexcept { return data.size(); }
};

int main() {
    // 5 test cases
    SafeStack st;

    // Test 1: Push 1,2,3 then peek
    bool t1a = st.push(1);
    bool t1b = st.push(2);
    bool t1c = st.push(3);
    Result t1p = st.peek();
    std::cout << "Test1: pushes=" << t1a << "," << t1b << "," << t1c
              << " peekOk=" << t1p.ok << " peekVal=" << (t1p.ok ? t1p.value : 0) << "\n";

    // Test 2: Pop once
    Result t2 = st.pop();
    std::cout << "Test2: popOk=" << t2.ok << " val=" << (t2.ok ? t2.value : 0) << "\n";

    // Test 3: Peek after pop
    Result t3 = st.peek();
    std::cout << "Test3: peekOk=" << t3.ok << " val=" << (t3.ok ? t3.value : 0) << "\n";

    // Test 4: Pop remaining and attempt extra pop
    Result t4a = st.pop();
    Result t4b = st.pop();
    Result t4c = st.pop(); // should fail
    std::cout << "Test4: popVals=" << (t4a.ok ? t4a.value : 0) << "," << (t4b.ok ? t4b.value : 0)
              << " extraPopOk=" << t4c.ok << "\n";

    // Test 5: Push after empty and peek
    bool t5a = st.push(42);
    Result t5b = st.peek();
    std::cout << "Test5: pushOk=" << t5a << " peekOk=" << t5b.ok << " val=" << (t5b.ok ? t5b.value : 0) << "\n";

    return 0;
}