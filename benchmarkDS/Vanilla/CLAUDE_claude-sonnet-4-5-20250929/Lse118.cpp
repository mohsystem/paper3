
#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <sstream>

void writeRandomFloatsToFile(const std::string& filename) {
    // Random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    
    // Generate 3 random floats
    float float1 = dis(gen);
    float float2 = dis(gen);
    float float3 = dis(gen);
    
    // Convert them to strings
    std::ostringstream oss1, oss2, oss3;
    oss1 << float1;
    oss2 << float2;
    oss3 << float3;
    
    std::string str1 = oss1.str();
    std::string str2 = oss2.str();
    std::string str3 = oss3.str();
    
    // Concatenate them into a single string
    std::string concatenated = str1 + str2 + str3;
    
    // Write the string to a file
    std::ofstream file(filename);
    if (file.is_open()) {
        file << concatenated;
        file.close();
    } else {
        std::cerr << "Unable to open file: " << filename << std::endl;
    }
}

int main() {
    // Test case 1
    writeRandomFloatsToFile("output1.txt");
    
    // Test case 2
    writeRandomFloatsToFile("output2.txt");
    
    // Test case 3
    writeRandomFloatsToFile("output3.txt");
    
    // Test case 4
    writeRandomFloatsToFile("output4.txt");
    
    // Test case 5
    writeRandomFloatsToFile("output5.txt");
    
    std::cout << "Files created successfully!" << std::endl;
    
    return 0;
}
