
#include <iostream>
#include <vector>
using namespace std;

bool isValidWalk(const vector<char>& walk) {
    // Check if the walk takes exactly 10 minutes
    if (walk.size() != 10) {
        return false;
    }
    
    // Track position on grid
    int x = 0;
    int y = 0;
    
    // Process each direction
    for (size_t i = 0; i < walk.size(); i++) {
        char direction = walk[i];
        
        // Validate that only allowed directions are present
        if (direction != 'n' && direction != 's' && 
            direction != 'e' && direction != 'w') {
            return false;
        }
        
        // Update position based on direction
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
        }
    }
    
    // Check if we returned to starting point
    return x == 0 && y == 0;
}

int main() {
    // Test case 1: Valid 10-minute walk returning to start
    vector<char> test1 = {'n', 's', 'n', 's', 'n', 's', 'n', 's', 'n', 's'};
    cout << "Test 1: " << (isValidWalk(test1) ? "true" : "false") << endl;
    
    // Test case 2: Valid 10-minute walk with all directions
    vector<char> test2 = {'n', 's', 'e', 'w', 'n', 's', 'e', 'w', 'n', 's'};
    cout << "Test 2: " << (isValidWalk(test2) ? "true" : "false") << endl;
    
    // Test case 3: Too short walk
    vector<char> test3 = {'n', 's', 'e', 'w'};
    cout << "Test 3: " << (isValidWalk(test3) ? "true" : "false") << endl;
    
    // Test case 4: Correct length but doesn't return to start\n    vector<char> test4 = {'n', 'n', 'n', 's', 's', 's', 'e', 'w', 'n', 's'};\n    cout << "Test 4: " << (isValidWalk(test4) ? "true" : "false") << endl;\n    \n    // Test case 5: Too long walk\n    vector<char> test5 = {'n', 's', 'n', 's', 'n', 's', 'n', 's', 'n', 's', 'n', 's'};
    cout << "Test 5: " << (isValidWalk(test5) ? "true" : "false") << endl;
    
    return 0;
}
