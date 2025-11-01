#include <iostream>
#include <vector>
using namespace std;

static const int id_sequence_arr[] = {101, 203, 305, 407, 509, 611, 713, 815, 917, 1020};
static const int ID_SIZE = sizeof(id_sequence_arr) / sizeof(id_sequence_arr[0]);

int getValueAtIndex(int index) {
    if (index >= 0 && index < ID_SIZE) {
        return id_sequence_arr[index];
    }
    return -1; // Return -1 for invalid index
}

int main() {
    // 5 test cases
    cout << "Running 5 test cases:" << endl;
    int tests[5] = {0, 1, 4, 9, 10};
    for (int i = 0; i < 5; ++i) {
        int idx = tests[i];
        cout << "Index " << idx << " -> " << getValueAtIndex(idx) << endl;
    }

    // Prompt user for input
    cout << "Enter index: ";
    int idx;
    if (cin >> idx) {
        cout << getValueAtIndex(idx) << endl;
    } else {
        cout << "Invalid input" << endl;
    }
    return 0;
}