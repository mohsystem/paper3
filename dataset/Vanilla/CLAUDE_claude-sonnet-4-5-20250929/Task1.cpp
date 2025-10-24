
#include <iostream>
#include <vector>

using namespace std;

int countPassengers(vector<vector<int>> busStops) {
    int peopleOnBus = 0;
    
    for (int i = 0; i < busStops.size(); i++) {
        peopleOnBus += busStops[i][0]; // people getting on
        peopleOnBus -= busStops[i][1]; // people getting off
    }
    
    return peopleOnBus;
}

int main() {
    // Test case 1
    vector<vector<int>> test1 = {{10, 0}, {3, 5}, {5, 8}};
    cout << "Test 1: " << countPassengers(test1) << endl; // Expected: 5
    
    // Test case 2
    vector<vector<int>> test2 = {{3, 0}, {9, 1}, {4, 10}, {12, 2}, {6, 1}, {7, 10}};
    cout << "Test 2: " << countPassengers(test2) << endl; // Expected: 17
    
    // Test case 3
    vector<vector<int>> test3 = {{3, 0}, {9, 1}, {4, 8}, {12, 2}, {6, 1}, {7, 8}};
    cout << "Test 3: " << countPassengers(test3) << endl; // Expected: 21
    
    // Test case 4
    vector<vector<int>> test4 = {{0, 0}};
    cout << "Test 4: " << countPassengers(test4) << endl; // Expected: 0
    
    // Test case 5
    vector<vector<int>> test5 = {{5, 0}, {2, 3}, {4, 1}};
    cout << "Test 5: " << countPassengers(test5) << endl; // Expected: 7
    
    return 0;
}
