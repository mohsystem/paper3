#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

// Helper function to encode binary data to base64
std::string base64_encode(const std::string& in) {
    std::string out;
    const std::string b64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";

    int val = 0, valb = -6;
    for (unsigned char c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(b64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) {
        out.push_back(b64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    }
    while (out.size() % 4) {
        out.push_back('=');
    }
    return out;
}

/**
 * @brief Reads an image file, encodes it to Base64, and writes the encoded string to a text file.
 *
 * @param imagePath The path to the input image file.
 * @param outputPath The path to the output file where the Base64 string will be saved.
 */
void upload_image(const std::string& imagePath, const std::string& outputPath) {
    // Open the input file in binary mode
    std::ifstream file(imagePath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open input file " << imagePath << std::endl;
        return;
    }

    // Read the entire file into a string
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string file_content = buffer.str();
    file.close();

    // Encode the content to Base64
    std::string encoded_data = base64_encode(file_content);

    // Open the output file
    std::ofstream outfile(outputPath);
    if (!outfile.is_open()) {
        std::cerr << "Error: Could not open output file " << outputPath << std::endl;
        return;
    }

    // Write the encoded data to the output file
    outfile << encoded_data;
    outfile.close();
}


// Helper function to create a dummy file for testing
void createTestFile(const std::string& fileName, const std::string& content) {
    std::ofstream file(fileName, std::ios::binary);
    if (file.is_open()) {
        file << content;
        file.close();
    }
}

// Helper function to read a file's content into a string
std::string readTestFile(const std::string& fileName) {
    std::ifstream file(fileName);
    if (file.is_open()) {
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
    return "";
}

int main() {
    // --- Test Cases ---

    std::vector<std::string> test_inputs = {
        "Man",          // Test case 1: Length is a multiple of 3
        "Ma",           // Test case 2: Length % 3 == 2
        "M",            // Test case 3: Length % 3 == 1
        "",             // Test case 4: Empty content
        "Hello, World!" // Test case 5: A longer string
    };

    for (int i = 0; i < test_inputs.size(); ++i) {
        std::string input_filename = "test_input_" + std::to_string(i) + ".bin";
        std::string output_filename = "test_output_" + std::to_string(i) + ".txt";

        std::cout << "--- Running Test Case " << i + 1 << " ---" << std::endl;
        std::cout << "Input data: \"" << test_inputs[i] << "\"" << std::endl;
        
        // 1. Create dummy input file
        createTestFile(input_filename, test_inputs[i]);

        // 2. Call the function
        upload_image(input_filename, output_filename);
        std::cout << "upload_image() called. Encoded data written to " << output_filename << std::endl;

        // 3. (Optional) Verify output
        std::string encoded_content = readTestFile(output_filename);
        std::cout << "Encoded output: " << encoded_content << std::endl;
        std::cout << "Test Case " << i + 1 << " completed.\n" << std::endl;

        // 4. Clean up files
        remove(input_filename.c_str());
        remove(output_filename.c_str());
    }

    return 0;
}