#include <iostream>
#include <vector>

bool isValidWalk(const std::vector<char>& walk) {
    // Validate length: must be exactly 10 minutes/steps
    if (walk.size() != 10U) {
        return false;
    }

    int x = 0;
    int y = 0;

    for (char dir : walk) {
        switch (dir) {
            case 'n': y += 1; break;
            case 's': y -= 1; break;
            case 'e': x += 1; break;
            case 'w': x -= 1; break;
            default:  // Invalid direction
                return false;
        }
    }

    return x == 0 && y == 0;
}

int main() {
    // Test case 1: Valid 10-minute walk returning to start
    std::vector<char> t1 = {'n','s','n','s','n','s','n','s','n','s'};
    std::cout << "Test 1: " << (isValidWalk(t1) ? "true" : "false") << '\n';

    // Test case 2: Less than 10 minutes
    std::vector<char> t2 = {'n','s','n','s','n','s','n','s'};
    std::cout << "Test 2: " << (isValidWalk(t2) ? "true" : "false") << '\n';

    // Test case 3: 10 minutes but does not return to start
    std::vector<char> t3 = {'n','n','n','n','n','s','s','s','s','e'};
    std::cout << "Test 3: " << (isValidWalk(t3) ? "true" : "false") << '\n';

    // Test case 4: Contains invalid character
    std::vector<char> t4 = {'n','s','x','s','n','s','n','s','n','s'};
    std::cout << "Test 4: " << (isValidWalk(t4) ? "true" : "false") << '\n';

    // Test case 5: Another valid 10-minute walk returning to start
    std::vector<char> t5 = {'e','w','e','w','n','s','n','s','e','w'};
    std::cout << "Test 5: " << (isValidWalk(t5) ? "true" : "false") << '\n';

    return 0;
}