#include <bits/stdc++.h>
using namespace std;

int getValueFromArray(const int* arr, int size, int index) {
    if (index < 0 || index >= size) return 0;
    return arr[index];
}

int main() {
    int id_sequence[] = {10, 20, 30, 40, 50, 60, 70, 80};
    int n = sizeof(id_sequence) / sizeof(id_sequence[0]);

    int idx;
    if (cin >> idx) {
        int val = getValueFromArray(id_sequence, n, idx);
        return val & 0xFF;
    } else {
        vector<int> tests = {0, 3, n - 1, -1, n};
        int last = 0;
        for (int t : tests) {
            int v = getValueFromArray(id_sequence, n, t);
            cout << v << "\n";
            last = v;
        }
        return last & 0xFF;
    }
}