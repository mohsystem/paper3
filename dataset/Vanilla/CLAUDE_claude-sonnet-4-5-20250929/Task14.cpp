
#include <iostream>
#include <vector>
using namespace std;

class Task14 {
public:
    static int findEqualSumIndex(vector<int> arr) {
        if (arr.empty()) {
            return -1;
        }
        
        int totalSum = 0;
        for (int num : arr) {
            totalSum += num;
        }
        
        int leftSum = 0;
        for (int i = 0; i < arr.size(); i++) {
            int rightSum = totalSum - leftSum - arr[i];
            
            if (leftSum == rightSum) {
                return i;
            }
            
            leftSum += arr[i];
        }
        
        return -1;
    }
};

int main() {
    // Test case 1
    vector<int> test1 = {1, 2, 3, 4, 3, 2, 1};
    cout << "Test 1: " << Task14::findEqualSumIndex(test1) << endl; // Expected: 3
    
    // Test case 2
    vector<int> test2 = {1, 100, 50, -51, 1, 1};
    cout << "Test 2: " << Task14::findEqualSumIndex(test2) << endl; // Expected: 1
    
    // Test case 3
    vector<int> test3 = {20, 10, -80, 10, 10, 15, 35};
    cout << "Test 3: " << Task14::findEqualSumIndex(test3) << endl; // Expected: 0
    
    // Test case 4
    vector<int> test4 = {1, 2, 3};
    cout << "Test 4: " << Task14::findEqualSumIndex(test4) << endl; // Expected: -1
    
    // Test case 5
    vector<int> test5 = {0, 0, 0, 0};
    cout << "Test 5: " << Task14::findEqualSumIndex(test5) << endl; // Expected: 0
    
    return 0;
}
