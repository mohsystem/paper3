#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>

class ZeroEvenOdd {
private:
    int n;
    int state; // 0 for zero, 1 for odd, 2 for even
    std::mutex mtx;
    std::condition_variable cv;

public:
    ZeroEvenOdd(int n) {
        this->n = n;
        this->state = 0;
    }

    void zero(std::function<void(int)> printNumber) {
        for (int i = 1; i <= n; ++i) {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] { return state == 0; });
            printNumber(0);
            if (i % 2 != 0) {
                state = 1; // Next is odd
            } else {
                state = 2; // Next is even
            }
            cv.notify_all();
        }
    }

    void even(std::function<void(int)> printNumber) {
        for (int i = 2; i <= n; i += 2) {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] { return state == 2; });
            printNumber(i);
            state = 0;
            cv.notify_all();
        }
    }

    void odd(std::function<void(int)> printNumber) {
        for (int i = 1; i <= n; i += 2) {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] { return state == 1; });
            printNumber(i);
            state = 0;
            cv.notify_all();
        }
    }
};

// C++ doesn't have a "main class" concept like Java.
// The main function is the entry point of the program.
// We'll place the main function within a class as requested.
class Task196 {
public:
    static void printNumber(int x) {
        std::cout << x;
    }

    static void runTest(int n) {
        std::cout << "Test case n = " << n << std::endl;
        std::cout << "Output: ";
        
        ZeroEvenOdd zeroEvenOdd(n);
        
        std::thread threadA(&ZeroEvenOdd::zero, &zeroEvenOdd, printNumber);
        std::thread threadB(&ZeroEvenOdd::even, &zeroEvenOdd, printNumber);
        std::thread threadC(&ZeroEvenOdd::odd, &zeroEvenOdd, printNumber);

        threadA.join();
        threadB.join();
        threadC.join();

        std::cout << std::endl << std::endl;
    }
};

int main() {
    Task196::runTest(2);
    Task196::runTest(5);
    Task196::runTest(1);
    Task196::runTest(6);
    Task196::runTest(10);
    return 0;
}