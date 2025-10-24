#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <string>

// Prints number to console
void printNumber(int x) {
    std::cout << x;
}

class ZeroEvenOdd {
private:
    int n;
    int current;
    int state; // 0: zero, 1: odd, 2: even
    std::mutex mtx;
    std::condition_variable cv;

public:
    ZeroEvenOdd(int n) : n(n), current(1), state(0) {}

    void zero(std::function<void(int)> printNumber) {
        for (int i = 0; i < n; ++i) {
            std::unique_lock<std::mutex> lk(mtx);
            cv.wait(lk, [&]{ return state == 0; });
            printNumber(0);
            if (current % 2 == 1) state = 1;
            else state = 2;
            cv.notify_all();
        }
    }

    void even(std::function<void(int)> printNumber) {
        while (true) {
            std::unique_lock<std::mutex> lk(mtx);
            cv.wait(lk, [&]{ return state == 2 || current > n; });
            if (current > n) {
                cv.notify_all();
                return;
            }
            int val = current;
            printNumber(val);
            ++current;
            state = 0;
            cv.notify_all();
        }
    }

    void odd(std::function<void(int)> printNumber) {
        while (true) {
            std::unique_lock<std::mutex> lk(mtx);
            cv.wait(lk, [&]{ return state == 1 || current > n; });
            if (current > n) {
                cv.notify_all();
                return;
            }
            int val = current;
            printNumber(val);
            ++current;
            state = 0;
            cv.notify_all();
        }
    }
};

std::string runZeroEvenOdd(int n) {
    ZeroEvenOdd zeo(n);
    std::string out;
    std::mutex out_mtx;

    auto p = [&](int x){
        {
            std::lock_guard<std::mutex> lg(out_mtx);
            out.push_back('0' + x); // x is single digit (0..9) given constraints per position
            if (x >= 10) { // handle numbers >= 10
                // overwrite last push; re-append full number
                out.pop_back();
                out += std::to_string(x);
            }
        }
        printNumber(x);
    };

    std::thread ta([&]{ zeo.zero(p); });
    std::thread tb([&]{ zeo.even(p); });
    std::thread tc([&]{ zeo.odd(p); });
    ta.join(); tb.join(); tc.join();
    return out;
}

int main() {
    int tests[5] = {1, 2, 5, 6, 7};
    for (int i = 0; i < 5; ++i) {
        runZeroEvenOdd(tests[i]);
        std::cout << std::endl;
    }
    return 0;
}