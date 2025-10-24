#include <bits/stdc++.h>
using namespace std;

class Task155 {
public:
    static vector<int> heap_sort(const vector<int>& arr) {
        vector<int> a = arr;
        const size_t n = a.size();

        auto sift_down = [&](size_t heap_size, size_t i) {
            while (true) {
                size_t largest = i;
                size_t left = i * 2 + 1;
                size_t right = left + 1;

                if (left < heap_size && a[left] > a[largest]) largest = left;
                if (right < heap_size && a[right] > a[largest]) largest = right;
                if (largest == i) break;
                std::swap(a[i], a[largest]);
                i = largest;
            }
        };

        if (n > 1) {
            for (size_t i = (n / 2); i-- > 0; ) {
                sift_down(n, i);
            }
            for (size_t end = n - 1; end > 0; --end) {
                std::swap(a[0], a[end]);
                sift_down(end, 0);
            }
        }
        return a;
    }
};

static void print_vec(const vector<int>& v) {
    cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) cout << ", ";
        cout << v[i];
    }
    cout << "]\n";
}

int main() {
    vector<vector<int>> tests = {
        {},
        {1},
        {5, 3, 8, 4, 2, 7, 1, 10},
        {1, 2, 3, 4, 5},
        {-3, -1, -7, 0, 2, 2, -3}
    };

    for (const auto& t : tests) {
        auto sorted = Task155::heap_sort(t);
        print_vec(sorted);
    }
    return 0;
}