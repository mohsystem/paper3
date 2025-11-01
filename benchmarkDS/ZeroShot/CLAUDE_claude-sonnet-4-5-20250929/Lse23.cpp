
#include <iostream>
#include <random>

class Lse23 {
public:
    static void generateAndSum() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dis(1001, INT_MAX);
        
        // Generate three random integers larger than 1000
        int a = dis(gen);
        int b = dis(gen);
        int c = dis(gen);
        
        // Print the three integers
        std::cout << "a : " << a << std::endl;
        std::cout << "b : " << b << std::endl;
        std::cout << "c : " << c << std::endl;
        
        // Add and print the result
        long long sum = (long long)a + (long long)b + (long long)c;
        long long sum2 = (long long)a + (long long)b + (long long)c;
        std::cout << "sum: " << sum << std::endl;
        std::cout << "sum2: " << sum2 << std::endl;
    }
};

int main() {
    // Run 5 test cases
    for (int i = 1; i <= 5; i++) {
        std::cout << "Test case " << i << ":" << std::endl;
        Lse23::generateAndSum();
        std::cout << std::endl;
    }
    return 0;
}
