#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>

class ZeroEvenOdd {
private:
    int n;
    int current;
    // 0 for zero, 1 for odd, 2 for even
    int state; 
    std::mutex mtx;
    std::condition_variable cv;

public:
    ZeroEvenOdd(int n) : n(n), current(1), state(0) {}

    void zero(std::function<void(int)> printNumber) {
        for (int i = 0; i < n; ++i) {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] { return this->state == 0; });
            
            printNumber(0);

            if (current % 2 != 0) {
                state = 1;
            } else {
                state = 2;
            }
            cv.notify_all();
        }
    }

    void even(std::function<void(int)> printNumber) {
        for (int i = 2; i <= n; i += 2) {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] { return this->state == 2; });

            printNumber(current);
            current++;
            state = 0;
            cv.notify_all();
        }
    }

    void odd(std::function<void(int)> printNumber) {
        for (int i = 1; i <= n; i += 2) {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] { return this->state == 1; });

            printNumber(current);
            current++;
            state = 0;
            cv.notify_all();
        }
    }
};

class Task189 {
public:
    static void run_test_case(int n) {
        std::cout << "Test Case n = " << n << std::endl;
        std::cout << "Output: ";
        auto printNumber = [](int x) { std::cout << x; };
        ZeroEvenOdd zeo(n);
        
        std::thread t1(&ZeroEvenOdd::zero, &zeo, printNumber);
        std::thread t2(&ZeroEvenOdd::even, &zeo, printNumber);
        std::thread t3(&ZeroEvenOdd::odd, &zeo, printNumber);

        t1.join();
        t2.join();
        t3.join();
        
        std::cout << std::endl << std::endl;
    }

    static void main() {
        std::vector<int> testCases = {1, 2, 5, 6, 10};
        for (int n : testCases) {
            run_test_case(n);
        }
    }
};

int main() {
    Task189::main();
    return 0;
}