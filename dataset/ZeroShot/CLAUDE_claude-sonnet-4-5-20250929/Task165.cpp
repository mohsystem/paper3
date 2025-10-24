
#include <iostream>
#include <string>
#include <vector>
#include <map>

bool checkFlush(std::vector<std::string> table, std::vector<std::string> hand) {
    // Count suits on table
    std::map<char, int> suitCount;
    suitCount['S'] = 0;
    suitCount['H'] = 0;
    suitCount['D'] = 0;
    suitCount['C'] = 0;
    
    for (const std::string& card : table) {
        char suit = card[card.length() - 1];
        suitCount[suit]++;
    }
    
    // Check each suit that has at least 3 cards on table
    for (auto& pair : suitCount) {
        if (pair.second >= 3) {
            char targetSuit = pair.first;
            int totalCount = pair.second;
            
            // Count matching suits in hand
            for (const std::string& card : hand) {
                char suit = card[card.length() - 1];
                if (suit == targetSuit) {
                    totalCount++;
                }
            }
            
            // Check if we have 5 or more cards of same suit
            if (totalCount >= 5) {
                return true;
            }
        }
    }
    
    return false;
}

int main() {
    // Test case 1
    std::cout << std::boolalpha << checkFlush(
        {"A_S", "J_H", "7_D", "8_D", "10_D"}, 
        {"J_D", "3_D"}
    ) << std::endl; // true - diamond flush
    
    // Test case 2
    std::cout << checkFlush(
        {"10_S", "7_S", "9_H", "4_S", "3_S"}, 
        {"K_S", "Q_S"}
    ) << std::endl; // true - spade flush
    
    // Test case 3
    std::cout << checkFlush(
        {"3_S", "10_H", "10_D", "10_C", "10_S"}, 
        {"3_S", "4_D"}
    ) << std::endl; // false
    
    // Test case 4
    std::cout << checkFlush(
        {"2_H", "5_H", "9_H", "K_H", "A_S"}, 
        {"3_H", "7_C"}
    ) << std::endl; // true - heart flush
    
    // Test case 5
    std::cout << checkFlush(
        {"2_C", "3_C", "4_C", "5_S", "6_H"}, 
        {"7_D", "8_S"}
    ) << std::endl; // false
    
    return 0;
}
