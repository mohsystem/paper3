
#include <iostream>
#include <stdexcept>
#include <cmath>

class Task11 {
public:
    static int nb_year(int p0, double percent, int aug, int p) {
        // Input validation to prevent security issues
        if (p0 <= 0 || p <= 0 || percent < 0) {
            throw std::invalid_argument("Invalid input parameters");
        }
        
        int years = 0;
        long long currentPopulation = p0;
        
        // Convert percent to decimal (e.g., 2 -> 0.02)
        double percentDecimal = percent / 100.0;
        
        // Calculate years needed with overflow protection
        const int MAX_ITERATIONS = 10000;
        while (currentPopulation < p) {
            // Prevent infinite loop
            if (years >= MAX_ITERATIONS) {
                throw std::runtime_error("Calculation exceeded maximum iterations");
            }
            
            // Calculate new population with integer conversion (floor)
            long long newPopulation = currentPopulation + 
                                     static_cast<long long>(currentPopulation * percentDecimal) + 
                                     aug;
            
            // Check for overflow
            if (newPopulation < 0 || newPopulation > 1e15) {
                throw std::overflow_error("Population overflow occurred");
            }
            
            currentPopulation = newPopulation;
            years++;
        }
        
        return years;
    }
};

int main() {
    try {
        // Test case 1
        std::cout << "Test 1: " << Task11::nb_year(1000, 2, 50, 1200) << " (Expected: 3)" << std::endl;
        
        // Test case 2
        std::cout << "Test 2: " << Task11::nb_year(1500, 5, 100, 5000) << " (Expected: 15)" << std::endl;
        
        // Test case 3
        std::cout << "Test 3: " << Task11::nb_year(1500000, 2.5, 10000, 2000000) << " (Expected: 10)" << std::endl;
        
        // Test case 4
        std::cout << "Test 4: " << Task11::nb_year(1000, 0, 50, 1200) << " (Expected: 4)" << std::endl;
        
        // Test case 5
        std::cout << "Test 5: " << Task11::nb_year(1500, 10, 200, 10000) << " (Expected: 14)" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
