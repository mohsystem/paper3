
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <cstring>

void generateRandomFloatStrings() {
    // Buffer size: sign(1) + digits(38) + decimal(1) + precision(6) + null(1) = 50 is safe
    const int BUFFER_SIZE = 50;
    char string1[BUFFER_SIZE];
    char string2[BUFFER_SIZE];
    char string3[BUFFER_SIZE];
    
    // Generate three random floats (0.0 to 1.0)
    float float1 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    float float2 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    float float3 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    
    // Convert to strings using snprintf (secure version of sprintf)
    snprintf(string1, BUFFER_SIZE, "%f", float1);
    snprintf(string2, BUFFER_SIZE, "%f", float2);
    snprintf(string3, BUFFER_SIZE, "%f", float3);
    
    std::cout << "Float 1: " << string1 << std::endl;
    std::cout << "Float 2: " << string2 << std::endl;
    std::cout << "Float 3: " << string3 << std::endl;
}

int main() {
    // Seed the random number generator
    srand(static_cast<unsigned int>(time(NULL)));
    
    std::cout << "Test Case 1:" << std::endl;
    generateRandomFloatStrings();
    
    std::cout << "\\nTest Case 2:" << std::endl;
    generateRandomFloatStrings();
    
    std::cout << "\\nTest Case 3:" << std::endl;
    generateRandomFloatStrings();
    
    std::cout << "\\nTest Case 4:" << std::endl;
    generateRandomFloatStrings();
    
    std::cout << "\\nTest Case 5:" << std::endl;
    generateRandomFloatStrings();
    
    return 0;
}
