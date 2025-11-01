#include <iostream>
#include <vector>
#include <string>
#include <stdexcept> // For std::invalid_argument, std::out_of_range

/**
 * @brief Allocates a buffer of the specified size and prints its contents.
 * 
 * This function handles string parsing, validation, allocation, and printing.
 * It uses std::vector for safe, automatic memory management.
 * 
 * @param size_str The desired buffer size as a string.
 */
void allocateAndPrint(const std::string& size_str) {
    std::cout << "--- Processing input: \"" << size_str << "\" ---" << std::endl;
    try {
        // 1. Parse the string to a long long to handle larger numbers and negatives
        size_t size = 0;
        long long parsed_size = std::stoll(size_str);

        // 2. Validate the input size
        if (parsed_size < 0) {
            std::cerr << "Error: Buffer size cannot be negative." << std::endl;
        } else {
            size = static_cast<size_t>(parsed_size);
            std::cout << "Allocating buffer of size: " << size << std::endl;

            // 3. Allocate the buffer using std::vector
            // std::vector is zero-initialized by default and handles its own memory.
            // A very large size will throw std::bad_alloc.
            std::vector<char> buffer(size);

            // 4. Print the contents of the buffer
            std::cout << "Buffer contents: [";
            for (size_t i = 0; i < buffer.size(); ++i) {
                // Print as integer values
                std::cout << static_cast<int>(buffer[i]) << (i == buffer.size() - 1 ? "" : ", ");
            }
            std::cout << "]" << std::endl;
        }
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: Invalid number format. Please provide an integer." << std::endl;
    } catch (const std::out_of_range& e) {
        std::cerr << "Error: Number is out of range." << std::endl;
    } catch (const std::bad_alloc& e) {
        std::cerr << "Error: Failed to allocate buffer. Memory limit exceeded." << std::endl;
    }
    std::cout << "--- Finished processing ---" << std::endl << std::endl;
}

/**
 * @brief Main entry point. Processes command-line argument or runs test cases.
 * 
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return int Exit code.
 */
int main(int argc, char* argv[]) {
    // Lse60 is the Java class name, but C++ doesn't have a main class concept.
    // The executable will have the desired name.

    if (argc == 2) {
        // Process the command-line argument if provided
        std::cout << "Processing command-line argument..." << std::endl;
        allocateAndPrint(argv[1]);
    } else {
        // Run built-in test cases if no command-line argument is given
        std::cout << "No command-line argument found. Running test cases..." << std::endl;
        
        // Test Case 1: Valid positive size
        allocateAndPrint("10");

        // Test Case 2: Zero size
        allocateAndPrint("0");

        // Test Case 3: Negative size (invalid)
        allocateAndPrint("-5");

        // Test Case 4: Non-numeric input (invalid)
        allocateAndPrint("hello");

        // Test Case 5: Large size (may cause bad_alloc)
        allocateAndPrint("2000000000"); // 2 GB
    }

    return 0;
}