#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

/**
 * @brief Retrieves the value from the arguments array at the index specified by argv[1].
 * 
 * @param argc The argument count.
 * @param argv The array of command-line arguments.
 * @return The string at the specified index or an error message.
 */
std::string getValueAtIndex(int argc, char* argv[]) {
    if (argc < 2) {
        return "Error: Not enough arguments. Please provide an index.";
    }

    int index;
    try {
        index = std::stoi(argv[1]);
    } catch (const std::invalid_argument& e) {
        return "Error: The index provided in the second argument must be an integer.";
    } catch (const std::out_of_range& e) {
        return "Error: The index provided is out of range for an integer.";
    }

    if (index < 0 || index >= argc) {
        return "Error: Index " + std::to_string(index) + " is out of bounds for the given arguments.";
    }

    return argv[index];
}

void run_test_case(const std::string& name, int argc, char* argv[]) {
    std::cout << "\n" << name << ": [";
    for (int i = 0; i < argc; ++i) {
        std::cout << "\"" << argv[i] << "\"" << (i == argc - 1 ? "" : ", ");
    }
    std::cout << "]" << std::endl;
    std::cout << "Output: " << getValueAtIndex(argc, argv) << std::endl;
}

int main(int argc, char* argv[]) {
    // Example with actual command line arguments
    std::cout << "--- Processing Actual Command Line Arguments ---" << std::endl;
    std::cout << "Arguments received: [";
    for (int i = 0; i < argc; ++i) {
        std::cout << "\"" << argv[i] << "\"" << (i == argc - 1 ? "" : ", ");
    }
    std::cout << "]" << std::endl;
    std::string result = getValueAtIndex(argc, argv);
    std::cout << "Result: " << result << std::endl;
    std::cout << "\n--- Running 5 Hardcoded Test Cases ---" << std::endl;

    // Test Case 1: Valid case
    char* test1_argv[] = {(char*)"program", (char*)"3", (char*)"apple", (char*)"banana", (char*)"cherry"};
    run_test_case("Test 1", 5, test1_argv); // Expected: cherry

    // Test Case 2: Index out of bounds
    char* test2_argv[] = {(char*)"program", (char*)"5", (char*)"one", (char*)"two"};
    run_test_case("Test 2", 4, test2_argv); // Expected: Error

    // Test Case 3: Index is not a number
    char* test3_argv[] = {(char*)"program", (char*)"foo", (char*)"bar"};
    run_test_case("Test 3", 3, test3_argv); // Expected: Error

    // Test Case 4: Not enough arguments
    char* test4_argv[] = {(char*)"program"};
    run_test_case("Test 4", 1, test4_argv); // Expected: Error

    // Test Case 5: Index points to itself
    char* test5_argv[] = {(char*)"program", (char*)"1", (char*)"arg2", (char*)"arg3"};
    run_test_case("Test 5", 4, test5_argv); // Expected: 1

    return 0;
}