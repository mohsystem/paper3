#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

// A struct to return both the result string and a success/failure flag.
struct Result {
    std::string value;
    bool success;
};

/**
 * @brief Retrieves a command-line argument by an index specified in the first argument.
 *
 * @param argc The argument count.
 * @param argv The argument vector. argv[1] is expected to be the index.
 * @return A Result struct containing the argument or an error message.
 */
Result getArgumentAtIndex(int argc, char* argv[]) {
    if (argc < 2) {
        return {"Error: No index provided.", false};
    }

    int targetIndex;
    try {
        targetIndex = std::stoi(argv[1]);
    } catch (const std::invalid_argument& e) {
        return {"Error: The first argument must be a valid integer index.", false};
    } catch (const std::out_of_range& e) {
        return {"Error: Index is out of range for an integer.", false};
    }

    // We enforce positive indices, as argv[0] is the program name and 
    // argv[1] is the index itself.
    if (targetIndex <= 0) {
        return {"Error: Index must be a positive integer.", false};
    }

    if (targetIndex >= argc) {
        return {"Error: Index " + std::to_string(targetIndex) + " is out of bounds. There are only " + std::to_string(argc) + " arguments.", false};
    }

    return {std::string(argv[targetIndex]), true};
}

// Helper function to run a single test case and print its output.
void run_test_case(const std::string& name, int argc, char* argv[]) {
    std::cout << name << " ";
    for (int i = 0; i < argc; ++i) {
        std::cout << argv[i] << " ";
    }
    std::cout << std::endl;
    Result res = getArgumentAtIndex(argc, argv);
    std::cout << "Output: " << res.value << std::endl << std::endl;
}

int main(int argc, char* argv[]) {
    // To run with actual command-line arguments, compile and execute:
    // g++ your_program.cpp -o program -std=c++11
    // ./program 2 zero one two three  (should print "one")
    // ./program 1 one two             (should print "1")
    // ./program 5 1 2 3             (should print error)
    // ./program -1 1 2 3            (should print error)
    // ./program 0 1 2 3             (should print error)
    // ./program abc 1 2 3           (should print error)
    
    std::cout << "--- Running with actual command line arguments ---" << std::endl;
    Result result = getArgumentAtIndex(argc, argv);
    std::cout << "Output: " << result.value << std::endl;
    std::cout << "------------------------------------------------\n" << std::endl;

    std::cout << "--- Running with simulated test cases ---" << std::endl;

    // Test Case 1: Valid index
    char* test1_argv[] = {(char*)"./program", (char*)"3", (char*)"apple", (char*)"banana", (char*)"cherry", nullptr};
    run_test_case("Test Case 1:", (sizeof(test1_argv)/sizeof(char*)) - 1, test1_argv); // Expected: banana

    // Test Case 2: Index out of bounds (too high)
    char* test2_argv[] = {(char*)"./program", (char*)"3", (char*)"dog", (char*)"cat", nullptr};
    run_test_case("Test Case 2:", (sizeof(test2_argv)/sizeof(char*)) - 1, test2_argv); // Expected: Error

    // Test Case 3: Invalid index (not a number)
    char* test3_argv[] = {(char*)"./program", (char*)"one", (char*)"two", (char*)"three", nullptr};
    run_test_case("Test Case 3:", (sizeof(test3_argv)/sizeof(char*)) - 1, test3_argv); // Expected: Error

    // Test Case 4: Invalid index (zero)
    char* test4_argv[] = {(char*)"./program", (char*)"0", (char*)"a", (char*)"b", nullptr};
    run_test_case("Test Case 4:", (sizeof(test4_argv)/sizeof(char*)) - 1, test4_argv); // Expected: Error
    
    // Test Case 5: Invalid index (negative)
    char* test5_argv[] = {(char*)"./program", (char*)"-1", (char*)"a", (char*)"b", nullptr};
    run_test_case("Test Case 5:", (sizeof(test5_argv)/sizeof(char*)) - 1, test5_argv); // Expected: Error

    return 0;
}