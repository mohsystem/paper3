
#include <iostream>
#include <random>
#include <cstdint>

const int MAX_SALES = 100000;

class Lse25 {
private:
    static std::random_device rd;
    static std::mt19937 gen;
    static std::uniform_int_distribution<int> dis;
    
public:
    static int getMonthlySales() {
        return dis(gen);
    }
    
    static void calculateQuarterlySales() {
        int64_t q1 = 0, q2 = 0, q3 = 0, q4 = 0;
        
        // Quarter 1 (Months 1-3)
        for (int i = 0; i < 3; i++) {
            q1 += getMonthlySales();
        }
        
        // Quarter 2 (Months 4-6)
        for (int i = 0; i < 3; i++) {
            q2 += getMonthlySales();
        }
        
        // Quarter 3 (Months 7-9)
        for (int i = 0; i < 3; i++) {
            q3 += getMonthlySales();
        }
        
        // Quarter 4 (Months 10-12)
        for (int i = 0; i < 3; i++) {
            q4 += getMonthlySales();
        }
        
        std::cout << "Quarterly Sales Report:" << std::endl;
        std::cout << "Q1 Sales: $" << q1 << std::endl;
        std::cout << "Q2 Sales: $" << q2 << std::endl;
        std::cout << "Q3 Sales: $" << q3 << std::endl;
        std::cout << "Q4 Sales: $" << q4 << std::endl;
        std::cout << "Total Annual Sales: $" << (q1 + q2 + q3 + q4) << std::endl;
        std::cout << std::endl;
    }
};

std::random_device Lse25::rd;
std::mt19937 Lse25::gen(Lse25::rd());
std::uniform_int_distribution<int> Lse25::dis(0, MAX_SALES);

int main() {
    std::cout << "Running 5 test cases:\\n" << std::endl;
    for (int i = 1; i <= 5; i++) {
        std::cout << "Test Case " << i << ":" << std::endl;
        Lse25::calculateQuarterlySales();
    }
    return 0;
}
