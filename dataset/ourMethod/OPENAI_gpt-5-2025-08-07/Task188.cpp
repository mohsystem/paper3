#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

class FooBar {
public:
    explicit FooBar(int n_) : n(n_), foo_turn(true) {}

    int foo(const std::function<void()>& printFoo) {
        for (int i = 0; i < n; ++i) {
            std::unique_lock<std::mutex> lk(m);
            cv.wait(lk, [this]() { return foo_turn; });
            printFoo();
            foo_turn = false;
            cv.notify_all();
        }
        return n;
    }

    int bar(const std::function<void()>& printBar) {
        for (int i = 0; i < n; ++i) {
            std::unique_lock<std::mutex> lk(m);
            cv.wait(lk, [this]() { return !foo_turn; });
            printBar();
            foo_turn = true;
            cv.notify_all();
        }
        return n;
    }

private:
    int n;
    std::mutex m;
    std::condition_variable cv;
    bool foo_turn;
};

std::string generateFoobar(int n) {
    if (n < 1 || n > 1000) {
        throw std::invalid_argument("n must be in [1, 1000]");
    }
    FooBar fb(n);
    std::string result;
    result.reserve(static_cast<size_t>(n) * 6);

    std::thread t1([&]() { fb.foo([&]() { result += "foo"; }); });
    std::thread t2([&]() { fb.bar([&]() { result += "bar"; }); });

    t1.join();
    t2.join();

    return result;
}

int main() {
    std::vector<int> tests = {1, 2, 5, 10, 0};
    for (int n : tests) {
        try {
            std::string res = generateFoobar(n);
            std::cout << "n=" << n << " -> " << res << "\n";
        } catch (const std::exception& e) {
            std::cout << "n=" << n << " -> error: " << e.what() << "\n";
        }
    }
    return 0;
}