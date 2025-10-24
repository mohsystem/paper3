
#include <iostream>
#include <string>
#include <stdexcept>
#include <algorithm>

class Task183 {
public:
    static long long largestProduct(const std::string& input, int span) {
        // Validate inputs
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
            throw std::invalid_argument("Span cannot be larger than input length");
        }
        
        // Validate that input contains only digits
        for (char c : input) {
            if (!isdigit(c)) {
                throw std::invalid_argument("Input must contain only digits");
            }
        }
        
        long long maxProduct = 0;
        
        // Iterate through all possible series
        for (size_t i = 0; i <= input.length() - span; i++) {
            long long product = 1;
            
            // Calculate product of current series
            for (int j = 0; j < span; j++) {
                product *= (input[i + j] - '0');
            }
            
            // Update max product if current is larger
            maxProduct = std::max(maxProduct, product);
        }
        
        return maxProduct;
    }
};

int main() {
    // Test case 1: Example from prompt
    std::cout << "Test 1: " << Task183::largestProduct("63915", 3) << std::endl; // Expected: 162
    
    // Test case 2: Span of 1
    std::cout << "Test 2: " << Task183::largestProduct("63915", 1) << std::endl; // Expected: 9
    
    // Test case 3: Span equals input length
    std::cout << "Test 3: " << Task183::largestProduct("123", 3) << std::endl; // Expected: 6
    
    // Test case 4: With zeros
    std::cout << "Test 4: " << Task183::largestProduct("1203", 2) << std::endl; // Expected: 6
    
    // Test case 5: Larger number
    std::cout << "Test 5: " << Task183::largestProduct("73167176531330624919225119674426574742355349194934", 6) << std::endl; // Expected: 23520
    
    return 0;
}
