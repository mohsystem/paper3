
#include <vector>
#include <utility>
#include <stdexcept>
#include <iostream>
#include <climits>

// Security: Validate input array to prevent integer overflow and ensure non-negative people count
// Takes a vector of pairs (people_on, people_off) and returns final count on bus
int numberOfPeopleOnBus(const std::vector<std::pair<int, int>>& busStops) {
    // Security: Validate input is not empty
    if (busStops.empty()) {
        throw std::invalid_argument("Bus stops array cannot be empty");
    }
    
    // Security: Validate first stop has 0 people getting off
    if (busStops[0].second != 0) {
        throw std::invalid_argument("First bus stop must have 0 people getting off");
    }
    
    int peopleOnBus = 0;
    
    // Process each bus stop
    for (size_t i = 0; i < busStops.size(); ++i) {
        const int peopleOn = busStops[i].first;
        const int peopleOff = busStops[i].second;
        
        // Security: Validate non-negative values for people getting on/off
        if (peopleOn < 0 || peopleOff < 0) {
            throw std::invalid_argument("People count cannot be negative");
        }
        
        // Security: Check for integer overflow when adding people on
        if (peopleOnBus > INT_MAX - peopleOn) {
            throw std::overflow_error("Integer overflow detected when adding people");
        }
        
        // Security: Ensure we don't have more people getting off than on bus
        if (peopleOff > peopleOnBus + peopleOn) {
            throw std::invalid_argument("Cannot have more people getting off than on bus");
        }
        
        // Update bus count: add people on, subtract people off
        peopleOnBus += peopleOn;
        peopleOnBus -= peopleOff;
        
        // Security: Ensure count never goes negative (defensive check)
        if (peopleOnBus < 0) {
            throw std::logic_error("Bus people count became negative");
        }
    }
    
    return peopleOnBus;
}

int main() {
    // Test case 1: Simple scenario
    std::vector<std::pair<int, int>> test1 = {{10, 0}, {3, 5}, {5, 8}};
    std::cout << "Test 1: " << numberOfPeopleOnBus(test1) << " (expected: 5)" << std::endl;
    
    // Test case 2: Everyone gets off
    std::vector<std::pair<int, int>> test2 = {{3, 0}, {9, 1}, {4, 10}, {12, 2}, {6, 1}, {7, 10}};
    std::cout << "Test 2: " << numberOfPeopleOnBus(test2) << " (expected: 17)" << std::endl;
    
    // Test case 3: Single stop
    std::vector<std::pair<int, int>> test3 = {{5, 0}};
    std::cout << "Test 3: " << numberOfPeopleOnBus(test3) << " (expected: 5)" << std::endl;
    
    // Test case 4: Multiple stops with various on/off
    std::vector<std::pair<int, int>> test4 = {{10, 0}, {3, 5}, {2, 5}};
    std::cout << "Test 4: " << numberOfPeopleOnBus(test4) << " (expected: 5)" << std::endl;
    
    // Test case 5: No one gets off except last stop
    std::vector<std::pair<int, int>> test5 = {{1, 0}, {2, 0}, {3, 0}, {4, 10}};
    std::cout << "Test 5: " << numberOfPeopleOnBus(test5) << " (expected: 0)" << std::endl;
    
    return 0;
}
