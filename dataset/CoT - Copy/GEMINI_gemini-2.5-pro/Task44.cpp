#include <iostream>
#include <string>
#include <limits> // Required for numeric_limits
#include <iomanip> // Required for setprecision

// Using a namespace for our functions
namespace Calc {

/**
 * @brief Performs a basic arithmetic operation after validating inputs.
 * 
 * @param num1 The first integer.
 * @param num2 The second integer.
 * @param op The character representing the operation (+, -, *, /).
 */
void performOperation(int num1, int num2, char op) {
    double result = 0.0;
    switch (op) {
        case '+':
            result = static_cast<double>(num1) + num2;
            break;
        case '-':
            result = static_cast<double>(num1) - num2;
            break;
        case '*':
            result = static_cast<double>(num1) * num2;
            break;
        case '/':
            // Security: Validate against division by zero.
            if (num2 == 0) {
                std::cout << "Error: Division by zero is not allowed." << std::endl;
                return;
            }
            result = static_cast<double>(num1) / num2;
            break;
        default:
            // Security: Validate the operator is one of the allowed characters.
            std::cout << "Error: Invalid operator '" << op << "'." << std::endl;
            return;
    }
    std::cout << num1 << " " << op << " " << num2 << " = " 
              << std::fixed << std::setprecision(2) << result << std::endl;
}

} // namespace Calc

// Dummy class to satisfy naming requirement if needed, but not idiomatic for C++.
class Task44 {
public:
    static void runTests() {
        // --- Test Cases ---
        std::cout << "--- Running Automated Test Cases ---" << std::endl;

        // Test Case 1: Addition
        std::cout << "Test Case 1 (Addition):" << std::endl;
        std::cout << "Input: 10, 5, '+'" << std::endl;
        std::cout << "Output: ";
        Calc::performOperation(10, 5, '+');
        std::cout << std::endl;

        // Test Case 2: Subtraction
        std::cout << "Test Case 2 (Subtraction):" << std::endl;
        std::cout << "Input: 10, 15, '-'" << std::endl;
        std::cout << "Output: ";
        Calc::performOperation(10, 15, '-');
        std::cout << std::endl;

        // Test Case 3: Valid Division
        std::cout << "Test Case 3 (Valid Division):" << std::endl;
        std::cout << "Input: 20, 4, '/'" << std::endl;
        std::cout << "Output: ";
        Calc::performOperation(20, 4, '/');
        std::cout << std::endl;

        // Test Case 4: Invalid Division (by zero)
        std::cout << "Test Case 4 (Division by Zero):" << std::endl;
        std::cout << "Input: 10, 0, '/'" << std::endl;
        std::cout << "Output: ";
        Calc::performOperation(10, 0, '/');
        std::cout << std::endl;

        // Test Case 5: Invalid Operator
        std::cout << "Test Case 5 (Invalid Operator):" << std::endl;
        std::cout << "Input: 7, 8, '%'" << std::endl;
        std::cout << "Output: ";
        Calc::performOperation(7, 8, '%');
        std::cout << std::endl;
    }

    static void runInteractive() {
        // --- Interactive Section (Example of secure input handling) ---
        std::cout << "\n--- Starting Interactive Session ---" << std::endl;
        int num1, num2;
        char op;

        // Securely read first integer
        std::cout << "Enter first integer: ";
        while (!(std::cin >> num1)) {
            // Security: Handle non-numeric input to prevent errors/infinite loops.
            std::cout << "Error: Invalid input. Please enter an integer: ";
            std::cin.clear(); // Clear the error flag
            // Discard the invalid input from the buffer
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); 
        }

        // Securely read second integer
        std::cout << "Enter second integer: ";
        while (!(std::cin >> num2)) {
            std::cout << "Error: Invalid input. Please enter an integer: ";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        // Securely read operator
        std::cout << "Enter an operator (+, -, *, /): ";
        while (std::cin >> op && (op != '+' && op != '-' && op != '*' && op != '/')) {
            std::cout << "Error: Invalid operator. Please use +, -, *, or /: ";
            // No need to clear buffer for single char, but good practice if reading strings
        }

        std::cout << "Result: ";
        Calc::performOperation(num1, num2, op);
    }
};

int main() {
    Task44::runTests();
    // To run interactively, uncomment the following line:
    // Task44::runInteractive();
    return 0;
}