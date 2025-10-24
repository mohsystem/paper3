#include <iostream>
#include <vector>
#include <string>

/**
 * Checks if a given walk is valid.
 * A valid walk takes exactly 10 minutes (10 steps) and returns to the starting point.
 *
 * @param walk A constant reference to a vector of characters representing directions.
 * @return true if the walk is valid, false otherwise.
 */
bool isValidWalk(const std::vector<char>& walk) {
    // The walk must be exactly 10 minutes long.
    if (walk.size() != 10) {
        return false;
    }

    int x = 0; // Represents horizontal displacement (East/West)
    int y = 0; // Represents vertical displacement (North/South)

    for (char direction : walk) {
        switch (direction) {
            case 'n': y++; break;
            case 's': y--; break;
            case 'e': x++; break;
            case 'w': x--; break;
            // According to the prompt, we don't need to handle other cases.
        }
    }

    // The walk must return to the starting point (0,0).
    return x == 0 && y == 0;
}

int main() {
    // Test Case 1: Valid walk
    std::vector<char> walk1 = {'n', 's', 'n', 's', 'n', 's', 'n', 's', 'n', 's'};
    std::cout << "Test Case 1: " << (isValidWalk(walk1) ? "true" : "false") << std::endl; // Expected: true

    // Test Case 2: Too long walk
    std::vector<char> walk2 = {'w', 'e', 'w', 'e', 'w', 'e', 'w', 'e', 'w', 'e', 'w', 'e'};
    std::cout << "Test Case 2: " << (isValidWalk(walk2) ? "true" : "false") << std::endl; // Expected: false

    // Test Case 3: Too short walk
    std::vector<char> walk3 = {'w'};
    std::cout << "Test Case 3: " << (isValidWalk(walk3) ? "true" : "false") << std::endl; // Expected: false

    // Test Case 4: Correct length, but does not return to start
    std::vector<char> walk4 = {'n', 'n', 'n', 's', 'n', 's', 'n', 's', 'n', 's'};
    std::cout << "Test Case 4: " << (isValidWalk(walk4) ? "true" : "false") << std::endl; // Expected: false

    // Test Case 5: Valid walk with mixed directions
    std::vector<char> walk5 = {'e', 'w', 'e', 'w', 'n', 's', 'n', 's', 'e', 'w'};
    std::cout << "Test Case 5: " << (isValidWalk(walk5) ? "true" : "false") << std::endl; // Expected: true

    return 0;
}