#include <iostream>
#include <vector>
using namespace std;

void heapify(vector<int>& a, int heapSize, int i) {
    while (true) {
        int largest = i;
        int l = 2 * i + 1;
        int r = 2 * i + 2;
        if (l < heapSize && a[l] > a[largest]) largest = l;
        if (r < heapSize && a[r] > a[largest]) largest = r;
        if (largest != i) {
            swap(a[i], a[largest]);
            i = largest;
        } else {
            break;
        }
    }
}

vector<int> heapSort(vector<int> a) {
    int n = (int)a.size();
    for (int i = n / 2 - 1; i >= 0; --i) heapify(a, n, i);
    for (int i = n - 1; i > 0; --i) {
        swap(a[0], a[i]);
        heapify(a, i, 0);
    }
    return a;
}

void printVec(const vector<int>& v) {
    cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) cout << ", ";
        cout << v[i];
    }
    cout << "]\n";
}

int main() {
    vector<vector<int>> tests = {
        {4, 10, 3, 5, 1},
        {},
        {1},
        {9, -2, 0, 7, 3, 3, -5},
        {5, 4, 3, 2, 1}
    };
    for (auto t : tests) {
        auto s = heapSort(t);
        printVec(s);
    }
    return 0;
}