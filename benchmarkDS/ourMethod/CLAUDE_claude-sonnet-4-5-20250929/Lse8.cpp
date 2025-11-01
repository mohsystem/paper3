
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <climits>
#include <cerrno>

/**
 * Returns the command line argument at the index specified by argv[1].
 * 
 * @param argc Argument count
 * @param argv Argument vector (array of C-strings)
 * @return The argument at the specified index, or nullptr if invalid
 */
const char* getArgumentAtIndex(int argc, char* argv[]) {
    // Check if we have at least 2 arguments (argv[0] and argv[1])
    if (argv == nullptr || argc < 2) {
        return nullptr;
    }
    
    // Validate argv[1] is not null before processing
    if (argv[1] == nullptr) {
        return nullptr;
    }
    
    // Parse the index from argv[1]
    char* endptr = nullptr;
    errno = 0;
    long index_long = strtol(argv[1], &endptr, 10);
    
    // Check for conversion errors:
    // - endptr points to argv[1]: no digits were found
    // - *endptr != '\\0': extra characters after number
    // - errno is set: overflow/underflow occurred
    if (endptr == argv[1] || *endptr != '\\0' || errno == ERANGE) {
        return nullptr;
    }
    
    // Validate index fits in int range and is non-negative
    if (index_long < 0 || index_long > INT_MAX) {
        return nullptr;
    }
    
    int index = static_cast<int>(index_long);
    
    // Check bounds to prevent out-of-bounds access
    if (index >= argc) {
        return nullptr;
    }
    
    // Validate the target argument is not null before returning
    if (argv[index] == nullptr) {
        return nullptr;
    }
    
    // Return the value at the validated index
    return argv[index];
}

int main() {
    // Test case 1: Valid index within bounds
    char* test1[] = {(char*)"program", (char*)"2", (char*)"hello", (char*)"world"};
    const char* result1 = getArgumentAtIndex(4, test1);
    std::cout << "Test 1: " << (result1 ? result1 : "null") << std::endl;
    
    // Test case 2: Index out of bounds
    char* test2[] = {(char*)"program", (char*)"5", (char*)"hello"};
    const char* result2 = getArgumentAtIndex(3, test2);
    std::cout << "Test 2: " << (result2 ? result2 : "null") << std::endl;
    
    // Test case 3: Negative index
    char* test3[] = {(char*)"program", (char*)"-1", (char*)"hello"};
    const char* result3 = getArgumentAtIndex(3, test3);
    std::cout << "Test 3: " << (result3 ? result3 : "null") << std::endl;
    
    // Test case 4: Invalid index format
    char* test4[] = {(char*)"program", (char*)"abc", (char*)"hello"};
    const char* result4 = getArgumentAtIndex(3, test4);
    std::cout << "Test 4: " << (result4 ? result4 : "null") << std::endl;
    
    // Test case 5: Insufficient arguments
    char* test5[] = {(char*)"program"};
    const char* result5 = getArgumentAtIndex(1, test5);
    std::cout << "Test 5: " << (result5 ? result5 : "null") << std::endl;
    
    return 0;
}
