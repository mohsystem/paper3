#include <iostream>
#include <stdexcept>
#include <vector>
#include <tuple>
#include <iomanip>

/**
 * @brief Performs a simple arithmetic operation.
 * @param num1 The first number.
 * @param num2 The second number.
 * @param op The operator character (+, -, *, /).
 * @return The result of the operation.
 * @throws std::invalid_argument if the operator is invalid or division by zero occurs.
 */
double calculate(double num1, double num2, char op) {
    switch (op) {
        case '+':
            return num1 + num2;
        case '-':
            return num1 - num2;
        case '*':
            return num1 * num2;
        case '/':
            if (num2 == 0) {
                throw std::invalid_argument("Error: Division by zero is not allowed.");
            }
            return num1 / num2;
        default:
            throw std::invalid_argument("Error: Invalid operator. Only +, -, *, / are supported.");
    }
}

int main() {
    std::vector<std::tuple<double, double, char>> test_cases = {
        {10.0, 5.0, '+'},
        {20.5, 4.5, '-'},
        {7.0, 6.0, '*'},
        {15.0, 4.0, '/'},
        {10.0, 0.0, '/'}
    };

    std::cout << "--- C++ Test Cases ---" << std::endl;
    int i = 1;
    for (const auto& test : test_cases) {
        double num1 = std::get<0>(test);
        double num2 = std::get<1>(test);
        char op = std::get<2>(test);
        try {
            double result = calculate(num1, num2, op);
            std::cout << "Test Case " << i << ": " << num1 << " " << op << " " << num2 << " = "
                      << std::fixed << std::setprecision(2) << result << std::endl;
        } catch (const std::invalid_argument& e) {
            std::cout << "Test Case " << i << ": " << num1 << " " << op << " " << num2 << " -> "
                      << e.what() << std::endl;
        }
        i++;
    }

    return 0;
}