#include <iostream>
#include <fstream>
#include <random>
#include <string>
#include <stdexcept>

std::string generate_and_write(const std::string& filePath) {
    if (filePath.empty()) {
        throw std::invalid_argument("filePath cannot be empty");
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    float f1 = dist(gen);
    float f2 = dist(gen);
    float f3 = dist(gen);

    std::string concatenated = std::to_string(f1) + "," + std::to_string(f2) + "," + std::to_string(f3);

    std::ofstream ofs(filePath, std::ios::out | std::ios::trunc);
    if (!ofs.is_open()) {
        throw std::runtime_error("Failed to open file: " + filePath);
    }
    ofs << concatenated;
    if (!ofs) {
        throw std::runtime_error("Failed to write to file: " + filePath);
    }
    ofs.close();

    return concatenated;
}

int main() {
    const char* files[5] = {
        "cpp_out_1.txt",
        "cpp_out_2.txt",
        "cpp_out_3.txt",
        "cpp_out_4.txt",
        "cpp_out_5.txt"
    };

    for (const char* f : files) {
        try {
            std::string result = generate_and_write(f);
            std::cout << "Wrote to " << f << ": " << result << std::endl;
        } catch (const std::exception& ex) {
            std::cerr << "Error for " << f << ": " << ex.what() << std::endl;
        }
    }

    return 0;
}