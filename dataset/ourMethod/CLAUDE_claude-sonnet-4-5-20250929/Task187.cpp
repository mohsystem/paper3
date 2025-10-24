
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <atomic>

class H2O {
private:
    std::mutex mtx;
    std::condition_variable cv;
    int hydrogen_count;
    int oxygen_count;
    std::string result;
    
public:
    H2O() : hydrogen_count(0), oxygen_count(0) {}
    
    // Thread-safe hydrogen release with synchronization
    void hydrogen(std::function<void()> releaseHydrogen) {
        std::unique_lock<std::mutex> lock(mtx);
        
        // Increment hydrogen count - validates molecule formation constraint
        hydrogen_count++;
        
        // Wait until we have 2 hydrogens and 1 oxygen to form a complete molecule
        // This prevents partial molecules from proceeding
        cv.wait(lock, [this]() { 
            return hydrogen_count >= 2 && oxygen_count >= 1; 
        });
        
        // Release hydrogen as part of complete molecule formation
        releaseHydrogen();
        
        // Decrement count - atomic operation protected by mutex
        hydrogen_count--;
        
        // Check if molecule is complete (both hydrogens released)
        if (hydrogen_count == 0 && oxygen_count == 1) {
            oxygen_count--;
            cv.notify_all(); // Wake up waiting threads for next molecule
        }
    }
    
    // Thread-safe oxygen release with synchronization
    void oxygen(std::function<void()> releaseOxygen) {
        std::unique_lock<std::mutex> lock(mtx);
        
        // Increment oxygen count - validates molecule formation constraint
        oxygen_count++;
        
        // Wait until we have at least 2 hydrogens to form a complete molecule
        // This prevents oxygen from proceeding without sufficient hydrogen
        cv.wait(lock, [this]() { 
            return hydrogen_count >= 2; 
        });
        
        // Release oxygen as part of complete molecule formation
        releaseOxygen();
        
        // Notify all waiting threads that oxygen is ready
        // Hydrogen threads will check if complete molecule can be formed
        cv.notify_all();
    }
    
    std::string getResult() const {
        return result;
    }
    
    void appendResult(char c) {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(mtx));
        result += c;
    }
};

// Test function with proper input validation
std::string buildMolecule(const std::string& water) {
    // Input validation: check for null/empty input
    if (water.empty()) {
        return "";
    }
    
    // Input validation: verify length constraint (3*n)
    if (water.length() % 3 != 0) {
        return "";
    }
    
    H2O h2o;
    std::vector<std::thread> threads;
    
    // Count validation: ensure correct H/O ratio before processing
    int h_count = 0, o_count = 0;
    for (char c : water) {
        if (c == 'H') h_count++;
        else if (c == 'O') o_count++;
        else return ""; // Invalid character detected
    }
    
    // Validate constraint: exactly 2*n H and n O
    if (h_count != 2 * o_count) {
        return "";
    }
    
    // Create threads for each atom with bounds-checked iteration
    for (size_t i = 0; i < water.length(); i++) {
        // Bounds check: ensure index is within valid range
        if (i >= water.length()) break;
        
        char c = water[i];
        if (c == 'H') {
            threads.emplace_back([&h2o]() {
                h2o.hydrogen([&h2o]() { h2o.appendResult('H'); });
            });
        } else if (c == 'O') {
            threads.emplace_back([&h2o]() {
                h2o.oxygen([&h2o]() { h2o.appendResult('O'); });
            });
        }
    }
    
    // Wait for all threads to complete - ensures all molecules formed
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
    
    return h2o.getResult();
}

int main() {
    // Test case 1: Basic HOH input
    std::cout << "Test 1: " << buildMolecule("HOH") << std::endl;
    
    // Test case 2: Multiple molecules
    std::cout << "Test 2: " << buildMolecule("OOHHHH") << std::endl;
    
    // Test case 3: Single molecule
    std::cout << "Test 3: " << buildMolecule("HHO") << std::endl;
    
    // Test case 4: Larger input
    std::cout << "Test 4: " << buildMolecule("HHHHHHOOO") << std::endl;
    
    // Test case 5: Edge case - minimum valid input
    std::cout << "Test 5: " << buildMolecule("OHH") << std::endl;
    
    return 0;
}
