#include <iostream>
#include <thread>
#include <functional>
#include <semaphore>
#include <vector>

// Note: This code requires a C++20 compliant compiler for std::counting_semaphore.
// To compile: g++ -std=c++20 -o task task.cpp -lpthread

class ZeroEvenOdd {
private:
    int n;
    std::counting_semaphore<> sem_zero{1};
    std::counting_semaphore<> sem_even{0};
    std::counting_semaphore<> sem_odd{0};

public:
    ZeroEvenOdd(int n) : n(n) {}

    void zero(std::function<void(int)> printNumber) {
        for (int i = 1; i <= n; ++i) {
            sem_zero.acquire();
            printNumber(0);
            if (i % 2 == 0) {
                sem_even.release();
            } else {
                sem_odd.release();
            }
        }
    }

    void even(std::function<void(int)> printNumber) {
        for (int i = 2; i <= n; i += 2) {
            sem_even.acquire();
            printNumber(i);
            sem_zero.release();
        }
    }

    void odd(std::function<void(int)> printNumber) {
        for (int i = 1; i <= n; i += 2) {
            sem_odd.acquire();
            printNumber(i);
            sem_zero.release();
        }
    }
};

void printNumber(int x) {
    std::cout << x;
}

void runTest(int n) {
    std::cout << "Testing with n = " << n << std::endl;
    ZeroEvenOdd zeroEvenOdd(n);
    
    std::thread threadA(&ZeroEvenOdd::zero, &zeroEvenOdd, printNumber);
    std::thread threadB(&ZeroEvenOdd::even, &zeroEvenOdd, printNumber);
    std::thread threadC(&ZeroEvenOdd::odd, &zeroEvenOdd, printNumber);
    
    threadA.join();
    threadB.join();
    threadC.join();
    
    std::cout << std::endl << std::endl;
}

int main() {
    std::vector<int> testCases = {2, 5, 1, 6, 10};
    for (int n : testCases) {
        runTest(n);
    }
    return 0;
}