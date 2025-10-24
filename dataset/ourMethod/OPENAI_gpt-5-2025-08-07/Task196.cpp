#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <string>

class ZeroEvenOdd {
public:
    explicit ZeroEvenOdd(int n) : n_(n), cur_(1), zero_turn_(true) {
        if (n_ < 1 || n_ > 1000) {
            throw std::invalid_argument("n must be in [1, 1000]");
        }
    }

    void zero(const std::function<void(int)>& printNumber) {
        std::unique_lock<std::mutex> lk(m_);
        while (true) {
            cv_.wait(lk, [&]{ return zero_turn_ || cur_ > n_; });
            if (cur_ > n_) {
                cv_.notify_all();
                return;
            }
            printNumber(0);
            zero_turn_ = false;
            cv_.notify_all();
        }
    }

    void even(const std::function<void(int)>& printNumber) {
        std::unique_lock<std::mutex> lk(m_);
        while (true) {
            cv_.wait(lk, [&]{ return cur_ > n_ || (!zero_turn_ && (cur_ % 2 == 0)); });
            if (cur_ > n_) {
                cv_.notify_all();
                return;
            }
            printNumber(cur_);
            ++cur_;
            zero_turn_ = true;
            cv_.notify_all();
        }
    }

    void odd(const std::function<void(int)>& printNumber) {
        std::unique_lock<std::mutex> lk(m_);
        while (true) {
            cv_.wait(lk, [&]{ return cur_ > n_ || (!zero_turn_ && (cur_ % 2 == 1)); });
            if (cur_ > n_) {
                cv_.notify_all();
                return;
            }
            printNumber(cur_);
            ++cur_;
            zero_turn_ = true;
            cv_.notify_all();
        }
    }

private:
    int n_;
    int cur_;
    bool zero_turn_;
    std::mutex m_;
    std::condition_variable cv_;
};

int main() {
    auto run_test = [](int n) {
        ZeroEvenOdd zeo(n);
        std::mutex out_mtx;
        std::string out;

        auto printNumber = [&](int x) {
            {
                std::lock_guard<std::mutex> lk(out_mtx);
                out += std::to_string(x);
            }
            std::cout << x;
            std::cout.flush();
        };

        std::thread t0([&]{ zeo.zero(printNumber); });
        std::thread t1([&]{ zeo.even(printNumber); });
        std::thread t2([&]{ zeo.odd(printNumber); });

        t0.join();
        t1.join();
        t2.join();

        std::cout << " -> " << out << std::endl;
    };

    run_test(1);
    run_test(2);
    run_test(3);
    run_test(5);
    run_test(10);

    return 0;
}