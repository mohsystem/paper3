#include <iostream>
#include <cstdlib>
#include <ctime>

void generateAndSumRandoms() {
    int a = rand() % 8999 + 1001;
    int b = rand() % 8999 + 1001;
    int c = rand() % 8999 + 1001;
    
    std::cout << "a : " << a << std::endl;
    std::cout << "b : " << b << std::endl;
    std::cout << "c : " << c << std::endl;
    
    int sum = a + b + c;
    std::cout << "sum: " << sum << std::endl;
    std::cout << "sum2: " << sum << std::endl;
}

int main() {
    srand(static_cast<unsigned int>(time(0)));
    
    for (int i = 0; i < 5; ++i) {
        generateAndSumRandoms();
        if (i < 4) {
            std::cout << std::endl;
        }
    }
    
    return 0;
}