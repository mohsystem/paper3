
#include <iostream>
#include <vector>
using namespace std;

class Task13 {
public:
    static bool isValidWalk(vector<char> walk) {
        if (walk.size() != 10) {
            return false;
        }
        
        int x = 0, y = 0;
        
        for (char direction : walk) {
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
        
        return x == 0 && y == 0;
    }
};

int main() {
    // Test case 1: Valid walk
    vector<char> test1 = {'n', 's', 'n', 's', 'n', 's', 'n', 's', 'n', 's'};
    cout << "Test 1: " << (Task13::isValidWalk(test1) ? "true" : "false") << endl;
    
    // Test case 2: Valid walk with all directions
    vector<char> test2 = {'n', 's', 'e', 'w', 'n', 's', 'e', 'w', 'n', 's'};
    cout << "Test 2: " << (Task13::isValidWalk(test2) ? "true" : "false") << endl;
    
    // Test case 3: Too short
    vector<char> test3 = {'n', 's', 'n', 's', 'n'};
    cout << "Test 3: " << (Task13::isValidWalk(test3) ? "true" : "false") << endl;
    
    // Test case 4: Doesn't return to start\n    vector<char> test4 = {'n', 'n', 'n', 's', 'n', 's', 'n', 's', 'n', 's'};\n    cout << "Test 4: " << (Task13::isValidWalk(test4) ? "true" : "false") << endl;\n    \n    // Test case 5: Valid circular walk\n    vector<char> test5 = {'n', 'e', 'n', 'e', 's', 'w', 's', 'w', 'n', 's'};
    cout << "Test 5: " << (Task13::isValidWalk(test5) ? "true" : "false") << endl;
    
    return 0;
}
