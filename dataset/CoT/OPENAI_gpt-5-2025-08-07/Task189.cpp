#include <iostream>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <string>
#include <vector>

class Semaphore {
public:
    explicit Semaphore(int count = 0) : count_(count) {}
    void acquire() {
        std::unique_lock<std::mutex> lk(m_);
        cv_.wait(lk, [this]() { return count_ > 0; });
        --count_;
    }
    void release() {
        std::lock_guard<std::mutex> lk(m_);
        ++count_;
        cv_.notify_one();
    }
private:
    std::mutex m_;
    std::condition_variable cv_;
    int count_;
};

class ZeroEvenOdd {
public:
    explicit ZeroEvenOdd(int n) : n_(n), zeroSem_(1), evenSem_(0), oddSem_(0) {
        if (n < 1 || n > 1000) {
            throw std::invalid_argument("n out of bounds");
        }
    }

    void zero(const std::function<void(int)>& printNumber) {
        for (int i = 1; i <= n_; ++i) {
            zeroSem_.acquire();
            printNumber(0);
            if (i % 2 == 1) {
                oddSem_.release();
            } else {
                evenSem_.release();
            }
        }
    }

    void even(const std::function<void(int)>& printNumber) {
        for (int i = 2; i <= n_; i += 2) {
            evenSem_.acquire();
            printNumber(i);
            zeroSem_.release();
        }
    }

    void odd(const std::function<void(int)>& printNumber) {
        for (int i = 1; i <= n_; i += 2) {
            oddSem_.acquire();
            printNumber(i);
            zeroSem_.release();
        }
    }

private:
    int n_;
    Semaphore zeroSem_;
    Semaphore evenSem_;
    Semaphore oddSem_;
};

std::string run_once(int n) {
    ZeroEvenOdd zeo(n);
    std::string out;
    out.reserve(static_cast<size_t>(n * 3)); // conservative reserve
    std::mutex m;

    auto printer = [&](int x) {
        std::lock_guard<std::mutex> lk(m);
        out += std::to_string(x);
    };

    std::thread t0(&ZeroEvenOdd::zero, &zeo, printer);
    std::thread te(&ZeroEvenOdd::even, &zeo, printer);
    std::thread to(&ZeroEvenOdd::odd, &zeo, printer);

    t0.join();
    te.join();
    to.join();

    return out;
}

int main() {
    std::vector<int> tests = {1, 2, 5, 7, 10};
    for (int n : tests) {
        std::cout << "n=" << n << ": " << run_once(n) << "\n";
    }
    return 0;
}