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
    int current_num;
    int turn; // 0 for zero, 1 for odd, 2 for even
    std::mutex mtx;
    std::condition_variable cv;

public:
    ZeroEvenOdd(int n) {
        this->n = n;
        this->current_num = 1;
        this->turn = 0;
    }

    void zero(std::function<void(int)> printNumber) {
        for (int i = 0; i < n; ++i) {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] { return turn == 0; });
            
            printNumber(0);
            
            if (current_num % 2 != 0) {
                turn = 1; // switch to odd
            } else {
                turn = 2; // switch to even
            }
            cv.notify_all();
        }
    }

    void even(std::function<void(int)> printNumber) {
        for (int i = 2; i <= n; i += 2) {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] { return turn == 2; });
            
            printNumber(current_num);
            current_num++;
            
            turn = 0; // switch to zero
            cv.notify_all();
        }
    }

    void odd(std::function<void(int)> printNumber) {
        for (int i = 1; i <= n; i += 2) {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [this] { return turn == 1; });
            
            printNumber(current_num);
            current_num++;
            
            turn = 0; // switch to zero
            cv.notify_all();
        }
    }
};

int main() {
    std::vector<int> test_cases = {1, 2, 5, 6, 10};
    for (int n : test_cases) {
        std::cout << "Test Case n = " << n << ": ";
        ZeroEvenOdd zeo(n);

        std::thread thread_a(&ZeroEvenOdd::zero, &zeo, printNumber);
        std::thread thread_b(&ZeroEvenOdd::even, &zeo, printNumber);
        std::thread thread_c(&ZeroEvenOdd::odd, &zeo, printNumber);

        thread_a.join();
        thread_b.join();
        thread_c.join();

        std::cout << std::endl << std::endl;
    }
    return 0;
}