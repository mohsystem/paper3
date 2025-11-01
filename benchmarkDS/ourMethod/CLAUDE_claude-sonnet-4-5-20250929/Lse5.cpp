
#include <iostream>
#include <string>
#include <cstdlib>
#include <cerrno>
#include <climits>
#include <cstring>

const std::string getElementAtIndex(int argc, char* argv[]) {
    // Predefined array of elements
    const std::string elements[] = {"apple", "banana", "cherry", "date", "elderberry"};
    const size_t elementsSize = sizeof(elements) / sizeof(elements[0]);
    
    // Validate that we have at least one argument (argc > 1 means at least one arg after program name)
    if (argc < 2 || argv == nullptr || argv[1] == nullptr) {
        return "Error: No index provided";
    }
    
    // Validate and parse the index
    char* endptr = nullptr;
    errno = 0;
    
    // Use strtol for safe conversion with error checking
    long indexLong = std::strtol(argv[1], &endptr, 10);
    
    // Check for conversion errors
    if (errno == ERANGE || endptr == argv[1] || *endptr != '\\0') {
        return "Error: Invalid index format";
    }
    
    // Check for integer overflow/underflow
    if (indexLong < 0 || indexLong > INT_MAX) {
        return "Error: Index out of bounds";
    }
    
    int index = static_cast<int>(indexLong);
    
    // Bounds checking - ensure index is within valid range
    if (index < 0 || static_cast<size_t>(index) >= elementsSize) {
        return "Error: Index out of bounds";
    }
    
    // Return the element at the validated index
    return elements[index];
}

int main(int argc, char* argv[]) {
    // Test case 1: Valid index 0
    char* test1[] = {(char*)"program", (char*)"0"};
    std::cout << "Test 1 (index 0): " << getElementAtIndex(2, test1) << std::endl;
    
    // Test case 2: Valid index 2
    char* test2[] = {(char*)"program", (char*)"2"};
    std::cout << "Test 2 (index 2): " << getElementAtIndex(2, test2) << std::endl;
    
    // Test case 3: Valid index 4 (last element)
    char* test3[] = {(char*)"program", (char*)"4"};
    std::cout << "Test 3 (index 4): " << getElementAtIndex(2, test3) << std::endl;
    
    // Test case 4: Invalid index (out of bounds)
    char* test4[] = {(char*)"program", (char*)"10"};
    std::cout << "Test 4 (index 10): " << getElementAtIndex(2, test4) << std::endl;
    
    // Test case 5: Invalid input (not a number)
    char* test5[] = {(char*)"program", (char*)"abc"};
    std::cout << "Test 5 (invalid): " << getElementAtIndex(2, test5) << std::endl;
    
    return 0;
}
