#include <bits/stdc++.h>
using namespace std;

int number(const vector<pair<int,int>>& stops) {
    int passengers = 0;
    for (const auto& p : stops) {
        passengers += p.first - p.second;
    }
    return passengers;
}

int main() {
    vector<pair<int,int>> t1 = {{10,0},{3,5},{5,8}};
    vector<pair<int,int>> t2 = {{3,0},{9,1},{4,10},{12,2},{6,1},{7,10}};
    vector<pair<int,int>> t3 = {{0,0}};
    vector<pair<int,int>> t4 = {{5,0},{0,0},{0,0}};
    vector<pair<int,int>> t5 = {{21,0},{0,20},{1,0}};
    cout << number(t1) << "\n";
    cout << number(t2) << "\n";
    cout << number(t3) << "\n";
    cout << number(t4) << "\n";
    cout << number(t5) << "\n";
    return 0;
}