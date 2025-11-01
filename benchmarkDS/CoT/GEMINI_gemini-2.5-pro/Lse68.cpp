#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <filesystem> // C++17 for path manipulation

// --- Base64 encoding utility ---
// This implementation is self-contained to adhere to the single-file requirement.
// It is a common and widely-used implementation style.
static const std::string base64_chars =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

std::string base64_encode(const std::vector<unsigned char>& buf) {
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    for (unsigned char c : buf) {
        char_array_3[i++] = c;
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for(i = 0; (i <4) ; i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for(j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];

        while((i++ < 3))
            ret += '=';
    }

    return ret;
}
// --- End Base64 utility ---

// Simulates a database table using std::map
static std::map<std::string, std::string> imageDatabase;

/**
 * Reads an image file, converts it to a Base64 string, and inserts it into a simulated database.
 * @param filePath The path to the image file.
 */
void upload_image(const std::string& filePath) {
    // Security: Prevent path traversal by using std::filesystem to get only the filename.
    std::filesystem::path p(filePath);
    std::string fileName = p.filename().string();
    
    // Open the file in binary mode. The ifstream object will automatically close the file (RAII).
    std::ifstream imageFile(filePath, std::ios::binary | std::ios::ate);

    if (!imageFile.is_open()) {
        std::cerr << "Error: Could not open file: " << filePath << std::endl;
        return;
    }

    // Get the file size and read it into a buffer (vector).
    std::streamsize size = imageFile.tellg();
    imageFile.seekg(0, std::ios::beg);

    std::vector<unsigned char> buffer(size);
    if (!imageFile.read(reinterpret_cast<char*>(buffer.data()), size)) {
        std::cerr << "Error: Could not read file content from: " << filePath << std::endl;
        return;
    }

    // Encode the buffer to a Base64 string.
    std::string encodedString = base64_encode(buffer);

    // Insert into the simulated database.
    imageDatabase[fileName] = encodedString;
    std::cout << "Successfully uploaded and encoded '" << fileName << "'." << std::endl;
}


int main() {
    // --- Test Case Setup ---
    try {
        std::ofstream("test1.txt") << "Hello World";
        std::ofstream("test2.jpg", std::ios::binary) << "\xFF\xD8\xFF\xE0";
        std::ofstream("test3.txt").close(); // Creates an empty file
    } catch (const std::exception& e) {
        std::cerr << "Failed to create test files: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "--- Running Test Cases ---" << std::endl;
    // Test Case 1: A valid text file.
    upload_image("test1.txt");

    // Test Case 2: A valid (simulated) image file.
    upload_image("test2.jpg");

    // Test Case 3: An empty file.
    upload_image("test3.txt");
    
    // Test Case 4: A non-existent file.
    upload_image("nonexistent.png");

    // Test Case 5: A path traversal attempt.
    // std::filesystem::path correctly extracts "test1.txt".
    upload_image("../test1.txt");
    
    std::cout << "\n--- Simulated Database Content ---" << std::endl;
    for (const auto& pair : imageDatabase) {
        std::string dataPreview = pair.second.substr(0, 30);
        if (pair.second.length() > 30) {
            dataPreview += "...";
        }
        std::cout << "FileName: " << pair.first << ", Base64: " << dataPreview << std::endl;
    }
    std::cout << "--------------------------------\n" << std::endl;

    // --- Test Case Cleanup ---
    try {
        std::filesystem::remove("test1.txt");
        std::filesystem::remove("test2.jpg");
        std::filesystem::remove("test3.txt");
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Failed to clean up test files: " << e.what() << std::endl;
    }

    return 0;
}