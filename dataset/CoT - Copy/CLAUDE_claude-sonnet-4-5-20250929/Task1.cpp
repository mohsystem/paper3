
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

class Task1 {
public:
    static int countPassengers(const vector<vector<int>>& busStops) {
        // Input validation
        if (busStops.empty()) {
            return 0;
        }
        
        int peopleOnBus = 0;
        
        // Process each bus stop
        for (size_t i = 0; i < busStops.size(); i++) {
            // Validate array structure
            if (busStops[i].size() != 2) {
                continue;
            }
            
            // Validate non-negative values
            int getOn = max(0, busStops[i][0]);
            int getOff = max(0, busStops[i][1]);
            
            // Update passenger count
            peopleOnBus += getOn;
            peopleOnBus -= getOff;
            
            // Ensure non-negative result
            peopleOnBus = max(0, peopleOnBus);
        }
        
        return peopleOnBus;
    }
};

int main() {
    // Test case 1
    vector<vector<int>> test1 = {{10, 0}, {3, 5}, {5, 8}};
    cout << "Test 1: " << Task1::countPassengers(test1) << endl; // Expected: 5
    
    // Test case 2
    vector<vector<int>> test2 = {{3, 0}, {9, 1}, {4, 10}, {12, 2}, {6, 1}, {7, 10}};
    cout << "Test 2: " << Task1::countPassengers(test2) << endl; // Expected: 17
    
    // Test case 3
    vector<vector<int>> test3 = {{3, 0}, {9, 1}, {4, 8}, {12, 2}, {6, 1}, {7, 8}};
    cout << "Test 3: " << Task1::countPassengers(test3) << endl; // Expected: 21
    
    // Test case 4
    vector<vector<int>> test4 = {{0, 0}};
    cout << "Test 4: " << Task1::countPassengers(test4) << endl; // Expected: 0
    
    // Test case 5
    vector<vector<int>> test5 = {{5, 0}, {2, 3}, {1, 4}};
    cout << "Test 5: " << Task1::countPassengers(test5) << endl; // Expected: 1
    
    return 0;
}
