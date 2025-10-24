
#include <iostream>
#include <vector>

using namespace std;

class Task35 {
public:
    static int countPassengers(vector<vector<int>> busStops) {
        int peopleOnBus = 0;
        
        for (int i = 0; i < busStops.size(); i++) {
            peopleOnBus += busStops[i][0]; // people getting on
            peopleOnBus -= busStops[i][1]; // people getting off
        }
        
        return peopleOnBus;
    }
};

int main() {
    // Test case 1
    vector<vector<int>> test1 = {{10, 0}, {3, 5}, {5, 8}};
    cout << "Test 1: " << Task35::countPassengers(test1) << endl; // Expected: 5
    
    // Test case 2
    vector<vector<int>> test2 = {{3, 0}, {9, 1}, {4, 10}, {12, 2}, {6, 1}, {7, 10}};
    cout << "Test 2: " << Task35::countPassengers(test2) << endl; // Expected: 17
    
    // Test case 3
    vector<vector<int>> test3 = {{3, 0}, {9, 1}, {4, 8}, {12, 2}, {6, 1}, {7, 8}};
    cout << "Test 3: " << Task35::countPassengers(test3) << endl; // Expected: 21
    
    // Test case 4
    vector<vector<int>> test4 = {{0, 0}};
    cout << "Test 4: " << Task35::countPassengers(test4) << endl; // Expected: 0
    
    // Test case 5
    vector<vector<int>> test5 = {{5, 0}, {2, 3}, {10, 5}, {4, 8}};
    cout << "Test 5: " << Task35::countPassengers(test5) << endl; // Expected: 5
    
    return 0;
}
