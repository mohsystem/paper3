
#include <iostream>
#include <vector>
#include <string>
#include <cstddef>

// Function to validate if a 10-minute walk returns to starting point
// Security: Validates input size and characters to prevent undefined behavior
// Returns true if walk takes exactly 10 minutes and returns to origin
bool isValidWalk(const std::vector<std::string>& walk) {
    // Validate input: must be exactly 10 directions for 10 minutes
    // Security: Check size to prevent processing invalid input
    if (walk.size() != 10) {
        return false;
    }
    
    // Track position on grid: x (east/west), y (north/south)
    int x = 0;
    int y = 0;
    
    // Process each direction
    for (size_t i = 0; i < walk.size(); ++i) {
        // Security: Validate each direction string is not empty
        if (walk[i].empty()) {
            return false;
        }
        
        // Security: Validate only single character directions
        if (walk[i].length() != 1) {
            return false;
        }
        
        char direction = walk[i][0];
        
        // Security: Validate direction is one of expected values
        // Update position based on validated direction
        switch (direction) {
            case 'n':
                y++;
                break;
            case 's':
                y--;
                break;
            case 'e':
                x++;
                break;
            case 'w':
                x--;
                break;
            default:
                // Security: Reject invalid direction characters
                return false;
        }
    }
    
    // Check if we returned to starting point (0, 0)
    return (x == 0 && y == 0);
}

int main() {
    // Test case 1: Valid 10-minute walk returning to origin
    std::vector<std::string> test1 = {"n", "s", "n", "s", "n", "s", "n", "s", "n", "s"};
    std::cout << "Test 1: " << (isValidWalk(test1) ? "true" : "false") << std::endl;
    
    // Test case 2: Valid walk with all directions
    std::vector<std::string> test2 = {"n", "e", "s", "w", "n", "e", "s", "w", "n", "s"};
    std::cout << "Test 2: " << (isValidWalk(test2) ? "true" : "false") << std::endl;
    
    // Test case 3: Too short (less than 10 minutes)
    std::vector<std::string> test3 = {"n", "s", "e", "w"};
    std::cout << "Test 3: " << (isValidWalk(test3) ? "true" : "false") << std::endl;
    
    // Test case 4: Correct length but doesn't return to origin
    std::vector<std::string> test4 = {"n", "n", "n", "n", "n", "n", "n", "n", "n", "n"};
    std::cout << "Test 4: " << (isValidWalk(test4) ? "true" : "false") << std::endl;
    
    // Test case 5: Too long (more than 10 minutes)
    std::vector<std::string> test5 = {"n", "s", "n", "s", "n", "s", "n", "s", "n", "s", "n", "s"};
    std::cout << "Test 5: " << (isValidWalk(test5) ? "true" : "false") << std::endl;
    
    return 0;
}
