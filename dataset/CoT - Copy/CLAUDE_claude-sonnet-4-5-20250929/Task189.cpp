
#include <iostream>
#include <thread>
#include <functional>
#include <semaphore.h>
#include <string>
#include <mutex>

class Task189 {
private:
    int n;
    sem_t zeroSem;
    sem_t evenSem;
    sem_t oddSem;
    
public:
    Task189(int n) : n(n) {
        sem_init(&zeroSem, 0, 1);
        sem_init(&evenSem, 0, 0);
        sem_init(&oddSem, 0, 0);
    }
    
    ~Task189() {
        sem_destroy(&zeroSem);
        sem_destroy(&evenSem);
        sem_destroy(&oddSem);
    }
    
    void zero(std::function<void(int)> printNumber) {
        for (int i = 0; i < n; i++) {
            sem_wait(&zeroSem);
            printNumber(0);
            if (i % 2 == 0) {
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

void testCase(int n, const std::string& expected) {
    Task189 zeroEvenOdd(n);
    std::string result;
    std::mutex mtx;
    
    auto printNumber = [&result, &mtx](int num) {
        std::lock_guard<std::mutex> lock(mtx);
        result += std::to_string(num);
    };
    
    std::thread threadA(&Task189::zero, &zeroEvenOdd, printNumber);
    std::thread threadB(&Task189::even, &zeroEvenOdd, printNumber);
    std::thread threadC(&Task189::odd, &zeroEvenOdd, printNumber);
    
    threadA.join();
    threadB.join();
    threadC.join();
    
    std::cout << "Input: n = " << n << std::endl;
    std::cout << "Expected: " << expected << std::endl;
    std::cout << "Output: " << result << std::endl;
    std::cout << "Test " << (result == expected ? "PASSED" : "FAILED") << std::endl;
    std::cout << std::endl;
}

int main() {
    testCase(2, "0102");
    testCase(5, "0102030405");
    testCase(1, "01");
    testCase(3, "010203");
    testCase(4, "01020304");
    return 0;
}
