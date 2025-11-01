
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstdio>
#include <string>

class Lse119 {
public:
    static void generateRandomFloatStrings(std::string& string1, std::string& string2, std::string& string3) {
        float float1 = static_cast<float>(rand()) / RAND_MAX;
        float float2 = static_cast<float>(rand()) / RAND_MAX;
        float float3 = static_cast<float>(rand()) / RAND_MAX;
        
        char buffer[50];
        
        sprintf(buffer, "%f", float1);
        string1 = buffer;
        
        sprintf(buffer, "%f", float2);
        string2 = buffer;
        
        sprintf(buffer, "%f", float3);
        string3 = buffer;
    }
};

int main() {
    srand(static_cast<unsigned int>(time(nullptr)));
    
    std::string string1, string2, string3;
    
    // Test case 1
    std::cout << "Test case 1:" << std::endl;
    Lse119::generateRandomFloatStrings(string1, string2, string3);
    std::cout << "String 1: " << string1 << std::endl;
    std::cout << "String 2: " << string2 << std::endl;
    std::cout << "String 3: " << string3 << std::endl;
    std::cout << std::endl;
    
    // Test case 2
    std::cout << "Test case 2:" << std::endl;
    Lse119::generateRandomFloatStrings(string1, string2, string3);
    std::cout << "String 1: " << string1 << std::endl;
    std::cout << "String 2: " << string2 << std::endl;
    std::cout << "String 3: " << string3 << std::endl;
    std::cout << std::endl;
    
    // Test case 3
    std::cout << "Test case 3:" << std::endl;
    Lse119::generateRandomFloatStrings(string1, string2, string3);
    std::cout << "String 1: " << string1 << std::endl;
    std::cout << "String 2: " << string2 << std::endl;
    std::cout << "String 3: " << string3 << std::endl;
    std::cout << std::endl;
    
    // Test case 4
    std::cout << "Test case 4:" << std::endl;
    Lse119::generateRandomFloatStrings(string1, string2, string3);
    std::cout << "String 1: " << string1 << std::endl;
    std::cout << "String 2: " << string2 << std::endl;
    std::cout << "String 3: " << string3 << std::endl;
    std::cout << std::endl;
    
    // Test case 5
    std::cout << "Test case 5:" << std::endl;
    Lse119::generateRandomFloatStrings(string1, string2, string3);
    std::cout << "String 1: " << string1 << std::endl;
    std::cout << "String 2: " << string2 << std::endl;
    std::cout << "String 3: " << string3 << std::endl;
    
    return 0;
}
