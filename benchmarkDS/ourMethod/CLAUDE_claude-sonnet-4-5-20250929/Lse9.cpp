
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <limits>

/**
 * Retrieves and returns the command line argument at the specified index.
 * 
 * @param args The command line arguments vector
 * @param index The index to retrieve (0-based after program name)
 * @return The argument at the specified index, or an error message if invalid
 */
std::string getArgumentAtIndex(const std::vector<std::string>& args, int index) {
    // Validate index is non-negative to prevent negative array access
    if (index < 0) {
        return "Error: Index cannot be negative";
    }
    
    // Check if index is within bounds to prevent buffer over-read
    // Cast to size_t after validation to ensure safe comparison
    if (static_cast<size_t>(index) >= args.size()) {
        return "Error: Index out of bounds";
    }
    
    // Return the argument at the valid index
    return args[static_cast<size_t>(index)];
}

int main(int argc, char* argv[]) {
    // Test case 1: Valid index within bounds
    std::vector<std::string> test1 = {"arg0", "arg1", "arg2", "arg3"};
    std::cout << "Test 1 - Index 1: " << getArgumentAtIndex(test1, 1) << std::endl;
    
    // Test case 2: Valid index at boundary
    std::vector<std::string> test2 = {"first", "second", "third"};
    std::cout << "Test 2 - Index 2: " << getArgumentAtIndex(test2, 2) << std::endl;
    
    // Test case 3: Index out of bounds (should return error)
    std::vector<std::string> test3 = {"only", "two"};
    std::cout << "Test 3 - Index 5: " << getArgumentAtIndex(test3, 5) << std::endl;
    
    // Test case 4: Negative index (should return error)
    std::vector<std::string> test4 = {"some", "args"};
    std::cout << "Test 4 - Index -1: " << getArgumentAtIndex(test4, -1) << std::endl;
    
    // Test case 5: Empty vector with index 0 (should return error)
    std::vector<std::string> test5;
    std::cout << "Test 5 - Index 0 on empty: " << getArgumentAtIndex(test5, 0) << std::endl;
    
    // Actual program logic: process command line arguments if provided
    // Validate argc to prevent buffer over-read
    if (argc > 1 && argv != nullptr) {
        // Validate argv[1] is not null before dereferencing
        if (argv[1] != nullptr) {
            try {
                // Parse the first argument as the index with overflow protection
                char* endptr = nullptr;
                long long_index = std::strtol(argv[1], &endptr, 10);
                
                // Validate conversion was successful and entire string was consumed
                if (endptr == argv[1] || *endptr != '\\0') {
                    std::cout << "Error: Invalid index format" << std::endl;
                    return 1;
                }
                
                // Check for overflow when converting long to int
                if (long_index > std::numeric_limits<int>::max() || 
                    long_index < std::numeric_limits<int>::min()) {
                    std::cout << "Error: Index value out of range" << std::endl;
                    return 1;
                }
                
                int index = static_cast<int>(long_index);
                
                // Build vector of actual arguments (starting from argv[2])
                std::vector<std::string> actualArgs;
                for (int i = 2; i < argc && argv[i] != nullptr; ++i) {
                    actualArgs.push_back(std::string(argv[i]));
                }
                
                // Retrieve and print the argument at the specified index
                std::string result = getArgumentAtIndex(actualArgs, index);
                std::cout << result << std::endl;
                
            } catch (const std::exception& e) {
                // Handle exceptions without exposing internal details
                std::cout << "Error: Failed to process arguments" << std::endl;
                return 1;
            }
        }
    }
    
    return 0;
}
