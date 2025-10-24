
#include <iostream>
#include <functional>
#include <thread>
#include <semaphore.h>
#include <string>
#include <vector>

class ZeroEvenOdd {
private:
    int n;
    sem_t zeroSem;
    sem_t evenSem;
    sem_t oddSem;

public:
    ZeroEvenOdd(int n) {
        this->n = n;
        sem_init(&zeroSem, 0, 1);
        sem_init(&evenSem, 0, 0);
        sem_init(&oddSem, 0, 0);
    }
    
    ~ZeroEvenOdd() {
        sem_destroy(&zeroSem);
        sem_destroy(&evenSem);
        sem_destroy(&oddSem);
    }

    void zero(std::function<void(int)> printNumber) {
        for (int i = 1; i <= n; i++) {
            sem_wait(&zeroSem);
            printNumber(0);
            if (i % 2 == 1) {
                sem_post(&oddSem);
            } else {
                sem_post(&evenSem);
            }
        }
    }

    void even(std::function<void(int)> printNumber) {
        for (int i = 2; i <= n; i += 2) {
            sem_wait(&evenSem);
            printNumber(i);
            sem_post(&zeroSem);
        }
    }

    void odd(std::function<void(int)> printNumber) {
        for (int i = 1; i <= n; i += 2) {
            sem_wait(&oddSem);
            printNumber(i);
            sem_post(&zeroSem);
        }
    }
};

void runTest(int n) {
    ZeroEvenOdd zeo(n);
    std::string result;
    
    auto printNumber = [&result](int num) {
        result += std::to_string(num);
    };
    
    std::thread t1(&ZeroEvenOdd::zero, &zeo, printNumber);
    std::thread t2(&ZeroEvenOdd::even, &zeo, printNumber);
    std::thread t3(&ZeroEvenOdd::odd, &zeo, printNumber);
    
    t1.join();
    t2.join();
    t3.join();
    
    std::cout << result << std::endl;
}

int main() {
    std::cout << "Test 1 (n=2): ";
    runTest(2);
    
    std::cout << "Test 2 (n=5): ";
    runTest(5);
    
    std::cout << "Test 3 (n=1): ";
    runTest(1);
    
    std::cout << "Test 4 (n=10): ";
    runTest(10);
    
    std::cout << "Test 5 (n=7): ";
    runTest(7);
    
    return 0;
}
