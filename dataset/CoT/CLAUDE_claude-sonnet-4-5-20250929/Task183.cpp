
#include <iostream>
#include <string>
#include <stdexcept>
#include <cctype>

class Task183 {
public:
    static long long largestProduct(const std::string& input, int span) {
        // Input validation
        if (input.empty()) {
            throw std::invalid_argument("Input cannot be empty");
        }
        
        if (span < 0) {
            throw std::invalid_argument("Span cannot be negative");
        }
        
        if (span == 0) {
            return 1;
        }
        
        if (span > static_cast<int>(input.length())) {
            throw std::invalid_argument("Span cannot be greater than input length");
        }
        
        // Validate that input contains only digits
        for (char c : input) {
            if (!std::isdigit(static_cast<unsigned char>(c))) {
                throw std::invalid_argument("Input must contain only digits");
            }
        }
        
        long long maxProduct = 0;
        
        // Iterate through all possible series
        for (size_t i = 0; i <= input.length() - span; i++) {
            long long product = 1;
            
            // Calculate product for current series
            for (int j = i; j < i + span; j++) {
                int digit = input[j] - '0';
                product *= digit;
            }
            
            // Update maximum product
            if (product > maxProduct) {
                maxProduct = product;
            }
        }
        
        return maxProduct;
    }
};

int main() {
    // Test case 1: Example from problem
    std::cout << "Test 1: " << Task183::largestProduct("63915", 3) << std::endl; // Expected: 162
    
    // Test case 2: Single digit span
    std::cout << "Test 2: " << Task183::largestProduct("987654321", 1) << std::endl; // Expected: 9
    
    // Test case 3: Full length span
    std::cout << "Test 3: " << Task183::largestProduct("123", 3) << std::endl; // Expected: 6
    
    // Test case 4: Contains zeros
    std::cout << "Test 4: " << Task183::largestProduct("1020304", 3) << std::endl; // Expected: 24
    
    // Test case 5: Larger span
    std::cout << "Test 5: " << Task183::largestProduct("73167176531330624919225119674426574742355349194934", 6) << std::endl; // Expected: 23520
    
    return 0;
}
