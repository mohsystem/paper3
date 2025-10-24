#include <iostream>
#include <stdexcept>
#include <vector>
#include <tuple>
#include <iomanip>

/**
 * @brief Performs a basic arithmetic operation.
 * 
 * @param num1 The first operand.
 * @param num2 The second operand.
 * @param op The operator character (+, -, *, /).
 * @return The result of the operation.
 * @throw std::invalid_argument if the operator is invalid or if division by zero is attempted.
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
            if (num2 == 0.0) {
                throw std::invalid_argument("Error: Division by zero is not allowed.");
            }
            return num1 / num2;
        default:
            throw std::invalid_argument("Error: Invalid operator. Only +, -, *, / are allowed.");
    }
}

int main() {
    std::vector<std::tuple<double, double, char>> test_cases = {
        {10.0, 5.0, '+'},
        {15.0, 3.0, '*'},
        {7.5, 2.5, '-'},
        {12.0, 0.0, '/'}, // Division by zero case
        {10.0, 3.0, '%'}  // Invalid operator case
    };

    int test_num = 1;
    for (const auto& test : test_cases) {
        double num1 = std::get<0>(test);
        double num2 = std::get<1>(test);
        char op = std::get<2>(test);

        std::cout << "Test Case " << test_num++ << ": " << num1 << " " << op << " " << num2 << std::endl;
        try {
            double result = calculate(num1, num2, op);
            std::cout << "Result: " << std::fixed << std::setprecision(2) << result << std::endl << std::endl;
        } catch (const std::invalid_argument& e) {
            std::cerr << e.what() << std::endl << std::endl;
        }
    }

    return 0;
}