#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <sstream>

/**
 * Generates 3 random floats, concatenates them into a string,
 * and appends the string to the specified file.
 * @param filename The name of the file to write to.
 */
void generateAndWriteRandomFloats(const std::string& filename) {
    // Static ensures the random number generator is seeded only once.
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> dis(0.0f, 1.0f);

    // 1. Generate 3 random floats
    float f1 = dis(gen);
    float f2 = dis(gen);
    float f3 = dis(gen);

    // 2. Convert to strings and 3. Concatenate using a stringstream
    std::stringstream ss;
    ss << f1 << f2 << f3 << "\n";
    std::string concatenatedString = ss.str();

    // 4. Writes the string to a file (in append mode)
    std::ofstream outFile(filename, std::ios_base::app);
    if (outFile.is_open()) {
        outFile << concatenatedString;
    } else {
        std::cerr << "Error: Unable to open file " << filename << std::endl;
    }
}

int main() {
    std::string filename = "output.cpp.txt";

    // Clear the file at the beginning of the run for consistent output
    std::ofstream ofs(filename, std::ofstream::trunc);
    ofs.close();

    // Run 5 test cases
    std::cout << "Running 5 C++ test cases. Writing to " << filename << std::endl;
    for (int i = 0; i < 5; ++i) {
        generateAndWriteRandomFloats(filename);
    }
    std::cout << "C++ tests finished." << std::endl;

    return 0;
}