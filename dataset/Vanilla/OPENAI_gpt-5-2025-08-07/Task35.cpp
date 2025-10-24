#include <bits/stdc++.h>
using namespace std;

int countPeople(const vector<pair<int,int>>& stops) {
    int total = 0;
    for (const auto& stop : stops) {
        total += stop.first - stop.second;
    }
    return total;
}

int main() {
    vector<pair<int,int>> test1 = {{10,0},{3,5},{5,8}}; // Expected: 5
    vector<pair<int,int>> test2 = {{3,0},{9,1},{4,10},{12,2},{6,1},{7,10}}; // Expected: 17
    vector<pair<int,int>> test3 = {{0,0}}; // Expected: 0
    vector<pair<int,int>> test4 = {{5,0},{5,5},{10,10},{1,1}}; // Expected: 0
    vector<pair<int,int>> test5 = {{50,0},{30,10},{5,5},{40,20},{0,0},{1,1}}; // Expected: 90

    cout << countPeople(test1) << "\n";
    cout << countPeople(test2) << "\n";
    cout << countPeople(test3) << "\n";
    cout << countPeople(test4) << "\n";
    cout << countPeople(test5) << "\n";

    return 0;
}