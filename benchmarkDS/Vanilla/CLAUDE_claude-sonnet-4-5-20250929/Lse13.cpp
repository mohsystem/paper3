
#include <iostream>
using namespace std;

int id_sequence[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
int id_sequence_size = 10;

int getValueAtIndex(int index) {
    if (index >= 0 && index < id_sequence_size) {
        return id_sequence[index];
    }
    return -1; // Return -1 for invalid index
}

int main() {
    // Test case 1
    int index1 = 0;
    cout << "Index " << index1 << ": " << getValueAtIndex(index1) << endl;
    
    // Test case 2
    int index2 = 5;
    cout << "Index " << index2 << ": " << getValueAtIndex(index2) << endl;
    
    // Test case 3
    int index3 = 9;
    cout << "Index " << index3 << ": " << getValueAtIndex(index3) << endl;
    
    // Test case 4
    int index4 = -1;
    cout << "Index " << index4 << ": " << getValueAtIndex(index4) << endl;
    
    // Test case 5
    int index5 = 10;
    cout << "Index " << index5 << ": " << getValueAtIndex(index5) << endl;
    
    return 0;
}
