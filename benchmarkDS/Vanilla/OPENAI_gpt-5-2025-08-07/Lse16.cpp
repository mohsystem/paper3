#include <bits/stdc++.h>
using namespace std;

int getValueAtIndex(int index) {
    vector<int> arr = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    return arr.at(index);
}

int main() {
    vector<int> tests = {0, 1, 5, 8, 9};
    for (int idx : tests) {
        try {
            int result = getValueAtIndex(idx);
            cout << "Index " << idx << " -> " << result << '\n';
        } catch (const out_of_range&) {
            cout << "Index " << idx << " -> Index out of bounds\n";
        }
    }
    return 0;
}