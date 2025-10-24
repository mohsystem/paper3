#include <iostream>
#include <vector>
#include <utility>
#include <limits>

using ll = long long;

static bool will_add_overflow_ll(ll a, ll b) {
    if (b > 0 && a > std::numeric_limits<ll>::max() - b) return true;
    if (b < 0 && a < std::numeric_limits<ll>::min() - b) return true;
    return false;
}

// Computes the number of people remaining on the bus after the last stop.
// Returns -1 on invalid input (negative values, underflow, or overflow).
ll people_on_bus(const std::vector<std::pair<int,int>>& stops) {
    ll occupants = 0;
    for (size_t i = 0; i < stops.size(); ++i) {
        int on = stops[i].first;
        int off = stops[i].second;

        if (on < 0 || off < 0) {
            return -1; // invalid numbers
        }

        // Check addition overflow
        if (will_add_overflow_ll(occupants, static_cast<ll>(on))) {
            return -1;
        }
        occupants += static_cast<ll>(on);

        // Validate we don't go negative (more off than current occupants)
        if (static_cast<ll>(off) > occupants) {
            return -1;
        }

        occupants -= static_cast<ll>(off);
    }
    return occupants;
}

int main() {
    // Test case 1
    std::vector<std::pair<int,int>> t1 = { {10,0}, {3,5}, {5,8} };
    std::cout << "Test 1: " << people_on_bus(t1) << "\n"; // Expected: 5

    // Test case 2
    std::vector<std::pair<int,int>> t2 = { {3,0}, {9,1}, {4,10}, {12,2}, {6,1}, {7,10} };
    std::cout << "Test 2: " << people_on_bus(t2) << "\n"; // Expected: 17

    // Test case 3 - empty stops
    std::vector<std::pair<int,int>> t3 = {};
    std::cout << "Test 3: " << people_on_bus(t3) << "\n"; // Expected: 0

    // Test case 4 - invalid: negative off
    std::vector<std::pair<int,int>> t4 = { {5,0}, {3,-1} };
    std::cout << "Test 4: " << people_on_bus(t4) << "\n"; // Expected: -1

    // Test case 5 - invalid: more people off than on+current (underflow)
    std::vector<std::pair<int,int>> t5 = { {0,0}, {0,1} };
    std::cout << "Test 5: " << people_on_bus(t5) << "\n"; // Expected: -1

    return 0;
}