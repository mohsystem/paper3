#include <iostream>

class Task11 {
public:
    /**
     * Calculates the number of years required for a population to reach a target.
     *
     * @param p0      The initial population.
     * @param percent The annual percentage increase.
     * @param aug     The number of inhabitants coming or leaving each year.
     * @param p       The target population to surpass.
     * @return The number of full years needed.
     */
    static int nbYear(int p0, double percent, int aug, int p) {
        int years = 0;
        int currentPopulation = p0;
        double percentDecimal = percent / 100.0;

        while (currentPopulation < p) {
            // Calculate the population for the next year and truncate to the nearest whole number.
            currentPopulation = static_cast<int>(currentPopulation + currentPopulation * percentDecimal + aug);
            years++;
        }
        return years;
    }
};

int main() {
    // Test Case 1: From example
    std::cout << Task11::nbYear(1500, 5, 100, 5000) << std::endl;
    // Test Case 2: From example
    std::cout << Task11::nbYear(1500000, 2.5, 10000, 2000000) << std::endl;
    // Test Case 3: Initial prompt example
    std::cout << Task11::nbYear(1000, 2, 50, 1200) << std::endl;
    // Test Case 4: Initial population is already at or above target
    std::cout << Task11::nbYear(1200, 2, 50, 1200) << std::endl;
    // Test Case 5: Small growth
    std::cout << Task11::nbYear(1000, 0.5, 2, 1010) << std::endl;

    return 0;
}