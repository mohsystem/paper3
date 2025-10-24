#include <iostream>
#include <string>
#include <stdexcept>
#include <limits>

// Function to perform division, throws an exception on division by zero.
// Rule #5: Ensure that all exceptions are caught and handled appropriately.
// Rule #6: Keep functions small and testable.
double performDivision(int numerator, int denominator) {
    if (denominator == 0) {
        throw std::runtime_error("Error: Division by zero is not allowed.");
    }
    return static_cast<double>(numerator) / denominator;
}

// Function to convert a string to an integer with robust error checking.
// Rule #1: Ensure all input is validated and sanitized.
// Rule #6: Treat all inputs as untrusted.
int stringToInteger(const std::string& inputStr) {
    size_t pos;
    long long long_val;

    // Use std::stoll to parse as long long to detect overflow for int
    try {
        long_val = std::stoll(inputStr, &pos);
    } catch (const std::invalid_argument&) {
        throw std::invalid_argument("Error: Input is not a valid number.");
    } catch (const std::out_of_range&) {
        throw std::out_of_range("Error: Input number is out of range for a long long.");
    }
    
    // Rule #1: Check for trailing characters after the number.
    if (pos != inputStr.length()) {
        throw std::invalid_argument("Error: Input contains non-numeric characters.");
    }
    
    // Rule #1: Validate range to ensure it fits within an integer.
    if (long_val < std::numeric_limits<int>::min() || long_val > std::numeric_limits<int>::max()) {
        throw std::out_of_range("Error: Input number is out of the valid integer range.");
    }

    return static_cast<int>(long_val);
}

// Encapsulates the logic for a single division operation to be called by tests.
void processDivision(const std::string& numStr, const std::string& denStr) {
    try {
        // Rule #1: Validate and convert inputs first.
        int numerator = stringToInteger(numStr);
        int denominator = stringToInteger(denStr);

        // Perform the operation
        double result = performDivision(numerator, denominator);
        
        std::cout << "Input: (" << numStr << " / " << denStr << ")" << std::endl;
        std::cout << "Result: " << result << std::endl;
    } 
    // Rule #5: Catch specific exceptions first.
    catch (const std::invalid_argument& e) {
        std::cerr << "Input: (" << numStr << " / " << denStr << ")" << std::endl;
        std::cerr << "Caught invalid_argument: " << e.what() << std::endl;
    } catch (const std::out_of_range& e) {
        std::cerr << "Input: (" << numStr << " / " << denStr << ")" << std::endl;
        std::cerr << "Caught out_of_range: " << e.what() << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << "Input: (" << numStr << " / " << denStr << ")" << std::endl;
        std::cerr << "Caught runtime_error: " << e.what() << std::endl;
    } 
    // General catch-all for any other unforeseen exceptions.
    catch (const std::exception& e) {
        std::cerr << "Input: (" << numStr << " / " << denStr << ")" << std::endl;
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
    }
    std::cout << "--------------------" << std::endl;
}

int main() {
    // --- Test Cases ---

    // 1. Valid input
    std::cout << "Test Case 1: Valid input" << std::endl;
    processDivision("100", "5");

    // 2. Division by zero
    std::cout << "Test Case 2: Division by zero" << std::endl;
    processDivision("42", "0");

    // 3. Invalid numerator (non-numeric)
    std::cout << "Test Case 3: Invalid numerator" << std::endl;
    processDivision("abc", "5");
    
    // 4. Invalid denominator (contains trailing non-numeric characters)
    std::cout << "Test Case 4: Trailing characters in denominator" << std::endl;
    processDivision("100", "5x");

    // 5. Out of range input
    std::cout << "Test Case 5: Out of range input" << std::endl;
    processDivision("3000000000", "2"); // Larger than max int

    return 0;
}