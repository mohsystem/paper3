
#include <iostream>
#include <vector>
using namespace std;

class Task192 {
public:
    static bool xorGame(vector<int>& nums) {
        int xorSum = 0;
        for (int num : nums) {
            xorSum ^= num;
        }
        return xorSum == 0 || nums.size() % 2 == 0;
    }
};

int main() {
    vector<int> test1 = {1, 1, 2};
    cout << (Task192::xorGame(test1) ? "true" : "false") << endl; // false
    
    vector<int> test2 = {0, 1};
    cout << (Task192::xorGame(test2) ? "true" : "false") << endl; // true
    
    vector<int> test3 = {1, 2, 3};
    cout << (Task192::xorGame(test3) ? "true" : "false") << endl; // true
    
    vector<int> test4 = {1, 1};
    cout << (Task192::xorGame(test4) ? "true" : "false") << endl; // true
    
    vector<int> test5 = {2, 2, 2, 2};
    cout << (Task192::xorGame(test5) ? "true" : "false") << endl; // true
    
    return 0;
}
