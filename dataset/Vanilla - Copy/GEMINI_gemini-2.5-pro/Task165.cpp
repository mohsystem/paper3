#include <iostream>
#include <vector>
#include <string>
#include <map>

/**
 * @brief Determines if a 5-card flush can be made from 5 table cards and 2 hand cards.
 * 
 * @param tableCards A vector of 5 strings representing cards on the table.
 * @param handCards A vector of 2 strings representing cards in hand.
 * @return true if a flush exists, false otherwise.
 */
bool checkFlush(const std::vector<std::string>& tableCards, const std::vector<std::string>& handCards) {
    std::map<char, int> suitCounts;

    // Process table cards
    for (const std::string& card : tableCards) {
        suitCounts[card.back()]++;
    }

    // Process hand cards
    for (const std::string& card : handCards) {
        suitCounts[card.back()]++;
    }

    // Check for a flush
    for (const auto& pair : suitCounts) {
        if (pair.second >= 5) {
            return true;
        }
    }

    return false;
}

int main() {
    // Test Case 1
    std::vector<std::string> table1 = {"A_S", "J_H", "7_D", "8_D", "10_D"};
    std::vector<std::string> hand1 = {"J_D", "3_D"};
    std::cout << std::boolalpha << checkFlush(table1, hand1) << std::endl;

    // Test Case 2
    std::vector<std::string> table2 = {"10_S", "7_S", "9_H", "4_S", "3_S"};
    std::vector<std::string> hand2 = {"K_S", "Q_S"};
    std::cout << std::boolalpha << checkFlush(table2, hand2) << std::endl;

    // Test Case 3
    std::vector<std::string> table3 = {"3_S", "10_H", "10_D", "10_C", "10_S"};
    std::vector<std::string> hand3 = {"3_S", "4_D"};
    std::cout << std::boolalpha << checkFlush(table3, hand3) << std::endl;

    // Test Case 4
    std::vector<std::string> table4 = {"A_H", "K_H", "Q_H", "J_H", "10_H"};
    std::vector<std::string> hand4 = {"3_S", "4_D"};
    std::cout << std::boolalpha << checkFlush(table4, hand4) << std::endl;
    
    // Test Case 5
    std::vector<std::string> table5 = {"A_S", "K_S", "Q_S", "J_S", "2_H"};
    std::vector<std::string> hand5 = {"3_D", "4_C"};
    std::cout << std::boolalpha << checkFlush(table5, hand5) << std::endl;

    return 0;
}