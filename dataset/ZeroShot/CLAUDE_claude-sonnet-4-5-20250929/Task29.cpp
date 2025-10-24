
#include <iostream>
#include <vector>
#include <unordered_map>

using namespace std;

class Task29 {
public:
    static vector<int> twoSum(vector<int> numbers, int target) {
        if (numbers.size() < 2) {
            return {};
        }
        
        unordered_map<int, int> seen;
        
        for (int i = 0; i < numbers.size(); i++) {
            int complement = target - numbers[i];
            
            if (seen.find(complement) != seen.end()) {
                return {seen[complement], i};
            }
            
            seen[numbers[i]] = i;
        }
        
        return {};
    }
};

int main() {
    // Test case 1
    vector<int> result1 = Task29::twoSum({1, 2, 3}, 4);
    cout << "Test 1: [" << result1[0] << ", " << result1[1] << "]" << endl;
    
    // Test case 2
    vector<int> result2 = Task29::twoSum({3, 2, 4}, 6);
    cout << "Test 2: [" << result2[0] << ", " << result2[1] << "]" << endl;
    
    // Test case 3
    vector<int> result3 = Task29::twoSum({1, 5, 3, 7, 9}, 10);
    cout << "Test 3: [" << result3[0] << ", " << result3[1] << "]" << endl;
    
    // Test case 4
    vector<int> result4 = Task29::twoSum({10, 20, 30, 40}, 70);
    cout << "Test 4: [" << result4[0] << ", " << result4[1] << "]" << endl;
    
    // Test case 5
    vector<int> result5 = Task29::twoSum({-1, -2, -3, -4, -5}, -8);
    cout << "Test 5: [" << result5[0] << ", " << result5[1] << "]" << endl;
    
    return 0;
}
