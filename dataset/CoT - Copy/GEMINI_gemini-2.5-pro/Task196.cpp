#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>

class ZeroEvenOdd {
private:
    int n;
    int turn;
    std::mutex mtx;
    std::condition_variable cv;

public:
    ZeroEvenOdd(int n) {
        this->n = n;
        this->turn = 0; // 0 for zero, 1 for odd, 2 for even
    }

    void zero(std::function<void(int)> printNumber) {
        for (int i = 1; i <= n; ++i) {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this]{ return turn == 0; });
            printNumber(0);
            if (i % 2 != 0) {
                turn = 1;
            } else {
                turn = 2;
            }
            cv.notify_all();
        }
    }

    void even(std::function<void(int)> printNumber) {
        for (int i = 2; i <= n; i += 2) {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this]{ return turn == 2; });
            printNumber(i);
            turn = 0;
            cv.notify_all();
        }
    }

    void odd(std::function<void(int)> printNumber) {
        for (int i = 1; i <= n; i += 2) {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this]{ return turn == 1; });
            printNumber(i);
            turn = 0;
            cv.notify_all();
        }
    }
};

void runTest(int n) {
    std::cout << "Test Case for n = " << n << std::endl;
    std::cout << "Output: ";

    ZeroEvenOdd zeroEvenOdd(n);
    auto printNumber = [](int x) { std::cout << x; };

    std::thread threadA(&ZeroEvenOdd::zero, &zeroEvenOdd, printNumber);
    std::thread threadB(&ZeroEvenOdd::even, &zeroEvenOdd, printNumber);
    std::thread threadC(&ZeroEvenOdd::odd, &zeroEvenOdd, printNumber);

    threadA.join();
    threadB.join();
    threadC.join();

    std::cout << std::endl << std::endl;
}

int main() {
    runTest(2);
    runTest(5);
    runTest(1);
    runTest(6);
    runTest(10);
    return 0;
}