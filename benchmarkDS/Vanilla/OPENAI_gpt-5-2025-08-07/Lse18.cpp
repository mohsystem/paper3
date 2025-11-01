#include <iostream>
#include <vector>
using namespace std;

// Function that takes an integer index and returns the value at that index in the array
int getValueAtIndex(int index) {
    vector<int> arr = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    return arr[index];
}

int main() {
    int testIndexes[5] = {0, 3, 5, 7, 9};
    for (int i = 0; i < 5; ++i) {
        int idx = testIndexes[i];
        int value = getValueAtIndex(idx);
        cout << "Index " << idx << " -> Value: " << value << endl;
    }
    return 0;
}