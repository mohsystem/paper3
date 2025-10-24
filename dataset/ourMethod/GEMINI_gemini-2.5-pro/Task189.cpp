#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>
#include <stdexcept>

class ZeroEvenOdd {
private:
    int n;
    int state; // 0 for zero, 1 for odd, 2 for even
    std::mutex mtx;
    std::condition_variable cv;

public:
    ZeroEvenOdd(int n) {
        if (n < 1 || n > 1000) {
            throw std::invalid_argument("n must be between 1 and 1000");
        }
        this->n = n;
        this->state = 0;
    }

    void zero(std::function<void(int)> printNumber) {
        for (int i = 1; i <= n; ++i) {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this]{ return state == 0; });
            printNumber(0);
            if (i % 2 != 0) {
                state = 1; // next is odd
            } else {
                state = 2; // next is even
            }
            cv.notify_all();
        }
    }

    void even(std::function<void(int)> printNumber) {
        for (int i = 2; i <= n; i += 2) {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this]{ return state == 2; });
            printNumber(i);
            state = 0;
            cv.notify_all();
        }
    }

    void odd(std::function<void(int)> printNumber) {
        for (int i = 1; i <= n; i += 2) {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this]{ return state == 1; });
            printNumber(i);
            state = 0;
            cv.notify_all();
        }
    }
};

void printNumber(int x) {
    std::cout << x;
}

void runTest(int n) {
    std::cout << "Test case n = " << n << std::endl;
    try {
        ZeroEvenOdd zeo(n);
        std::thread tA(&ZeroEvenOdd::zero, &zeo, printNumber);
        std::thread tB(&ZeroEvenOdd::even, &zeo, printNumber);
        std::thread tC(&ZeroEvenOdd::odd, &zeo, printNumber);

        tA.join();
        tB.join();
        tC.join();
        std::cout << std::endl << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cerr << e.what() << std::endl;
    }
}

int main() {
    int testCases[] = {1, 2, 5, 6, 10};
    for (int n : testCases) {
        runTest(n);
    }
    return 0;
}