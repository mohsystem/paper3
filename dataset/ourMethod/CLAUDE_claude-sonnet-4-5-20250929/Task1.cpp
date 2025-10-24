
#include <iostream>
#include <vector>
#include <utility>
#include <stdexcept>
#include <limits>

// Function to calculate the number of people remaining on the bus
// Parameters:
//   stops: vector of pairs representing (people getting on, people getting off) at each stop
// Returns: number of people still on the bus after the last stop
// Security: validates all inputs to ensure non-negative values and prevent integer overflow
int peopleOnBus(const std::vector<std::pair<int, int>>& stops) {
    // Validate input: empty list means no stops, return 0
    if (stops.empty()) {
        return 0;
    }
    
    // Validate first stop: people getting off must be 0 (bus starts empty)
    if (stops[0].second != 0) {
        throw std::invalid_argument("First stop must have 0 people getting off");
    }
    
    int peopleOnBus = 0;
    
    for (size_t i = 0; i < stops.size(); ++i) {
        int on = stops[i].first;
        int off = stops[i].second;
        
        // Validate input: both values must be non-negative
        if (on < 0 || off < 0) {
            throw std::invalid_argument("Number of people cannot be negative");
        }
        
        // Check for integer overflow when adding people getting on
        if (peopleOnBus > std::numeric_limits<int>::max() - on) {
            throw std::overflow_error("Integer overflow: too many people on bus");
        }
        
        // Add people getting on
        peopleOnBus += on;
        
        // Validate: people getting off cannot exceed people on bus
        if (off > peopleOnBus) {
            throw std::invalid_argument("More people getting off than on bus");
        }
        
        // Remove people getting off
        peopleOnBus -= off;
        
        // Sanity check: ensure non-negative result (should always be true due to prior checks)
        if (peopleOnBus < 0) {
            throw std::logic_error("Invalid state: negative people count");
        }
    }
    
    return peopleOnBus;
}

int main() {
    // Test case 1: Basic scenario
    std::vector<std::pair<int, int>> test1 = {{10, 0}, {3, 5}, {5, 8}};
    std::cout << "Test 1: " << peopleOnBus(test1) << " (Expected: 5)" << std::endl;
    
    // Test case 2: Everyone gets off
    std::vector<std::pair<int, int>> test2 = {{3, 0}, {9, 1}, {4, 10}, {12, 2}, {6, 1}, {7, 10}};
    std::cout << "Test 2: " << peopleOnBus(test2) << " (Expected: 17)" << std::endl;
    
    // Test case 3: Single stop
    std::vector<std::pair<int, int>> test3 = {{5, 0}};
    std::cout << "Test 3: " << peopleOnBus(test3) << " (Expected: 5)" << std::endl;
    
    // Test case 4: Multiple stops with varying numbers
    std::vector<std::pair<int, int>> test4 = {{10, 0}, {3, 5}, {2, 5}};
    std::cout << "Test 4: " << peopleOnBus(test4) << " (Expected: 5)" << std::endl;
    
    // Test case 5: Empty bus scenario
    std::vector<std::pair<int, int>> test5 = {{0, 0}};
    std::cout << "Test 5: " << peopleOnBus(test5) << " (Expected: 0)" << std::endl;
    
    return 0;
}
