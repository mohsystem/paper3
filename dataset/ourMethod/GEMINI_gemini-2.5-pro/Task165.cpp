#include <iostream>
#include <vector>
#include <string>
#include <map>

bool isValidCard(const std::string& card) {
    if (card.length() < 3 || card.find('_') == std::string::npos) {
        return false;
    }
    char suit = card.back();
    return suit == 'S' || suit == 'H' || suit == 'D' || suit == 'C';
}

bool checkFlush(const std::vector<std::string>& tableCards, const std::vector<std::string>& handCards) {
    if (tableCards.size() != 5 || handCards.size() != 2) {
        return false;
    }

    std::map<char, int> suitCounts;

    for (const auto& card : tableCards) {
        if (!isValidCard(card)) {
            return false;
        }
        suitCounts[card.back()]++;
    }

    for (const auto& card : handCards) {
        if (!isValidCard(card)) {
            return false;
        }
        suitCounts[card.back()]++;
    }

    for (const auto& pair : suitCounts) {
        if (pair.second >= 5) {
            return true;
        }
    }

    return false;
}

int main() {
    // Test case 1
    std::vector<std::string> table1 = {"A_S", "J_H", "7_D", "8_D", "10_D"};
    std::vector<std::string> hand1 = {"J_D", "3_D"};
    std::cout << "Test 1: " << std::boolalpha << checkFlush(table1, hand1) << std::endl;

    // Test case 2
    std::vector<std::string> table2 = {"10_S", "7_S", "9_H", "4_S", "3_S"};
    std::vector<std::string> hand2 = {"K_S", "Q_S"};
    std::cout << "Test 2: " << std::boolalpha << checkFlush(table2, hand2) << std::endl;

    // Test case 3
    std::vector<std::string> table3 = {"3_S", "10_H", "10_D", "10_C", "10_S"};
    std::vector<std::string> hand3 = {"3_S", "4_D"};
    std::cout << "Test 3: " << std::boolalpha << checkFlush(table3, hand3) << std::endl;

    // Test case 4
    std::vector<std::string> table4 = {"A_C", "K_C", "Q_C", "J_C", "10_C"};
    std::vector<std::string> hand4 = {"2_H", "3_D"};
    std::cout << "Test 4: " << std::boolalpha << checkFlush(table4, hand4) << std::endl;

    // Test case 5
    std::vector<std::string> table5 = {"A_H", "A_S", "A_D", "A_C", "K_H"};
    std::vector<std::string> hand5 = {"K_S", "K_D"};
    std::cout << "Test 5: " << std::boolalpha << checkFlush(table5, hand5) << std::endl;

    return 0;
}