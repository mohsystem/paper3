#include <iostream>
#include <vector>
#include <string>
#include <cstdlib> // For rand(), srand(), RAND_MAX
#include <ctime>   // For time()
#include <cstdio>  // For snprintf

/**
 * Generates three random float numbers and converts them to strings.
 * Note: Uses C-style rand() and snprintf as per the prompt's implication.
 * For modern C++, the <random> library is generally preferred over rand().
 *
 * @return A vector of three strings, each representing a random float.
 */
std::vector<std::string> generateRandomFloatStrings() {
    // Generate three random floats between 0.0 and 1.0
    float f1 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    float f2 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    float f3 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

    // Buffers to hold the string representation of floats.
    // A size of 50 is safely larger than needed for a standard float.
    char string1_buffer[50];
    char string2_buffer[50];
    char string3_buffer[50];

    // Use snprintf for safe, bounded conversion from float to string.
    snprintf(string1_buffer, sizeof(string1_buffer), "%f", f1);
    snprintf(string2_buffer, sizeof(string2_buffer), "%f", f2);
    snprintf(string3_buffer, sizeof(string3_buffer), "%f", f3);

    std::vector<std::string> result;
    result.push_back(string1_buffer);
    result.push_back(string2_buffer);
    result.push_back(string3_buffer);

    return result;
}

int main() {
    // Seed the random number generator once at the start of the program.
    srand(static_cast<unsigned int>(time(0)));

    std::cout << "Running 5 test cases for CPP:" << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::vector<std::string> randomStrings = generateRandomFloatStrings();
        std::cout << "Test Case " << (i + 1) << ": [\"" << randomStrings[0]
                  << "\", \"" << randomStrings[1] << "\", \"" << randomStrings[2]
                  << "\"]" << std::endl;
    }
    return 0;
}