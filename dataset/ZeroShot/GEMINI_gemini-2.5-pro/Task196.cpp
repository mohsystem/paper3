#include <iostream>
#include <thread>
#include <functional>
#include <semaphore>
#include <vector>

// Note: This code requires C++20 for std::counting_semaphore.
// Compile with: g++ -std=c++20 -o your_program_name your_source_file.cpp -lpthread

class ZeroEvenOdd {
private:
    int n;
    std::counting_semaphore<1> semZero;
    std::counting_semaphore<1> semEven;
    std::counting_semaphore<1> semOdd;

public:
    ZeroEvenOdd(int n) : n(n), semZero(1), semEven(0), semOdd(0) {}

    void zero(std::function<void(int)> printNumber) {
        for (int i = 1; i <= n; ++i) {
            semZero.acquire();
            printNumber(0);
            if (i % 2 != 0) { // Next is odd
                semOdd.release();
            } else { // Next is even
                semEven.release();
            }
        }
    }

    void even(std::function<void(int)> printNumber) {
        for (int i = 2; i <= n; i += 2) {
            semEven.acquire();
            printNumber(i);
            semZero.release();
        }
    }

    void odd(std::function<void(int)> printNumber) {
        for (int i = 1; i <= n; i += 2) {
            semOdd.acquire();
            printNumber(i);
            semZero.release();
        }
    }
};

void printNumber(int x) {
    std::cout << x;
}

void run_test(int n) {
    std::cout << "Test Case n = " << n << ":" << std::endl;
    ZeroEvenOdd zeo(n);

    std::thread threadA(&ZeroEvenOdd::zero, &zeo, printNumber);
    std::thread threadB(&ZeroEvenOdd::even, &zeo, printNumber);
    std::thread threadC(&ZeroEvenOdd::odd, &zeo, printNumber);

    threadA.join();
    threadB.join();
    threadC.join();

    std::cout << std::endl << std::endl;
}

class Task196 {
public:
    static void main() {
        std::vector<int> testCases = {2, 5, 1, 6, 10};
        for (int n : testCases) {
            run_test(n);
        }
    }
};

int main() {
    Task196::main();
    return 0;
}