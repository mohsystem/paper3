#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <cstdio>

/**
 * Generates three random floats and converts them to strings using snprintf.
 * @return A vector of three strings, each representing a random float.
 */
std::vector<std::string> generateRandomFloatStrings() {
    // Generate three random floats between 0.0 and 1.0
    float f1 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    float f2 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    float f3 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

    // Buffers to hold the string representations of the floats
    char buffer1[50];
    char buffer2[50];
    char buffer3[50];

    // Convert floats to strings using snprintf (a safer version of sprintf)
    snprintf(buffer1, sizeof(buffer1), "%f", f1);
    snprintf(buffer2, sizeof(buffer2), "%f", f2);
    snprintf(buffer3, sizeof(buffer3), "%f", f3);

    std::vector<std::string> result;
    result.push_back(std::string(buffer1));
    result.push_back(std::string(buffer2));
    result.push_back(std::string(buffer3));
    
    return result;
}

int main() {
    // Seed the random number generator once
    srand(static_cast<unsigned int>(time(NULL)));

    std::cout << "CPP Test Cases:" << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::vector<std::string> randomStrings = generateRandomFloatStrings();
        std::cout << "Test Case " << i + 1 << ": [";
        for (size_t j = 0; j < randomStrings.size(); ++j) {
            std::cout << "\"" << randomStrings[j] << "\"" << (j == randomStrings.size() - 1 ? "" : ", ");
        }
        std::cout << "]" << std::endl;
    }

    return 0;
}