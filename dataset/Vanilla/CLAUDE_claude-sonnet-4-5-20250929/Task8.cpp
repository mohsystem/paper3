
#include <iostream>
#include <vector>
using namespace std;

char findMissingLetter(vector<char> array) {
    for (int i = 0; i < array.size() - 1; i++) {
        if (array[i + 1] - array[i] != 1) {
            return (char)(array[i] + 1);
        }
    }
    return ' ';
}

int main() {
    // Test case 1
    vector<char> test1 = {'a', 'b', 'c', 'd', 'f'};
    cout << "Test 1: " << findMissingLetter(test1) << endl; // Expected: e
    
    // Test case 2
    vector<char> test2 = {'O', 'Q', 'R', 'S'};
    cout << "Test 2: " << findMissingLetter(test2) << endl; // Expected: P
    
    // Test case 3
    vector<char> test3 = {'a', 'b', 'd', 'e'};
    cout << "Test 3: " << findMissingLetter(test3) << endl; // Expected: c
    
    // Test case 4
    vector<char> test4 = {'m', 'n', 'o', 'q', 'r'};
    cout << "Test 4: " << findMissingLetter(test4) << endl; // Expected: p
    
    // Test case 5
    vector<char> test5 = {'X', 'Z'};
    cout << "Test 5: " << findMissingLetter(test5) << endl; // Expected: Y
    
    return 0;
}
