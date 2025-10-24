#include <iostream>
#include <vector>
#include <string>
#include <array>
#include <algorithm>

static int suitIndex(const std::string& card) {
    auto pos = card.find('_');
    if (pos == std::string::npos || pos == 0 || pos != card.rfind('_') || pos + 1 >= card.size()) {
        return -1;
    }
    if (card.size() - pos - 1 != 1) return -1;
    char s = card[pos + 1];
    switch (s) {
        case 'S': return 0;
        case 'H': return 1;
        case 'D': return 2;
        case 'C': return 3;
        default: return -1;
    }
}

bool checkFlush(const std::vector<std::string>& table, const std::vector<std::string>& hand) {
    if (table.size() != 5 || hand.size() != 2) return false;

    std::array<int, 4> tableCounts{0, 0, 0, 0};
    for (const auto& card : table) {
        int idx = suitIndex(card);
        if (idx < 0) return false;
        tableCounts[idx]++;
    }

    if (*std::max_element(tableCounts.begin(), tableCounts.end()) < 3) {
        return false;
    }

    std::array<int, 4> totalCounts = tableCounts;
    for (const auto& card : hand) {
        int idx = suitIndex(card);
        if (idx < 0) return false;
        totalCounts[idx]++;
    }

    return std::any_of(totalCounts.begin(), totalCounts.end(), [](int c){ return c >= 5; });
}

int main() {
    std::cout << std::boolalpha;

    // Test case 1: diamond flush
    std::cout << checkFlush(
        {"A_S", "J_H", "7_D", "8_D", "10_D"},
        {"J_D", "3_D"}
    ) << "\n"; // true

    // Test case 2: spade flush
    std::cout << checkFlush(
        {"10_S", "7_S", "9_H", "4_S", "3_S"},
        {"K_S", "Q_S"}
    ) << "\n"; // true

    // Test case 3: no flush
    std::cout << checkFlush(
        {"3_S", "10_H", "10_D", "10_C", "10_S"},
        {"3_S", "4_D"}
    ) << "\n"; // false

    // Test case 4: clubs flush using 3 on table + 2 in hand
    std::cout << checkFlush(
        {"2_C", "5_C", "9_C", "K_H", "A_D"},
        {"3_C", "7_C"}
    ) << "\n"; // true

    // Test case 5: four hearts on table, none in hand -> no flush
    std::cout << checkFlush(
        {"2_H", "4_H", "6_H", "8_H", "J_D"},
        {"Q_S", "K_C"}
    ) << "\n"; // false

    return 0;
}