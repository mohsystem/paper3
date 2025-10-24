
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <sstream>
#include <string>

class ZeroEvenOdd {
private:
    int n;
    std::mutex mtx;
    std::condition_variable cv;
    int state; // 0: zero turn, 1: odd turn, 2: even turn
    int current; // current number to print (1 to n)
    
public:
    // Initialize with validated input
    ZeroEvenOdd(int n) : n(n), state(0), current(1) {
        // Validate input range as per constraints
        if (n < 1 || n > 1000) {
            throw std::invalid_argument("n must be between 1 and 1000");
        }
    }
    
    // Prevent copying to avoid shared state issues
    ZeroEvenOdd(const ZeroEvenOdd&) = delete;
    ZeroEvenOdd& operator=(const ZeroEvenOdd&) = delete;

    void zero(std::function<void(int)> printNumber) {
        // Loop through all numbers from 1 to n
        for (int i = 1; i <= n; ++i) {
            std::unique_lock<std::mutex> lock(mtx);
            // Wait until it's zero's turn (state == 0)
            cv.wait(lock, [this]() { return state == 0; });
            
            // Print zero
            printNumber(0);
            
            // Determine next state: odd if current is odd, even if current is even
            // current is guaranteed to be in range [1, n] by loop bounds
            state = (current % 2 == 1) ? 1 : 2;
            
            // Notify all waiting threads
            cv.notify_all();
        }
    }

    void even(std::function<void(int)> printNumber) {
        // Loop through even numbers
        for (int i = 2; i <= n; i += 2) {
            std::unique_lock<std::mutex> lock(mtx);
            // Wait until it's even's turn and current matches
            cv.wait(lock, [this, i]() { return state == 2 && current == i; });
            
            // Print the even number
            printNumber(i);
            
            // Move to next number and set state to zero
            current++;
            state = 0;
            
            // Notify all waiting threads
            cv.notify_all();
        }
    }

    void odd(std::function<void(int)> printNumber) {
        // Loop through odd numbers
        for (int i = 1; i <= n; i += 2) {
            std::unique_lock<std::mutex> lock(mtx);
            // Wait until it's odd's turn and current matches
            cv.wait(lock, [this, i]() { return state == 1 && current == i; });
            
            // Print the odd number
            printNumber(i);
            
            // Move to next number and set state to zero
            current++;
            state = 0;
            
            // Notify all waiting threads
            cv.notify_all();
        }
    }
};

// Test helper function
void runTest(int n, const std::string& expected) {
    std::stringstream ss;
    std::mutex output_mtx;
    
    auto printNumber = [&ss, &output_mtx](int x) {
        std::lock_guard<std::mutex> lock(output_mtx);
        ss << x;
    };
    
    ZeroEvenOdd zeo(n);
    
    std::thread t1(&ZeroEvenOdd::zero, &zeo, printNumber);
    std::thread t2(&ZeroEvenOdd::even, &zeo, printNumber);
    std::thread t3(&ZeroEvenOdd::odd, &zeo, printNumber);
    
    t1.join();
    t2.join();
    t3.join();
    
    std::string result = ss.str();
    std::cout << "Test n=" << n << ": " << result 
              << (result == expected ? " PASS" : " FAIL") << std::endl;
}

int main() {
    // Test case 1: n = 2
    runTest(2, "0102");
    
    // Test case 2: n = 5
    runTest(5, "0102030405");
    
    // Test case 3: n = 1
    runTest(1, "01");
    
    // Test case 4: n = 3
    runTest(3, "010203");
    
    // Test case 5: n = 10
    runTest(10, "010203040506070809010");
    
    return 0;
}
