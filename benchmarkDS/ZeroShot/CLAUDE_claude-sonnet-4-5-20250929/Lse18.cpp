
#include <iostream>
#include <vector>
using namespace std;

class Lse18 {
private:
    static const int arrayData[];
    static const int arraySize = 10;
    
public:
    static int getValueAtIndex(int index, bool& valid) {
        if (index < 0 || index >= arraySize) {
            valid = false;
            return -1; // Return -1 for invalid index (secure approach)
        }
        valid = true;
        return arrayData[index];
    }
};

const int Lse18::arrayData[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

int main() {
    bool valid;
    
    // Test case 1: Valid index in middle
    int result1 = Lse18::getValueAtIndex(5, valid);
    cout << "Test 1 - Index 5: " << (valid ? to_string(result1) : "Invalid") << endl;
    
    // Test case 2: Valid index at start
    int result2 = Lse18::getValueAtIndex(0, valid);
    cout << "Test 2 - Index 0: " << (valid ? to_string(result2) : "Invalid") << endl;
    
    // Test case 3: Valid index at end
    int result3 = Lse18::getValueAtIndex(9, valid);
    cout << "Test 3 - Index 9: " << (valid ? to_string(result3) : "Invalid") << endl;
    
    // Test case 4: Invalid index (negative)
    int result4 = Lse18::getValueAtIndex(-1, valid);
    cout << "Test 4 - Index -1: " << (valid ? to_string(result4) : "Invalid") << endl;
    
    // Test case 5: Invalid index (out of bounds)
    int result5 = Lse18::getValueAtIndex(15, valid);
    cout << "Test 5 - Index 15: " << (valid ? to_string(result5) : "Invalid") << endl;
    
    return 0;
}
