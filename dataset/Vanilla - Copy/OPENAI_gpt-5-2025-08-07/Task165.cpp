#include <bits/stdc++.h>
using namespace std;

bool checkFlush(const vector<string>& table, const vector<string>& hand) {
    array<int, 4> cnt = {0, 0, 0, 0}; // 0:S,1:H,2:D,3:C
    auto suitIndex = [](char s) -> int {
        if (s == 'S') return 0;
        if (s == 'H') return 1;
        if (s == 'D') return 2;
        if (s == 'C') return 3;
        return -1;
    };
    auto getSuit = [](const string& card) -> char {
        size_t pos = card.rfind('_');
        if (pos != string::npos && pos + 1 < card.size()) return card[pos + 1];
        return '?';
    };
    for (const auto& c : table) {
        int idx = suitIndex(getSuit(c));
        if (idx >= 0) cnt[idx]++;
    }
    for (const auto& c : hand) {
        int idx = suitIndex(getSuit(c));
        if (idx >= 0) cnt[idx]++;
    }
    for (int v : cnt) if (v >= 5) return true;
    return false;
}

int main() {
    cout << (checkFlush({"A_S", "J_H", "7_D", "8_D", "10_D"}, {"J_D", "3_D"}) ? "true" : "false") << endl;
    cout << (checkFlush({"10_S", "7_S", "9_H", "4_S", "3_S"}, {"K_S", "Q_S"}) ? "true" : "false") << endl;
    cout << (checkFlush({"3_S", "10_H", "10_D", "10_C", "10_S"}, {"3_S", "4_D"}) ? "true" : "false") << endl;
    cout << (checkFlush({"2_H", "5_H", "7_H", "9_H", "K_H"}, {"A_S", "3_D"}) ? "true" : "false") << endl;
    cout << (checkFlush({"2_C", "5_C", "7_C", "9_H", "K_D"}, {"A_C", "3_D"}) ? "true" : "false") << endl;
    return 0;
}