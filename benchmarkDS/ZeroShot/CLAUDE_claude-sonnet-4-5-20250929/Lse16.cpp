
#include <iostream>
#include <vector>
#include <optional>

class Lse16 {
private:
    static const std::vector<int> ARRAY;
    
public:
    static std::optional<int> getValueAtIndex(int index) {
        if (index < 0 || index >= static_cast<int>(ARRAY.size())) {
            return std::nullopt; // Return nullopt for out of bounds access
        }
        return ARRAY[index];
    }
};

const std::vector<int> Lse16::ARRAY = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

int main() {
    // Test case 1: Valid index at the beginning
    auto result1 = Lse16::getValueAtIndex(0);
    std::cout << "Test 1 (index 0): " << (result1.has_value() ? std::to_string(result1.value()) : "null") << std::endl;
    
    // Test case 2: Valid index in the middle
    auto result2 = Lse16::getValueAtIndex(5);
    std::cout << "Test 2 (index 5): " << (result2.has_value() ? std::to_string(result2.value()) : "null") << std::endl;
    
    // Test case 3: Valid index at the end
    auto result3 = Lse16::getValueAtIndex(9);
    std::cout << "Test 3 (index 9): " << (result3.has_value() ? std::to_string(result3.value()) : "null") << std::endl;
    
    // Test case 4: Invalid negative index
    auto result4 = Lse16::getValueAtIndex(-1);
    std::cout << "Test 4 (index -1): " << (result4.has_value() ? std::to_string(result4.value()) : "null") << std::endl;
    
    // Test case 5: Invalid index beyond array length
    auto result5 = Lse16::getValueAtIndex(15);
    std::cout << "Test 5 (index 15): " << (result5.has_value() ? std::to_string(result5.value()) : "null") << std::endl;
    
    return 0;
}
