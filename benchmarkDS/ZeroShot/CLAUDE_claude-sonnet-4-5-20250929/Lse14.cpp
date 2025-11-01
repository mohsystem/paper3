
#include <iostream>
#include <vector>
using namespace std;

class Lse14 {
private:
    static const int id_sequence[];
    static const int size = 10;
    
public:
    static int getValueAtIndex(int index, bool& valid) {
        if (index < 0 || index >= size) {
            valid = false;
            return -1; // Return -1 for invalid index
        }
        valid = true;
        return id_sequence[index];
    }
};

const int Lse14::id_sequence[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

int main() {
    bool valid;
    
    // Test case 1: Valid index at beginning
    int result1 = Lse14::getValueAtIndex(0, valid);
    cout << "Test 1 (index 0): " << (valid ? to_string(result1) : "Invalid") << endl;
    
    // Test case 2: Valid index in middle
    int result2 = Lse14::getValueAtIndex(4, valid);
    cout << "Test 2 (index 4): " << (valid ? to_string(result2) : "Invalid") << endl;
    
    // Test case 3: Valid index at end
    int result3 = Lse14::getValueAtIndex(9, valid);
    cout << "Test 3 (index 9): " << (valid ? to_string(result3) : "Invalid") << endl;
    
    // Test case 4: Invalid negative index
    int result4 = Lse14::getValueAtIndex(-1, valid);
    cout << "Test 4 (index -1): " << (valid ? to_string(result4) : "Invalid") << endl;
    
    // Test case 5: Invalid index beyond array length
    int result5 = Lse14::getValueAtIndex(15, valid);
    cout << "Test 5 (index 15): " << (valid ? to_string(result5) : "Invalid") << endl;
    
    return 0;
}
