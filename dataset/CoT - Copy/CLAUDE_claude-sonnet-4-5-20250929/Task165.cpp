
#include <iostream>
#include <string>
#include <vector>
#include <map>

bool checkFlush(const std::vector<std::string>& table, const std::vector<std::string>& hand) {
    // Input validation
    if (table.size() != 5 || hand.size() != 2) {
        return false;
    }
    
    // Count suits from all cards
    std::map<char, int> suitCount;
    suitCount['S'] = 0;
    suitCount['H'] = 0;
    suitCount['D'] = 0;
    suitCount['C'] = 0;
    
    // Process table cards
    for (const std::string& card : table) {
        if (card.empty()) {
            continue;
        }
        size_t pos = card.find_last_of('_');
        if (pos != std::string::npos && pos + 1 < card.length()) {
            char suit = card[pos + 1];
            if (suitCount.find(suit) != suitCount.end()) {
                suitCount[suit]++;
            }
        }
    }
    
    // Process hand cards
    for (const std::string& card : hand) {
        if (card.empty()) {
            continue;
        }
        size_t pos = card.find_last_of('_');
        if (pos != std::string::npos && pos + 1 < card.length()) {
            char suit = card[pos + 1];
            if (suitCount.find(suit) != suitCount.end()) {
                suitCount[suit]++;
            }
        }
    }
    
    // Check if any suit has at least 5 cards (flush)
    for (const auto& pair : suitCount) {
        if (pair.second >= 5) {
            return true;
        }
    }
    
    return false;
}

int main() {
    // Test case 1: Diamond flush
    std::cout << std::boolalpha << checkFlush(
        {"A_S", "J_H", "7_D", "8_D", "10_D"}, 
        {"J_D", "3_D"}
    ) << std::endl; // true
    
    // Test case 2: Spade flush
    std::cout << checkFlush(
        {"10_S", "7_S", "9_H", "4_S", "3_S"}, 
        {"K_S", "Q_S"}
    ) << std::endl; // true
    
    // Test case 3: No flush
    std::cout << checkFlush(
        {"3_S", "10_H", "10_D", "10_C", "10_S"}, 
        {"3_S", "4_D"}
    ) << std::endl; // false
    
    // Test case 4: Heart flush
    std::cout << checkFlush(
        {"2_H", "5_H", "8_H", "K_H", "A_S"}, 
        {"Q_H", "J_H"}
    ) << std::endl; // true
    
    // Test case 5: Club flush
    std::cout << checkFlush(
        {"9_C", "7_C", "4_C", "2_C", "K_H"}, 
        {"6_C", "3_D"}
    ) << std::endl; // true
    
    return 0;
}
