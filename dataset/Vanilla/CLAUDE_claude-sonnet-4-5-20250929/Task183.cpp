
#include <iostream>
#include <string>
#include <stdexcept>
#include <algorithm>

class Task183 {
public:
    static long long largestProduct(const std::string& input, int span) {
        // Handle edge cases
        if (span < 0) {
            throw std::invalid_argument("Invalid span");
        }
        
        // Remove non-digit characters
        std::string digits;
        for (char c : input) {
            if (c >= '0' && c <= '9') {
                digits += c;
            }
        }
        
        if (span > static_cast<int>(digits.length())) {
            throw std::invalid_argument("Span is larger than input length");
        }
        
        if (span == 0) {
            return 1;
        }
        
        long long maxProduct = 0;
        
        // Iterate through all possible series
        for (size_t i = 0; i <= digits.length() - span; i++) {
            long long product = 1;
            for (int j = i; j < i + span; j++) {
                product *= (digits[j] - '0');
            }
            maxProduct = std::max(maxProduct, product);
        }
        
        return maxProduct;
    }
};

int main() {
    // Test case 1
    std::cout << "Test 1: " << Task183::largestProduct("63915", 3) << std::endl; // Expected: 162
    
    // Test case 2
    std::cout << "Test 2: " << Task183::largestProduct("123456789", 4) << std::endl; // Expected: 3024
    
    // Test case 3
    std::cout << "Test 3: " << Task183::largestProduct("0123456789", 3) << std::endl; // Expected: 504
    
    // Test case 4
    std::cout << "Test 4: " << Task183::largestProduct("99999", 2) << std::endl; // Expected: 81
    
    // Test case 5
    std::cout << "Test 5: " << Task183::largestProduct("1234", 1) << std::endl; // Expected: 4
    
    return 0;
}
