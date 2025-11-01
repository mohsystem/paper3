#include <iostream>
#include <fstream>
#include <string>
#include <random>
#include <sstream>

/**
 * @brief Generates 3 random floats, concatenates them into a space-separated
 * string, and writes the string to a specified file.
 *
 * This function uses C++11's <random> library for high-quality random numbers.
 * std::random_device is used as a source of non-deterministic randomness to seed
 * the Mersenne Twister engine.
 *
 * @param filename The name of the file to write to. This should not come from
 *                 untrusted input to prevent path traversal attacks.
 * @return true on success, false on failure.
 */
bool generateAndWriteRandoms(const std::string& filename) {
    // 1. Generate 3 random floats using a modern C++ approach.
    std::random_device rd;  // Non-deterministic random number generator (if available)
    std::mt19937 gen(rd()); // Seed the Mersenne Twister engine
    std::uniform_real_distribution<float> dis(0.0f, 1.0f); // Distribution for floats in [0.0, 1.0]

    // 2. Convert them to strings and concatenate.
    // std::ostringstream is an efficient and safe way to build strings, preventing buffer overflows.
    std::ostringstream oss;
    oss << dis(gen) << " " << dis(gen) << " " << dis(gen);
    std::string concatenatedString = oss.str();

    // 3. Write the string to a file.
    // std::ofstream automatically handles closing the file when it goes out of scope (RAII).
    std::ofstream outFile(filename);

    // Check if the file was opened successfully. This is a critical security check.
    if (!outFile.is_open()) {
        std::cerr << "Error: Could not open file '" << filename << "' for writing." << std::endl;
        return false;
    }

    outFile << concatenatedString;

    // Check for write errors.
    if (outFile.fail()) {
        std::cerr << "Error: Failed to write to file '" << filename << "'." << std::endl;
        // The file is automatically closed by the ofstream destructor even on failure.
        return false;
    }
    
    return true;
}

int main() {
    std::cout << "Running C++ test cases..." << std::endl;
    for (int i = 1; i <= 5; ++i) {
        std::string testFilename = "output_cpp_" + std::to_string(i) + ".txt";
        std::cout << "Test Case " << i << ": Writing to " << testFilename << std::endl;
        bool success = generateAndWriteRandoms(testFilename);
        if (success) {
            std::cout << " -> Success." << std::endl;
        } else {
            std::cout << " -> Failed." << std::endl;
        }
    }
    return 0;
}