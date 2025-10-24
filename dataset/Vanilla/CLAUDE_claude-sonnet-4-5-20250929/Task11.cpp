
#include <iostream>

class Task11 {
public:
    static int nbYear(int p0, double percent, int aug, int p) {
        int years = 0;
        int currentPopulation = p0;
        double percentDecimal = percent / 100.0;
        
        while (currentPopulation < p) {
            currentPopulation = static_cast<int>(currentPopulation + currentPopulation * percentDecimal + aug);
            years++;
        }
        
        return years;
    }
};

int main() {
    // Test cases
    std::cout << "Test 1: " << Task11::nbYear(1000, 2, 50, 1200) << " (Expected: 3)" << std::endl;
    std::cout << "Test 2: " << Task11::nbYear(1500, 5, 100, 5000) << " (Expected: 15)" << std::endl;
    std::cout << "Test 3: " << Task11::nbYear(1500000, 2.5, 10000, 2000000) << " (Expected: 10)" << std::endl;
    std::cout << "Test 4: " << Task11::nbYear(1000, 2, 0, 1200) << " (Expected: 10)" << std::endl;
    std::cout << "Test 5: " << Task11::nbYear(1000, 0, 50, 1500) << " (Expected: 10)" << std::endl;
    
    return 0;
}
