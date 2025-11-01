#include <bits/stdc++.h>
using namespace std;

int getValueAtIndex(const vector<int>& id_sequence, int index) {
    if (index < 0 || index >= static_cast<int>(id_sequence.size())) {
        throw out_of_range("index out of range");
    }
    return id_sequence[index];
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    vector<int> id_sequence{11, 22, 33, 44, 55, 66, 77, 88, 99};

    cout << "Enter index: ";
    int idx;
    if (cin >> idx) {
        try {
            int val = getValueAtIndex(id_sequence, idx);
            cout << "Value at index " << idx << ": " << val << "\n";
        } catch (const out_of_range&) {
            cout << "Invalid index: " << idx << "\n";
        }
    } else {
        cout << "Invalid input.\n";
        cin.clear();
        string dummy;
        getline(cin, dummy);
    }

    vector<int> testIndices{0, 3, (int)id_sequence.size() - 1, -1, 100};
    for (size_t i = 0; i < testIndices.size(); ++i) {
        int ti = testIndices[i];
        try {
            int val = getValueAtIndex(id_sequence, ti);
            cout << "Test " << (i + 1) << " -> index " << ti << " value: " << val << "\n";
        } catch (const out_of_range&) {
            cout << "Test " << (i + 1) << " -> index " << ti << " invalid\n";
        }
    }
    return 0;
}