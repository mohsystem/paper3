#include <iostream>
#include <vector>
#include <string>
#include <cctype>

using namespace std;

// Step 1: Problem understanding and function definition
bool checkFlush(const vector<string>& table, const vector<string>& hand) {
    int counts[4] = {0, 0, 0, 0}; // 0:S, 1:H, 2:D, 3:C

    auto suitIndex = [](char s) -> int {
        s = static_cast<char>(toupper(static_cast<unsigned char>(s)));
        switch (s) {
            case 'S': return 0;
            case 'H': return 1;
            case 'D': return 2;
            case 'C': return 3;
            default:  return -1;
        }
    };

    // Step 2-4: Secure parsing and counting with input validation
    auto add = [&](const vector<string>& cards) {
        for (const auto& card : cards) {
            size_t pos = card.find('_');
            if (pos == string::npos || pos + 1 >= card.size()) continue;
            int si = suitIndex(card[pos + 1]);
            if (si >= 0) counts[si]++;
        }
    };

    add(table);
    add(hand);

    for (int c : counts) {
        if (c >= 5) return true;
    }
    return false;
}

// Step 5: Main with 5 test cases
int main() {
    vector<vector<string>> tables = {
        {"A_S", "J_H", "7_D", "8_D", "10_D"},
        {"10_S", "7_S", "9_H", "4_S", "3_S"},
        {"3_S", "10_H", "10_D", "10_C", "10_S"},
        {"2_H", "5_H", "K_H", "9_C", "J_D"},
        {"a_s", "j_h", "7_d", "8_d", "10_d"}
    };
    vector<vector<string>> hands = {
        {"J_D", "3_D"},
        {"K_S", "Q_S"},
        {"3_S", "4_D"},
        {"A_H", "3_H"},
        {"j_d", "3_c"}
    };

    for (size_t i = 0; i < tables.size(); ++i) {
        bool res = checkFlush(tables[i], hands[i]);
        cout << "Test " << (i + 1) << ": " << (res ? "true" : "false") << "\n";
    }
    return 0;
}