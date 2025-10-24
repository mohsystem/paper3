#include <iostream>
#include <vector>
#include <string>
#include <map>

/**
 * @brief Determines if a flush is possible given 5 table cards and 2 hand cards.
 * A flush is 5 or more cards of the same suit.
 * 
 * @param table A vector of 5 strings representing cards on the table.
 * @param hand A vector of 2 strings representing cards in hand.
 * @return true if a flush exists, false otherwise.
 */
bool checkFlush(const std::vector<std::string>& table, const std::vector<std::string>& hand) {
    std::map<char, int> suitCounts;
    
    // Combine cards into a single vector
    std::vector<std::string> allCards;
    allCards.insert(allCards.end(), table.begin(), table.end());
    allCards.insert(allCards.end(), hand.begin(), hand.end());

    // Count suits
    for (const std::string& card : allCards) {
        if (!card.empty()) {
            suitCounts[card.back()]++;
        }
    }

    // Check for a flush
    for (auto const& pair : suitCounts) {
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
    std::cout << "Test Case 1: " << std::boolalpha << checkFlush(table1, hand1) << std::endl;

    // Test Case 2
    std::vector<std::string> table2 = {"10_S", "7_S", "9_H", "4_S", "3_S"};
    std::vector<std::string> hand2 = {"K_S", "Q_S"};
    std::cout << "Test Case 2: " << std::boolalpha << checkFlush(table2, hand2) << std::endl;

    // Test Case 3
    std::vector<std::string> table3 = {"3_S", "10_H", "10_D", "10_C", "10_S"};
    std::vector<std::string> hand3 = {"3_S", "4_D"};
    std::cout << "Test Case 3: " << std::boolalpha << checkFlush(table3, hand3) << std::endl;

    // Test Case 4
    std::vector<std::string> table4 = {"A_H", "A_S", "A_D", "A_C", "2_H"};
    std::vector<std::string> hand4 = {"3_H", "4_H"};
    std::cout << "Test Case 4: " << std::boolalpha << checkFlush(table4, hand4) << std::endl;

    // Test Case 5
    std::vector<std::string> table5 = {"K_C", "Q_C", "J_C", "3_S", "4_H"};
    std::vector<std::string> hand5 = {"10_C", "9_C"};
    std::cout << "Test Case 5: " << std::boolalpha << checkFlush(table5, hand5) << std::endl;

    return 0;
}