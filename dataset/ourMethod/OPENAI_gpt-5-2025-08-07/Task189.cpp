#include <iostream>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <stdexcept>

class ZeroEvenOdd {
public:
    explicit ZeroEvenOdd(int n) : n_(n), state_(0) {
        if (n < 1 || n > 1000) throw std::invalid_argument("n out of range");
    }

    void zero(const std::function<void(int)>& printNumber) {
        for (int i = 1; i <= n_; ++i) {
            std::unique_lock<std::mutex> lk(m_);
            cv_.wait(lk, [&]{ return state_ == 0; });
            printNumber(0);
            state_ = (i % 2 == 1) ? 1 : 2;
            cv_.notify_all();
        }
    }

    void even(const std::function<void(int)>& printNumber) {
        for (int i = 2; i <= n_; i += 2) {
            std::unique_lock<std::mutex> lk(m_);
            cv_.wait(lk, [&]{ return state_ == 2; });
            printNumber(i);
            state_ = 0;
            cv_.notify_all();
        }
    }

    void odd(const std::function<void(int)>& printNumber) {
        for (int i = 1; i <= n_; i += 2) {
            std::unique_lock<std::mutex> lk(m_);
            cv_.wait(lk, [&]{ return state_ == 1; });
            printNumber(i);
            state_ = 0;
            cv_.notify_all();
        }
    }

private:
    int n_;
    int state_; // 0 -> zero's turn, 1 -> odd's turn, 2 -> even's turn
    std::mutex m_;
    std::condition_variable cv_;
};

static void printNumber(int x) {
    std::cout << x;
}

static void run_test(int n) {
    ZeroEvenOdd zeo(n);
    std::thread t0([&]{ zeo.zero(printNumber); });
    std::thread t1([&]{ zeo.even(printNumber); });
    std::thread t2([&]{ zeo.odd(printNumber); });
    t0.join(); t1.join(); t2.join();
    std::cout << "\n";
}

int main() {
    std::vector<int> tests = {1, 2, 5, 7, 10};
    for (int n : tests) {
        run_test(n);
    }
    return 0;
}