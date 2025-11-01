// Chain-of-Through process:
// 1) Problem understanding:
//    - Create an array of elements.
//    - Read a command-line argument, convert it to an integer index, print element at that index.
//    - Provide a function that accepts inputs as parameters and returns the output.
// 2) Security requirements:
//    - Validate presence of argument, safe integer parsing, bounds checks.
// 3) Secure coding generation:
//    - Use std::stoll with try/catch, ensure 0 <= index < size before access.
// 4) Code review:
//    - No unchecked access; clear error handling; no undefined behavior.
// 5) Secure code output:
//    - Final code ensures robust input handling and safe access.

#include <iostream>
#include <vector>
#include <string>

static const std::vector<std::string> ELEMENTS = { "alpha", "beta", "gamma", "delta", "epsilon" };

// Function: accepts inputs as parameters and returns the output if any.
// Returns empty string if invalid index (since our elements are non-empty).
std::string elementAtIndex(const std::vector<std::string>& elements, long long index) {
    if (index < 0 || static_cast<size_t>(index) >= elements.size()) {
        return std::string();
    }
    return elements[static_cast<size_t>(index)];
}

static void processArgument(const std::string& arg) {
    try {
        size_t pos = 0;
        long long idx = std::stoll(arg, &pos, 10);
        if (pos != arg.size()) {
            std::cerr << "Error: Index must be a valid integer." << std::endl;
            return;
        }
        std::string result = elementAtIndex(ELEMENTS, idx);
        if (result.empty()) {
            std::cerr << "Error: Index out of bounds. Valid range is 0 to " << (ELEMENTS.size() - 1) << "." << std::endl;
        } else {
            std::cout << result << std::endl;
        }
    } catch (const std::exception&) {
        std::cerr << "Error: Index must be a valid integer." << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc >= 2) {
        processArgument(std::string(argv[1]));
    } else {
        // 5 test cases
        std::vector<long long> tests = { 0, 2, 4, -1, 5 };
        for (long long t : tests) {
            std::string res = elementAtIndex(ELEMENTS, t);
            if (res.empty()) {
                std::cerr << "Test index " << t << ": Error - Index out of bounds. Valid range is 0 to " << (ELEMENTS.size() - 1) << "." << std::endl;
            } else {
                std::cout << "Test index " << t << ": " << res << std::endl;
            }
        }
    }
    return 0;
}