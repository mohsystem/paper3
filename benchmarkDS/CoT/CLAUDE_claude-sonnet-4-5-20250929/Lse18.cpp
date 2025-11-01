
#include <iostream>
#include <vector>
using namespace std;

class Lse18 {
private:
    static vector<int> array;
    
public:
    static int* getValueAtIndex(int index) {
        // Input validation: check if index is negative
        if (index < 0) {
            cout << "Error: Index cannot be negative" << endl;
            return nullptr;
        }
        
        // Boundary check: ensure index is within array bounds
        if (index >= array.size()) {
            cout << "Error: Index out of bounds" << endl;
            return nullptr;
        }
        
        return &array[index];
    }
};

vector<int> Lse18::array = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

int main() {
    // Test case 1: Valid index
    int* result1 = Lse18::getValueAtIndex(5);
    cout << "Test 1 - Index 5: " << (result1 ? to_string(*result1) : "null") << endl;
    
    // Test case 2: First element
    int* result2 = Lse18::getValueAtIndex(0);
    cout << "Test 2 - Index 0: " << (result2 ? to_string(*result2) : "null") << endl;
    
    // Test case 3: Last element
    int* result3 = Lse18::getValueAtIndex(9);
    cout << "Test 3 - Index 9: " << (result3 ? to_string(*result3) : "null") << endl;
    
    // Test case 4: Out of bounds
    int* result4 = Lse18::getValueAtIndex(15);
    cout << "Test 4 - Index 15: " << (result4 ? to_string(*result4) : "null") << endl;
    
    // Test case 5: Negative index
    int* result5 = Lse18::getValueAtIndex(-1);
    cout << "Test 5 - Index -1: " << (result5 ? to_string(*result5) : "null") << endl;
    
    return 0;
}
