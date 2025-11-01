#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>

// Global constant data array
const std::vector<int> DATA = {100, 200, 300, 400, 500};

/**
 * @brief Parses an index from args, validates it, and prints the element from DATA.
 * @param argc Argument count.
 * @param argv Argument vector.
 */
void printElementAtIndex(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ./a.out <index>" << std::endl;
        return;
    }

    try {
        std::string arg = argv[1];
        size_t pos;
        int index = std::stoi(arg, &pos);

        // std::stoi can parse "123xyz" as 123. Ensure the entire string is numeric.
        if (pos != arg.length()) {
            std::cerr << "Error: Argument '" << arg << "' contains non-numeric characters." << std::endl;
            return;
        }

        // Using .at() for automatic bounds checking, which throws std::out_of_range
        int value = DATA.at(index);
        std::cout << "Element at index " << index << ": " << value << std::endl;

    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: Argument is not a valid integer." << std::endl;
    } catch (const std::out_of_range& e) {
        // This catch handles both std::stoi overflow and DATA.at() out-of-bounds access.
        std::cerr << "Error: Index is out of bounds. Valid range is 0 to " << (DATA.size() - 1) << "." << std::endl;
    }
}

int main(int argc, char* argv[]) {
    // This block allows the program to be run with actual command-line arguments.
    if (argc > 1) {
        std::cout << "--- Running with actual command-line arguments ---" << std::endl;
        printElementAtIndex(argc, argv);
        std::cout << "-----------------------------------------------" << std::endl << std::endl;
    }

    // Running 5 test cases as requested.
    std::cout << "--- Running 5 built-in test cases ---" << std::endl;
    
    char arg1[] = "2";
    char* argv1[] = { (char*)"./a.out", arg1, NULL };
    std::cout << "\n[Test 1] Valid index '2':" << std::endl;
    printElementAtIndex(2, argv1);

    char arg2[] = "10";
    char* argv2[] = { (char*)"./a.out", arg2, NULL };
    std::cout << "\n[Test 2] Out of bounds index '10':" << std::endl;
    printElementAtIndex(2, argv2);

    char arg3[] = "-1";
    char* argv3[] = { (char*)"./a.out", arg3, NULL };
    std::cout << "\n[Test 3] Negative index '-1':" << std::endl;
    printElementAtIndex(2, argv3);

    char arg4[] = "abc";
    char* argv4[] = { (char*)"./a.out", arg4, NULL };
    std::cout << "\n[Test 4] Non-numeric input 'abc':" << std::endl;
    printElementAtIndex(2, argv4);

    char* argv5[] = { (char*)"./a.out", NULL };
    std::cout << "\n[Test 5] No arguments:" << std::endl;
    printElementAtIndex(1, argv5);

    std::cout << "\n--- Test cases finished ---" << std::endl;

    return 0;
}