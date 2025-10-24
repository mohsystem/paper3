
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
    vector<int> test1 = {1, 1, 2};
    cout << Task193::xorGame(test1) << endl; // 0 (false)
    
    vector<int> test2 = {0, 1};
    cout << Task193::xorGame(test2) << endl; // 1 (true)
    
    vector<int> test3 = {1, 2, 3};
    cout << Task193::xorGame(test3) << endl; // 1 (true)
    
    vector<int> test4 = {1};
    cout << Task193::xorGame(test4) << endl; // 1 (true)
    
    vector<int> test5 = {2, 2, 2, 2};
    cout << Task193::xorGame(test5) << endl; // 1 (true)
    
    return 0;
}
