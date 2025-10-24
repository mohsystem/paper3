
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>
#include <stdexcept>

class FooBar {
private:
    int n;
    std::mutex mtx;
    std::condition_variable cv;
    bool fooTurn; // Synchronization flag to ensure alternating execution
    
public:
    // Constructor validates n to prevent integer overflow and invalid states
    FooBar(int n) : n(n), fooTurn(true) {
        // Input validation: n must be in valid range per constraints
        if (n < 1 || n > 1000) {
            throw std::invalid_argument("n must be between 1 and 1000");
        }
    }
    
    // Prevent copying to avoid race conditions on shared state
    FooBar(const FooBar&) = delete;
    FooBar& operator=(const FooBar&) = delete;
    
    void foo(std::string& result) {
        for (int i = 0; i < n; i++) {
            std::unique_lock<std::mutex> lock(mtx); // RAII lock management
            // Wait until it's foo's turn, prevents spurious wakeups
            cv.wait(lock, [this]() { return fooTurn; });
            
            result += "foo"; // Thread-safe append under lock
            fooTurn = false; // Signal bar's turn\n            cv.notify_one(); // Wake up bar thread\n        }\n    }\n    \n    void bar(std::string& result) {\n        for (int i = 0; i < n; i++) {\n            std::unique_lock<std::mutex> lock(mtx); // RAII lock management\n            // Wait until it's bar's turn, prevents spurious wakeups\n            cv.wait(lock, [this]() { return !fooTurn; });\n            \n            result += "bar"; // Thread-safe append under lock\n            fooTurn = true; // Signal foo's turn
            cv.notify_one(); // Wake up foo thread
        }
    }
};

int main() {
    // Test case 1: n = 1
    try {
        FooBar fb1(1);
        std::string result1;
        std::thread t1(&FooBar::foo, &fb1, std::ref(result1));
        std::thread t2(&FooBar::bar, &fb1, std::ref(result1));
        t1.join();
        t2.join();
        std::cout << "Test 1 (n=1): " << result1 << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 1 error: " << e.what() << std::endl;
    }
    
    // Test case 2: n = 2
    try {
        FooBar fb2(2);
        std::string result2;
        std::thread t3(&FooBar::foo, &fb2, std::ref(result2));
        std::thread t4(&FooBar::bar, &fb2, std::ref(result2));
        t3.join();
        t4.join();
        std::cout << "Test 2 (n=2): " << result2 << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 2 error: " << e.what() << std::endl;
    }
    
    // Test case 3: n = 5
    try {
        FooBar fb3(5);
        std::string result3;
        std::thread t5(&FooBar::foo, &fb3, std::ref(result3));
        std::thread t6(&FooBar::bar, &fb3, std::ref(result3));
        t5.join();
        t6.join();
        std::cout << "Test 3 (n=5): " << result3 << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 3 error: " << e.what() << std::endl;
    }
    
    // Test case 4: n = 10
    try {
        FooBar fb4(10);
        std::string result4;
        std::thread t7(&FooBar::foo, &fb4, std::ref(result4));
        std::thread t8(&FooBar::bar, &fb4, std::ref(result4));
        t7.join();
        t8.join();
        std::cout << "Test 4 (n=10): " << result4 << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 4 error: " << e.what() << std::endl;
    }
    
    // Test case 5: Edge case n = 1000
    try {
        FooBar fb5(1000);
        std::string result5;
        std::thread t9(&FooBar::foo, &fb5, std::ref(result5));
        std::thread t10(&FooBar::bar, &fb5, std::ref(result5));
        t9.join();
        t10.join();
        std::cout << "Test 5 (n=1000): Length = " << result5.length() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 5 error: " << e.what() << std::endl;
    }
    
    return 0;
}
