
#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <sstream>

std::string generateAndConcatenateFloats() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);
    
    float float1 = dis(gen);
    float float2 = dis(gen);
    float float3 = dis(gen);
    
    std::ostringstream oss;
    oss << float1 << float2 << float3;
    return oss.str();
}

void writeToFile(const std::string& content, const std::string& filename) {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << content;
        file.close();
    } else {
        std::cerr << "Error writing to file: " << filename << std::endl;
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
