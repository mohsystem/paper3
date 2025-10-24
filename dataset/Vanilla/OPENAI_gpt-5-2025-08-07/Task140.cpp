#include <iostream>
#include <limits>

double calculate(double a, double b, char op) {
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': return (b != 0) ? (a / b) : std::numeric_limits<double>::quiet_NaN();
        default: return std::numeric_limits<double>::quiet_NaN();
    }
}

int main() {
    std::cout << "3 + 5 = " << calculate(3, 5, '+') << std::endl;
    std::cout << "10 - 4 = " << calculate(10, 4, '-') << std::endl;
    std::cout << "7 * 6 = " << calculate(7, 6, '*') << std::endl;
    std::cout << "20 / 5 = " << calculate(20, 5, '/') << std::endl;
    std::cout << "7 / 0 = " << calculate(7, 0, '/') << std::endl;
    return 0;
}