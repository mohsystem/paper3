// Task188 - C++ implementation
// Chain-of-Through process:
// 1) Problem understanding: ensure "foobar" order using two threads.
// 2) Security requirements: constrain input, avoid races with mutex/condition_variable.
// 3) Secure coding generation: strict alternation with turn flag and condition_variable.
// 4) Code review: no data race on shared string; threads joined properly.
// 5) Secure code output: clamp n to [0,1000]; reserve capacity to avoid realloc surprises.

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>
#include <vector>
#include <algorithm>

struct FooBar {
    int n;
    std::mutex m;
    std::condition_variable cv;
    int turn; // 0 -> foo's turn, 1 -> bar's turn
    std::string result;

    explicit FooBar(int n_) : n(n_), turn(0) {
        result.reserve(static_cast<size_t>(n) * 6);
    }

    void foo() {
        for (int i = 0; i < n; ++i) {
            std::unique_lock<std::mutex> lk(m);
            cv.wait(lk, [this]{ return turn == 0; });
            result += "foo";
            turn = 1;
            lk.unlock();
            cv.notify_one();
        }
    }

    void bar() {
        for (int i = 0; i < n; ++i) {
            std::unique_lock<std::mutex> lk(m);
            cv.wait(lk, [this]{ return turn == 1; });
            result += "bar";
            turn = 0;
            lk.unlock();
            cv.notify_one();
        }
    }
};

// Function that accepts input as parameter and returns the output
std::string solve(int n) {
    if (n < 0) n = 0;
    if (n > 1000) n = 1000;

    FooBar fb(n);
    std::thread t1(&FooBar::foo, &fb);
    std::thread t2(&FooBar::bar, &fb);
    t1.join();
    t2.join();
    return fb.result;
}

int main() {
    std::vector<int> tests = {1, 2, 3, 5, 7};
    for (int n : tests) {
        std::cout << solve(n) << std::endl;
    }
    return 0;
}