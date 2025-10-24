#include <iostream>
#include <vector>
#include <string>
using namespace std;

vector<string> buildTower(int n) {
    vector<string> res;
    if (n <= 0) return res;
    int width = 2 * n - 1;
    for (int i = 1; i <= n; ++i) {
        int stars = 2 * i - 1;
        int spaces = n - i;
        string row(spaces, ' ');
        row += string(stars, '*');
        row += string(spaces, ' ');
        res.push_back(row);
    }
    return res;
}

int main() {
    vector<int> tests = {1, 2, 3, 4, 6};
    for (int t : tests) {
        cout << "Tower with " << t << " floors:" << endl;
        vector<string> tower = buildTower(t);
        for (const auto& row : tower) {
            cout << row << endl;
        }
        cout << endl;
    }
    return 0;
}