#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <sstream>

/**
 * Generates 3 random floats, concatenates them into a single string,
 * and writes the string to a specified file.
 *
 * @param filename The name of the file to write to.
 */
void generateAndWriteToFile(const std::string& filename) {
    // 1. Generate 3 random floats using a modern C++ approach
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);

    float f1 = dis(gen);
    float f2 = dis(gen);
    float f3 = dis(gen);

    // 2. Convert to strings and 3. Concatenate using stringstream
    std::stringstream ss;
    ss << f1 << f2 << f3;
    std::string concatenatedString = ss.str();

    // 4. Write the string to a file
    // ofstream's destructor will close the file automatically when it goes
    // out of scope (RAII), preventing resource leaks.
    std::ofstream outFile(filename);

    if (outFile.is_open()) {
        outFile << concatenatedString;
        outFile.close(); // Explicitly closing is good practice
        std::cout << "Successfully wrote to " << filename << std::endl;
    } else {
        std::cerr << "Error: Unable to open file " << filename << " for writing." << std::endl;
    }
}

int main() {
    // Run 5 test cases
    std::cout << "--- C++ Test Cases ---" << std::endl;
    for (int i = 1; i <= 5; ++i) {
        // Use std::to_string for safe integer to string conversion
        std::string filename = "cpp_output_" + std::to_string(i) + ".txt";
        std::cout << "Running test case " << i << "..." << std::endl;
        generateAndWriteToFile(filename);
        std::cout << "--------------------" << std::endl;
    }
    return 0;
}