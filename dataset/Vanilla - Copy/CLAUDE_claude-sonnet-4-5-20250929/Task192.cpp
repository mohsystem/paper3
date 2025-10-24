
#include <iostream>
#include <vector>
using namespace std;

bool xorGame(vector<int>& nums) {
    int xorSum = 0;
    for (int num : nums) {
        xorSum ^= num;
    }
    return xorSum == 0 || nums.size() % 2 == 0;
}

int main() {
    // Test case 1
    vector<int> test1 = {1, 1, 2};
    cout << (xorGame(test1) ? "true" : "false") << endl;
    
    // Test case 2
    vector<int> test2 = {0, 1};
    cout << (xorGame(test2) ? "true" : "false") << endl;
    
    // Test case 3
    vector<int> test3 = {1, 2, 3};
    cout << (xorGame(test3) ? "true" : "false") << endl;
    
    // Test case 4
    vector<int> test4 = {1, 1, 1, 1};
    cout << (xorGame(test4) ? "true" : "false") << endl;
    
    // Test case 5
    vector<int> test5 = {5, 3, 6};
    cout << (xorGame(test5) ? "true" : "false") << endl;
    
    return 0;
}
