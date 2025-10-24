#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>

void printNumber(int x) {
    std::cout << x;
}

class ZeroEvenOdd {
private:
    int n;
    std::mutex mtx;
    std::condition_variable cv;
    int state; // 0 for zero, 1 for odd, 2 for even
    int current_num;

public:
    ZeroEvenOdd(int n) {
        this->n = n;
        this->state = 0;
        this->current_num = 1;
    }

    void zero(std::function<void(int)> printNumber) {
        for (int i = 0; i < n; ++i) {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] { return state == 0; });
            
            printNumber(0);
            
            if (current_num % 2 != 0) {
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
            cv.wait(lock, [this] { return state == 2; });

            printNumber(current_num);
            
            current_num++;
            state = 0;
            cv.notify_all();
        }
    }

    void odd(std::function<void(int)> printNumber) {
        for (int i = 1; i <= n; i += 2) {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] { return state == 1; });
            
            printNumber(current_num);

            current_num++;
            state = 0;
            cv.notify_all();
        }
    }
};

class Task189 {
public:
    static void runTest(int n) {
        std::cout << "Running test for n = " << n << std::endl;
        ZeroEvenOdd zeo(n);
        
        std::thread threadA(&ZeroEvenOdd::zero, &zeo, printNumber);
        std::thread threadB(&ZeroEvenOdd::even, &zeo, printNumber);
        std::thread threadC(&ZeroEvenOdd::odd, &zeo, printNumber);

        threadA.join();
        threadB.join();
        threadC.join();
        
        std::cout << std::endl << std::endl;
    }

    static void main() {
        std::vector<int> testCases = {2, 5, 1, 6, 7};
        for (int n : testCases) {
            runTest(n);
        }
    }
};

int main() {
    Task189::main();
    return 0;
}