#include <iostream>
#include <vector>
#include <string>
#include <cctype>
using namespace std;

int suitIndex(const string& card) {
    size_t pos = card.find_last_of('_');
    if (pos == string::npos || pos + 1 >= card.size()) return -1;
    char ch = static_cast<char>(toupper(static_cast<unsigned char>(card[pos + 1])));
    switch (ch) {
        case 'S': return 0;
        case 'H': return 1;
        case 'D': return 2;
        case 'C': return 3;
        default: return -1;
    }
}

bool checkFlush(const vector<string>& table, const vector<string>& hand) {
    int counts[4] = {0,0,0,0};
    int tableCounts[4] = {0,0,0,0};

    for (const auto& card : table) {
        int idx = suitIndex(card);
        if (idx >= 0) {
            counts[idx]++;
            tableCounts[idx]++;
        }
    }

    int maxTable = 0;
    for (int i = 0; i < 4; ++i) maxTable = max(maxTable, tableCounts[i]);
    if (maxTable < 3) return false;

    for (const auto& card : hand) {
        int idx = suitIndex(card);
        if (idx >= 0) counts[idx]++;
    }

    for (int i = 0; i < 4; ++i) {
        if (counts[i] >= 5) return true;
    }
    return false;
}

int main() {
    vector<vector<string>> tables = {
        {"A_S", "J_H", "7_D", "8_D", "10_D"},
        {"10_S", "7_S", "9_H", "4_S", "3_S"},
        {"3_S", "10_H", "10_D", "10_C", "10_S"},
        {"2_H", "5_C", "9_D", "K_H", "Q_H"},
        {"A_C", "2_D", "3_H", "4_S", "5_C"}
    };
    vector<vector<string>> hands = {
        {"J_D", "3_D"},
        {"K_S", "Q_S"},
        {"3_S", "4_D"},
        {"A_H", "3_H"},
        {"6_D", "7_H"}
    };

    for (size_t i = 0; i < tables.size(); ++i) {
        cout << "Test " << (i + 1) << ": " << (checkFlush(tables[i], hands[i]) ? "true" : "false") << "\n";
    }
    return 0;
}