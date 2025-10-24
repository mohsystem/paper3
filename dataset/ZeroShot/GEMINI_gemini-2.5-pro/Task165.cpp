#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

/**
 * @brief Determines if a flush exists given 5 table cards and 2 hand cards.
 * A flush is 5 or more cards of the same suit.
 * 
 * @param table A vector of 5 strings representing the cards on the table.
 * @param hand A vector of 2 strings representing the cards in hand.
 * @return true if a flush exists, false otherwise.
 */
bool checkFlush(const std::vector<std::string>& table, const std::vector<std::string>& hand) {
    // Securely validate input size
    if (table.size() != 5 || hand.size() != 2) {
        return false;
    }
    
    std::unordered_map<char, int> suitCounts;
    
    // Process table cards
    for (const std::string& card : table) {
        // Check for non-empty string to prevent UB with .back()
        if (!card.empty()) {
            suitCounts[card.back()]++;
        }
    }
    
    // Process hand cards
    for (const std::string& card : hand) {
        if (!card.empty()) {
            suitCounts[card.back()]++;
        }
    }
    
    // Check if any suit count is 5 or more
    for (const auto& pair : suitCounts) {
        if (pair.second >= 5) {
            return true;
        }
    }
    
    return false;
}

int main() {
    // Test Case 1: Diamond flush
    std::vector<std::string> table1 = {"A_S", "J_H", "7_D", "8_D", "10_D"};
    std::vector<std::string> hand1 = {"J_D", "3_D"};
    std::cout << "Test Case 1: " << (checkFlush(table1, hand1) ? "true" : "false") << std::endl;

    // Test Case 2: Spade flush (6 spades)
    std::vector<std::string> table2 = {"10_S", "7_S", "9_H", "4_S", "3_S"};
    std::vector<std::string> hand2 = {"K_S", "Q_S"};
    std::cout << "Test Case 2: " << (checkFlush(table2, hand2) ? "true" : "false") << std::endl;

    // Test Case 3: No flush
    std::vector<std::string> table3 = {"3_S", "10_H", "10_D", "10_C", "10_S"};
    std::vector<std::string> hand3 = {"3_S", "4_D"};
    std::cout << "Test Case 3: " << (checkFlush(table3, hand3) ? "true" : "false") << std::endl;

    // Test Case 4: No flush (4 Hearts, 3 Diamonds)
    std::vector<std::string> table4 = {"A_H", "2_H", "3_H", "4_H", "5_D"};
    std::vector<std::string> hand4 = {"6_D", "7_D"};
    std::cout << "Test Case 4: " << (checkFlush(table4, hand4) ? "true" : "false") << std::endl;
    
    // Test Case 5: No flush (3 Clubs, 4 Spades)
    std::vector<std::string> table5 = {"A_C", "2_C", "3_C", "4_S", "5_S"};
    std::vector<std::string> hand5 = {"6_S", "7_S"};
    std::cout << "Test Case 5: " << (checkFlush(table5, hand5) ? "true" : "false") << std::endl;

    return 0;
}