#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <string>

class ZeroEvenOdd {
public:
    explicit ZeroEvenOdd(int n_) : n(n_), state(0) {
        if (n < 1 || n > 1000) {
            throw std::invalid_argument("n must be in [1, 1000]");
        }
    }

    void zero(const std::function<void(int)>& printNumber) {
        for (int i = 1; i <= n; ++i) {
            std::unique_lock<std::mutex> lk(mtx);
            cv.wait(lk, [&]{ return state == 0; });
            printNumber(0);
            state = (i % 2 == 1) ? 1 : 2;
            lk.unlock();
            cv.notify_all();
        }
    }

    void even(const std::function<void(int)>& printNumber) {
        for (int i = 2; i <= n; i += 2) {
            std::unique_lock<std::mutex> lk(mtx);
            cv.wait(lk, [&]{ return state == 2; });
            printNumber(i);
            state = 0;
            lk.unlock();
            cv.notify_all();
        }
    }

    void odd(const std::function<void(int)>& printNumber) {
        for (int i = 1; i <= n; i += 2) {
            std::unique_lock<std::mutex> lk(mtx);
            cv.wait(lk, [&]{ return state == 1; });
            printNumber(i);
            state = 0;
            lk.unlock();
            cv.notify_all();
        }
    }

private:
    int n;
    std::mutex mtx;
    std::condition_variable cv;
    int state; // 0 -> zero's turn, 1 -> odd's turn, 2 -> even's turn
};

std::string runZeroEvenOdd(int n) {
    ZeroEvenOdd zeo(n);
    std::string buf;
    std::mutex out_mtx;

    auto printNumber = [&](int x) {
        std::lock_guard<std::mutex> lk(out_mtx);
        std::cout << x;
        buf += std::to_string(x);
    };

    std::thread tZero(&ZeroEvenOdd::zero, &zeo, std::cref(printNumber));
    std::thread tEven(&ZeroEvenOdd::even, &zeo, std::cref(printNumber));
    std::thread tOdd(&ZeroEvenOdd::odd, &zeo, std::cref(printNumber));

    tZero.join();
    tEven.join();
    tOdd.join();
    std::cout << std::endl;
    return buf;
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int tests[5] = {1, 2, 5, 10, 3};
    for (int n : tests) {
        std::string out = runZeroEvenOdd(n);
        std::cout << "Returned: " << out << "\n";
    }
    return 0;
}