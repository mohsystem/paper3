
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

class Task21 {
public:
    static vector<int> removeSmallest(vector<int> numbers) {
        if (numbers.empty()) {
            return vector<int>();
        }
        
        vector<int> result = numbers;
        int minIndex = 0;
        int minValue = numbers[0];
        
        for (size_t i = 1; i < numbers.size(); i++) {
            if (numbers[i] < minValue) {
                minValue = numbers[i];
                minIndex = i;
            }
        }
        
        result.erase(result.begin() + minIndex);
        return result;
    }
};

void printVector(const vector<int>& vec) {
    cout << "[";
    for (size_t i = 0; i < vec.size(); i++) {
        cout << vec[i];
        if (i < vec.size() - 1) cout << ", ";
    }
    cout << "]";
}

int main() {
    // Test case 1
    vector<int> test1 = {1, 2, 3, 4, 5};
    cout << "Input: "; printVector(test1);
    cout << ", Output: "; printVector(Task21::removeSmallest(test1));
    cout << endl;
    
    // Test case 2
    vector<int> test2 = {5, 3, 2, 1, 4};
    cout << "Input: "; printVector(test2);
    cout << ", Output: "; printVector(Task21::removeSmallest(test2));
    cout << endl;
    
    // Test case 3
    vector<int> test3 = {2, 2, 1, 2, 1};
    cout << "Input: "; printVector(test3);
    cout << ", Output: "; printVector(Task21::removeSmallest(test3));
    cout << endl;
    
    // Test case 4
    vector<int> test4 = {};
    cout << "Input: "; printVector(test4);
    cout << ", Output: "; printVector(Task21::removeSmallest(test4));
    cout << endl;
    
    // Test case 5
    vector<int> test5 = {10};
    cout << "Input: "; printVector(test5);
    cout << ", Output: "; printVector(Task21::removeSmallest(test5));
    cout << endl;
    
    return 0;
}
