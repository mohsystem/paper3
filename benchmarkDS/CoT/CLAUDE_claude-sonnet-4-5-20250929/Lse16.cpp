
#include <iostream>
#include <vector>
#include <optional>

class Lse16 {
private:
    static const std::vector<int> DATA_ARRAY;

public:
    static std::optional<int> getValueAtIndex(int index) {
        // Input validation: check for negative index
        if (index < 0) {
            std::cout << "Error: Index cannot be negative" << std::endl;
            return std::nullopt;
        }
        
        // Bounds checking: ensure index is within array bounds
        if (index >= static_cast<int>(DATA_ARRAY.size())) {
            std::cout << "Error: Index out of bounds" << std::endl;
            return std::nullopt;
        }
        
        // Safe array access
        return DATA_ARRAY[index];
    }
};

const std::vector<int> Lse16::DATA_ARRAY = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

int main() {
    // Test case 1: Valid index at beginning
    auto result1 = Lse16::getValueAtIndex(0);
    std::cout << "Test 1 (index 0): " << (result1 ? std::to_string(*result1) : "null") << std::endl;
    
    // Test case 2: Valid index in middle
    auto result2 = Lse16::getValueAtIndex(5);
    std::cout << "Test 2 (index 5): " << (result2 ? std::to_string(*result2) : "null") << std::endl;
    
    // Test case 3: Valid index at end
    auto result3 = Lse16::getValueAtIndex(9);
    std::cout << "Test 3 (index 9): " << (result3 ? std::to_string(*result3) : "null") << std::endl;
    
    // Test case 4: Invalid index (out of bounds)
    auto result4 = Lse16::getValueAtIndex(15);
    std::cout << "Test 4 (index 15): " << (result4 ? std::to_string(*result4) : "null") << std::endl;
    
    // Test case 5: Invalid index (negative)
    auto result5 = Lse16::getValueAtIndex(-1);
    std::cout << "Test 5 (index -1): " << (result5 ? std::to_string(*result5) : "null") << std::endl;
    
    return 0;
}
