
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <atomic>
#include <sstream>

// Global shared resources with proper initialization
static std::mutex counterMutex;
static int currentCounter = 0;
static const int maxCounter = 20;

// Thread-safe function to manage access to shared resource
// Each parameter is validated and all operations are protected
void accessSharedResource(int threadId) {
    // Input validation: threadId must be non-negative
    if (threadId < 0) {
        return;
    }
    
    // Check-then-act pattern protected by the same lock to prevent TOCTOU
    // Lock is acquired before checking to ensure atomicity of check and increment
    std::unique_lock<std::mutex> lock(counterMutex);
    
    // Check if we can increment (prevent overflow and respect maxCounter)
    // Comparison done while holding lock to prevent race condition
    if (currentCounter <= maxCounter) {
        // Safe increment: check for integer overflow before incrementing
        if (currentCounter < INT_MAX) {
            currentCounter++;
            
            // Use ostringstream for thread-safe formatted output
            // This prevents interleaved output from multiple threads
            std::ostringstream oss;
            oss << "Thread " << threadId << " accessing currentCounter: " 
                << currentCounter << "\\n";
            std::cout << oss.str() << std::flush;
        }
    }
    // Lock automatically released by RAII when unique_lock goes out of scope
}

int main() {
    // Test case 1: Single thread access
    std::cout << "Test 1: Single thread\\n";
    currentCounter = 0;
    accessSharedResource(1);
    
    // Test case 2: Two threads
    std::cout << "\\nTest 2: Two threads\\n";
    currentCounter = 0;
    {
        std::thread t1(accessSharedResource, 1);
        std::thread t2(accessSharedResource, 2);
        t1.join();
        t2.join();
    }
    
    // Test case 3: Five threads
    std::cout << "\\nTest 3: Five threads\\n";
    currentCounter = 0;
    {
        std::vector<std::thread> threads;
        threads.reserve(5); // Pre-allocate to avoid reallocation
        for (int i = 0; i < 5; i++) {
            threads.emplace_back(accessSharedResource, i + 1);
        }
        for (auto& t : threads) {
            t.join();
        }
    }
    
    // Test case 4: Ten threads with counter near maxCounter
    std::cout << "\\nTest 4: Ten threads near limit\\n";
    currentCounter = 15;
    {
        std::vector<std::thread> threads;
        threads.reserve(10);
        for (int i = 0; i < 10; i++) {
            threads.emplace_back(accessSharedResource, i + 1);
        }
        for (auto& t : threads) {
            t.join();
        }
    }
    
    // Test case 5: Threads when counter exceeds maxCounter
    std::cout << "\\nTest 5: Threads when counter exceeds max\\n";
    currentCounter = 21;
    {
        std::vector<std::thread> threads;
        threads.reserve(3);
        for (int i = 0; i < 3; i++) {
            threads.emplace_back(accessSharedResource, i + 1);
        }
        for (auto& t : threads) {
            t.join();
        }
    }
    
    return 0;
}
