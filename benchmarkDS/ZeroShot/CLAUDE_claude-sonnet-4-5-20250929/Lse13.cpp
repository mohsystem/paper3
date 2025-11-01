
#include <iostream>
#include <vector>
#include <optional>

std::vector<int> id_sequence = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

std::optional<int> getValueAtIndex(int index) {
    if (index < 0 || index >= static_cast<int>(id_sequence.size())) {
        return std::nullopt; // Return empty optional for invalid index
    }
    return id_sequence[index];
}

int main() {
    // Test case 1: Valid index at start
    auto result1 = getValueAtIndex(0);
    std::cout << "Test 1 (index 0): " << (result1 ? std::to_string(*result1) : "null") << std::endl;
    
    // Test case 2: Valid index in middle
    auto result2 = getValueAtIndex(5);
    std::cout << "Test 2 (index 5): " << (result2 ? std::to_string(*result2) : "null") << std::endl;
    
    // Test case 3: Valid index at end
    auto result3 = getValueAtIndex(9);
    std::cout << "Test 3 (index 9): " << (result3 ? std::to_string(*result3) : "null") << std::endl;
    
    // Test case 4: Invalid negative index
    auto result4 = getValueAtIndex(-1);
    std::cout << "Test 4 (index -1): " << (result4 ? std::to_string(*result4) : "null") << std::endl;
    
    // Test case 5: Invalid out of bounds index
    auto result5 = getValueAtIndex(10);
    std::cout << "Test 5 (index 10): " << (result5 ? std::to_string(*result5) : "null") << std::endl;
    
    return 0;
}
