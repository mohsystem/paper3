#include <iostream>
#include <vector>
using namespace std;

int getMonthlySales(int i, const vector<int>& sales) {
    return sales[i];
}

int totalFirstQuarterSales(const vector<int>& sales) {
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        sum += getMonthlySales(i, sales);
    }
    return sum;
}

int main() {
    vector<vector<int>> tests = {
        {100, 200, 300},
        {0, 0, 0, 0},
        {5, 10, 15, 20},
        {1000, -50, 25, 0},
        {7, 8, 9, 10, 11}
    };
    for (size_t t = 0; t < tests.size(); ++t) {
        int result = totalFirstQuarterSales(tests[t]);
        cout << "Test " << (t + 1) << ": " << result << '\n';
    }
    return 0;
}