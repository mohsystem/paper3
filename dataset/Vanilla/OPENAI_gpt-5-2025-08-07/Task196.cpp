#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>

class ZeroEvenOdd {
private:
    int n;
    std::mutex m;
    std::condition_variable cv;
    int state; // 0: zero's turn, 1: odd's turn, 2: even's turn
    int curr;

public:
    ZeroEvenOdd(int n) : n(n), state(0), curr(1) {}

    void zero(const std::function<void(int)>& printNumber) {
        for (int i = 1; i <= n; ++i) {
            std::unique_lock<std::mutex> lk(m);
            cv.wait(lk, [&] { return state == 0; });
            printNumber(0);
            state = (curr % 2 == 1) ? 1 : 2;
            lk.unlock();
            cv.notify_all();
        }
    }

    void even(const std::function<void(int)>& printNumber) {
        while (true) {
            std::unique_lock<std::mutex> lk(m);
            cv.wait(lk, [&] { return state == 2 || curr > n; });
            if (curr > n) {
                cv.notify_all();
                break;
            }
            int v = curr;
            printNumber(v);
            ++curr;
            state = 0;
            lk.unlock();
            cv.notify_all();
        }
    }

    void odd(const std::function<void(int)>& printNumber) {
        while (true) {
            std::unique_lock<std::mutex> lk(m);
            cv.wait(lk, [&] { return state == 1 || curr > n; });
            if (curr > n) {
                cv.notify_all();
                break;
            }
            int v = curr;
            printNumber(v);
            ++curr;
            state = 0;
            lk.unlock();
            cv.notify_all();
        }
    }
};

int main() {
    auto printNumber = [](int x) { std::cout << x; std::cout.flush(); };
    std::vector<int> tests = {1, 2, 5, 7, 10};
    for (int n : tests) {
        ZeroEvenOdd zeo(n);
        std::thread t0(&ZeroEvenOdd::zero, &zeo, printNumber);
        std::thread t1(&ZeroEvenOdd::even, &zeo, printNumber);
        std::thread t2(&ZeroEvenOdd::odd, &zeo, printNumber);
        t0.join();
        t1.join();
        t2.join();
        std::cout << std::endl;
    }
    return 0;
}