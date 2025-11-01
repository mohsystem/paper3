#include <iostream>
#include <vector>
using namespace std;

int getValueAtIndex(const vector<int>& id_sequence, int index) {
    if (index >= 0 && index < static_cast<int>(id_sequence.size())) {
        return id_sequence[index];
    }
    return -1; // invalid index
}

int main() {
    vector<int> id_sequence {10, 20, 30, 40, 50, 60, 70};

    cout << "Enter index: ";
    int userIndex;
    if (!(cin >> userIndex)) userIndex = -1;

    int testIndices[5] = { userIndex, 0, 3, 6, -1 };

    for (int i = 0; i < 5; ++i) {
        cout << getValueAtIndex(id_sequence, testIndices[i]) << endl;
    }

    return 0;
}