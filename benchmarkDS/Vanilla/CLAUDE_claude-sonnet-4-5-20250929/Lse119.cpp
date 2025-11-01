
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <string>

struct FloatStrings {
    std::string string1;
    std::string string2;
    std::string string3;
};

FloatStrings generateRandomFloatStrings() {
    FloatStrings result;
    char buffer[50];
    
    // Generate three random floats
    float float1 = (float)rand() / RAND_MAX;
    float float2 = (float)rand() / RAND_MAX;
    float float3 = (float)rand() / RAND_MAX;
    
    // Convert to strings using sprintf
    sprintf(buffer, "%f", float1);
    result.string1 = buffer;
    
    sprintf(buffer, "%f", float2);
    result.string2 = buffer;
    
    sprintf(buffer, "%f", float3);
    result.string3 = buffer;
    
    return result;
}

int main() {
    srand(time(NULL));
    
    // Test case 1
    FloatStrings result1 = generateRandomFloatStrings();
    std::cout << "Test 1: " << result1.string1 << ", " << result1.string2 << ", " << result1.string3 << std::endl;
    
    // Test case 2
    FloatStrings result2 = generateRandomFloatStrings();
    std::cout << "Test 2: " << result2.string1 << ", " << result2.string2 << ", " << result2.string3 << std::endl;
    
    // Test case 3
    FloatStrings result3 = generateRandomFloatStrings();
    std::cout << "Test 3: " << result3.string1 << ", " << result3.string2 << ", " << result3.string3 << std::endl;
    
    // Test case 4
    FloatStrings result4 = generateRandomFloatStrings();
    std::cout << "Test 4: " << result4.string1 << ", " << result4.string2 << ", " << result4.string3 << std::endl;
    
    // Test case 5
    FloatStrings result5 = generateRandomFloatStrings();
    std::cout << "Test 5: " << result5.string1 << ", " << result5.string2 << ", " << result5.string3 << std::endl;
    
    return 0;
}
