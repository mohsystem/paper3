
#include <iostream>
#include <vector>
using namespace std;

class Task192 {
public:
    static bool xorGame(vector<int>& nums) {
        // Input validation
        if (nums.empty()) {
            return false;
        }
        
        // Calculate XOR of all elements
        int xorValue = 0;
        for (int num : nums) {
            xorValue ^= num;
        }
        
        // Alice wins if XOR is 0 or array length is even
        return xorValue == 0 || nums.size() % 2 == 0;
    }
};

int main() {
    // Test case 1
    vector<int> test1 = {1, 1, 2};
    cout << "Test 1: " << (Task192::xorGame(test1) ? "true" : "false") << endl;
    
    // Test case 2
    vector<int> test2 = {0, 1};
    cout << "Test 2: " << (Task192::xorGame(test2) ? "true" : "false") << endl;
    
    // Test case 3
    vector<int> test3 = {1, 2, 3};
    cout << "Test 3: " << (Task192::xorGame(test3) ? "true" : "false") << endl;
    
    // Test case 4
    vector<int> test4 = {1};
    cout << "Test 4: " << (Task192::xorGame(test4) ? "true" : "false") << endl;
    
    // Test case 5
    vector<int> test5 = {2, 2};
    cout << "Test 5: " << (Task192::xorGame(test5) ? "true" : "false") << endl;
    
    return 0;
}
