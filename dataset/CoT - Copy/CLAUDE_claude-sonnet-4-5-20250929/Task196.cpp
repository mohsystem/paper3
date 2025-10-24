
#include <iostream>
#include <thread>
#include <semaphore.h>
#include <functional>
#include <string>
#include <sstream>

class Task196 {
private:
    int n;
    sem_t zeroSem;
    sem_t evenSem;
    sem_t oddSem;
    
public:
    Task196(int n) : n(n) {
        sem_init(&zeroSem, 0, 1);
        sem_init(&evenSem, 0, 0);
        sem_init(&oddSem, 0, 0);
    }
    
    ~Task196() {
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
    Task196 zeo(n);
    std::stringstream result;
    
    auto printNumber = [&result](int num) {
        result << num;
    };
    
    std::thread t1([&zeo, &printNumber]() { zeo.zero(printNumber); });
    std::thread t2([&zeo, &printNumber]() { zeo.even(printNumber); });
    std::thread t3([&zeo, &printNumber]() { zeo.odd(printNumber); });
    
    t1.join();
    t2.join();
    t3.join();
    
    std::string output = result.str();
    std::cout << "Test n=" << n << ": " << (output == expected ? "PASS" : "FAIL") 
              << " (Expected: " << expected << ", Got: " << output << ")" << std::endl;
}

int main() {
    testCase(2, "0102");
    testCase(5, "0102030405");
    testCase(1, "01");
    testCase(3, "010203");
    testCase(10, "01020304050607080910");
    return 0;
}
