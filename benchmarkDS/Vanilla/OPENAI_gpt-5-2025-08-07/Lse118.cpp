#include <iostream>
#include <fstream>
#include <random>
#include <string>

std::string generateAndWrite(const std::string& filePath, unsigned int seed) {
    std::mt19937 gen(seed);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    float f1 = dist(gen);
    float f2 = dist(gen);
    float f3 = dist(gen);
    std::string concatenated = std::to_string(f1) + "," + std::to_string(f2) + "," + std::to_string(f3);
    std::ofstream ofs(filePath);
    if (!ofs) throw std::runtime_error("Failed to open file: " + filePath);
    ofs << concatenated;
    return concatenated;
}

int main() {
    try {
        std::cout << generateAndWrite("out_cpp_1.txt", 1) << std::endl;
        std::cout << generateAndWrite("out_cpp_2.txt", 2) << std::endl;
        std::cout << generateAndWrite("out_cpp_3.txt", 3) << std::endl;
        std::cout << generateAndWrite("out_cpp_4.txt", 4) << std::endl;
        std::cout << generateAndWrite("out_cpp_5.txt", 5) << std::endl;
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
    return 0;
}