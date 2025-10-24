
#include <iostream>
#include <vector>
#include <limits>

using namespace std;

vector<int> removeSmallest(const vector<int>& numbers) {
    // Handle empty vector
    if (numbers.empty()) {
        return vector<int>();
    }
    
    // Handle single element vector
    if (numbers.size() == 1) {
        return vector<int>();
    }
    
    // Find the minimum value and its first index
    int minValue = numbers[0];
    size_t minIndex = 0;
    
    for (size_t i = 1; i < numbers.size(); i++) {
        if (numbers[i] < minValue) {
            minValue = numbers[i];
            minIndex = i;
        }
    }
    
    // Create new vector without mutating original
    vector<int> result;
    result.reserve(numbers.size() - 1);
    
    for (size_t i = 0; i < numbers.size(); i++) {
        if (i != minIndex) {
            result.push_back(numbers[i]);
        }
    }
    
    return result;
}

void printVector(const vector<int>& vec) {
    cout << "[";
    for (size_t i = 0; i < vec.size(); i++) {
        cout << vec[i];
        if (i < vec.size() - 1) cout << ", ";
    }
    cout << "]" << endl;
}

int main() {
    // Test case 1
    vector<int> test1 = {1, 2, 3, 4, 5};
    cout << "Test 1: ";
    printVector(removeSmallest(test1));
    
    // Test case 2
    vector<int> test2 = {5, 3, 2, 1, 4};
    cout << "Test 2: ";
    printVector(removeSmallest(test2));
    
    // Test case 3
    vector<int> test3 = {2, 2, 1, 2, 1};
    cout << "Test 3: ";
    printVector(removeSmallest(test3));
    
    // Test case 4
    vector<int> test4 = {};
    cout << "Test 4: ";
    printVector(removeSmallest(test4));
    
    // Test case 5
    vector<int> test5 = {10};
    cout << "Test 5: ";
    printVector(removeSmallest(test5));
    
    return 0;
}
