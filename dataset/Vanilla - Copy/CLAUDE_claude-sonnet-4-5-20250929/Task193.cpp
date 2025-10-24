
#include <iostream>
#include <vector>
using namespace std;

class Task193 {
public:
    static bool xorGame(vector<int>& nums) {
        int xorVal = 0;
        for (int num : nums) {
            xorVal ^= num;
        }
        return xorVal == 0 || nums.size() % 2 == 0;
    }
};

int main() {
    // Test case 1
    vector<int> test1 = {1, 1, 2};
    cout << (Task193::xorGame(test1) ? "true" : "false") << endl;
    
    // Test case 2
    vector<int> test2 = {0, 1};
    cout << (Task193::xorGame(test2) ? "true" : "false") << endl;
    
    // Test case 3
    vector<int> test3 = {1, 2, 3};
    cout << (Task193::xorGame(test3) ? "true" : "false") << endl;
    
    // Test case 4
    vector<int> test4 = {1, 1};
    cout << (Task193::xorGame(test4) ? "true" : "false") << endl;
    
    // Test case 5
    vector<int> test5 = {2, 2, 2, 2};
    cout << (Task193::xorGame(test5) ? "true" : "false") << endl;
    
    return 0;
}
