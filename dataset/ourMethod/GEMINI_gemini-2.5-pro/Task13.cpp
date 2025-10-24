#include <iostream>
#include <vector>
#include <stdbool.h>

/**
 * @brief Checks if a given walk is valid.
 * A valid walk takes exactly 10 minutes (10 steps) and returns to the starting point.
 * @param walk A vector of characters representing the directions of the walk ('n', 's', 'e', 'w').
 * @return true if the walk is valid, false otherwise.
 */
bool isValidWalk(const std::vector<char>& walk) {
    // A valid walk must be exactly 10 minutes long.
    if (walk.size() != 10) {
        return false;
    }

    int x = 0; // Represents horizontal position (East/West)
    int y = 0; // Represents vertical position (North/South)

    // Iterate through each step of the walk and update coordinates.
    for (char direction : walk) {
        switch (direction) {
            case 'n': y++; break;
            case 's': y--; break;
            case 'e': x++; break;
            case 'w': x--; break;
            // Per problem description, only valid characters will be in the input.
        }
    }

    // The walk is valid if it returns to the origin (0,0).
    return x == 0 && y == 0;
}

int main() {
    // Test Case 1: Valid walk
    std::vector<char> walk1 = {'n','s','n','s','n','s','n','s','n','s'};
    std::cout << "Test 1: {'n','s','n','s','n','s','n','s','n','s'} -> " 
              << (isValidWalk(walk1) ? "true" : "false") << std::endl;

    // Test Case 2: Invalid walk (too long)
    std::vector<char> walk2 = {'w','e','w','e','w','e','w','e','w','e','w','e'};
    std::cout << "Test 2: {'w','e','w','e','w','e','w','e','w','e','w','e'} -> " 
              << (isValidWalk(walk2) ? "true" : "false") << std::endl;

    // Test Case 3: Invalid walk (too short)
    std::vector<char> walk3 = {'w'};
    std::cout << "Test 3: {'w'} -> " 
              << (isValidWalk(walk3) ? "true" : "false") << std::endl;

    // Test Case 4: Invalid walk (10 minutes but doesn't return to start)
    std::vector<char> walk4 = {'n','n','n','s','n','s','n','s','n','s'};
    std::cout << "Test 4: {'n','n','n','s','n','s','n','s','n','s'} -> " 
              << (isValidWalk(walk4) ? "true" : "false") << std::endl;

    // Test Case 5: Valid walk
    std::vector<char> walk5 = {'e', 'w', 'e', 'w', 'n', 's', 'n', 's', 'e', 'w'};
    std::cout << "Test 5: {'e', 'w', 'e', 'w', 'n', 's', 'n', 's', 'e', 'w'} -> " 
              << (isValidWalk(walk5) ? "true" : "false") << std::endl;

    return 0;
}