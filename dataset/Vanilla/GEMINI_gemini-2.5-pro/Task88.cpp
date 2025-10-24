#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <random>
#include <chrono>
#include <filesystem> // Requires C++17

// Note: This code requires a C++17 compliant compiler (e.g., GCC 8+, Clang 7+, MSVC 2017+)
// and may need to be linked with the filesystem library (e.g., -lstdc++fs).

class Task88 {
public:
    /**
     * Creates a temporary file, writes a random UTF-8 encoded string to it,
     * and returns the file path.
     * @return std::string containing the path, or an empty string on failure.
     */
    static std::string createTempFileWithRandomUnicodeString() {
        try {
            // 1. Get the path to the temporary directory and create a unique filename.
            std::filesystem::path temp_dir = std::filesystem::temp_directory_path();
            uint64_t timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()).count();
            std::string filename = "temp_unicode_" + std::to_string(timestamp) + ".txt";
            std::filesystem::path file_path = temp_dir / filename;

            // 2. Create a random string.
            // Using a vector of UTF-8 strings to correctly handle multi-byte characters.
            std::vector<std::string> chars = {
                "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", 
                "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z",
                "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M",
                "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
                "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
                "à", "á", "ç", "è", "é", "ê", "ë", "Α", "Β", "Γ", "Δ", "Ε",
                "€", "£", "¥", "©", "®", "™"
            };
            
            std::string random_string;
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> distrib(0, chars.size() - 1);
            
            for (int i = 0; i < 100; ++i) {
                random_string += chars[distrib(gen)];
            }

            // 4. Write the UTF-8 string to the file in binary mode.
            std::ofstream outfile(file_path, std::ios::binary);
            if (!outfile.is_open()) {
                return "";
            }
            outfile << random_string;
            outfile.close();

            // 5. Return the path of the temporary file.
            return file_path.string();

        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Filesystem error: " << e.what() << std::endl;
            return "";
        }
    }
};

int main() {
    for (int i = 1; i <= 5; ++i) {
        std::cout << "Test Case " << i << ":" << std::endl;
        std::string file_path_str = Task88::createTempFileWithRandomUnicodeString();
        if (!file_path_str.empty()) {
            std::cout << "  Temporary file created at: " << file_path_str << std::endl;
            
            // Optional: Read back to verify.
            std::ifstream infile(file_path_str, std::ios::binary);
            if(infile) {
                std::string content((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());
                std::cout << "  File content byte length: " << content.length() << std::endl;
                infile.close();
            }

            // Cleanup the temporary file.
            try {
                if (std::filesystem::remove(file_path_str)) {
                    std::cout << "  Temporary file deleted." << std::endl;
                } else {
                    std::cout << "  Could not delete temporary file." << std::endl;
                }
            } catch (const std::filesystem::filesystem_error& e) {
                std::cerr << "  Error deleting file: " << e.what() << std::endl;
            }
        } else {
            std::cout << "  Failed to create temporary file." << std::endl;
        }
        std::cout << std::endl;
    }
    return 0;
}