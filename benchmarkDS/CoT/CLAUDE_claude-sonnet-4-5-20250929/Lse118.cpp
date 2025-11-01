
#include <iostream>
#include <fstream>
#include <random>
#include <string>

std::string generateAndConcatenateFloats() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 100.0f);
    
    float float1 = dis(gen);
    float float2 = dis(gen);
    float float3 = dis(gen);
    
    std::string str1 = std::to_string(float1);
    std::string str2 = std::to_string(float2);
    std::string str3 = std::to_string(float3);
    
    return str1 + str2 + str3;
}

void writeToFile(const std::string& content, const std::string& filename) {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << content;
        file.close();
    } else {
        std::cerr << "Error opening file: " << filename << std::endl;
    }
}

int main() {
    // Test case 1
    std::string result1 = generateAndConcatenateFloats();
    writeToFile(result1, "output1.txt");
    std::cout << "Test 1: " << result1 << std::endl;
    
    // Test case 2
    std::string result2 = generateAndConcatenateFloats();
    writeToFile(result2, "output2.txt");
    std::cout << "Test 2: " << result2 << std::endl;
    
    // Test case 3
    std::string result3 = generateAndConcatenateFloats();
    writeToFile(result3, "output3.txt");
    std::cout << "Test 3: " << result3 << std::endl;
    
    // Test case 4
    std::string result4 = generateAndConcatenateFloats();
    writeToFile(result4, "output4.txt");
    std::cout << "Test 4: " << result4 << std::endl;
    
    // Test case 5
    std::string result5 = generateAndConcatenateFloats();
    writeToFile(result5, "output5.txt");
    std::cout << "Test 5: " << result5 << std::endl;
    
    return 0;
}
