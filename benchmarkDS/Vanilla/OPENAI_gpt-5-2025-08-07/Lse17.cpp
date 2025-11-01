#include <iostream>
#include <vector>
#include <stdexcept>
using namespace std;

class Lse17 {
public:
    static int getValueAtIndex(const vector<int>& arr, int index) {
        if (index < 0 || index >= static_cast<int>(arr.size())) {
            throw out_of_range("Invalid index");
        }
        return arr[index];
    }
};

int main() {
    vector<int> arr{10, 25, 37, 48, 59};
    vector<int> testIndices{0, 1, 2, 3, 4};

    for (int idx : testIndices) {
        cout << Lse17::getValueAtIndex(arr, idx) << '\n';
    }
    return 0;
}