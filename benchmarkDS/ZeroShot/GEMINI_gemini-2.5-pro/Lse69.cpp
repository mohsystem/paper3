#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem> // C++17 for path manipulation

// --- Start of Base64 utility ---
// This Base64 implementation is a simplified version based on public domain examples.
static const std::string base64_chars =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

std::string base64_encode(const std::vector<char>& buf) {
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    size_t in_len = buf.size();
    const char* bytes_to_encode = buf.data();

    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
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
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];

        while((i++ < 3))
            ret += '=';
    }
    return ret;
}
// --- End of Base64 utility ---

/**
 * Reads an image file, encodes it to Base64, and saves the encoded string to a new file.
 * This function is secured against path traversal attacks.
 *
 * @param imagePath The path to the image file to upload.
 * @return true if the operation was successful, false otherwise.
 */
bool upload_image(const std::string& imagePath) {
    // Define a safe base directory for uploads.
    const std::filesystem::path upload_dir = std::filesystem::absolute("uploads");
    std::error_code ec;

    // Create the upload directory if it doesn't exist for the check to work.
    if (!std::filesystem::exists(upload_dir, ec)) {
        std::filesystem::create_directories(upload_dir, ec);
        if (ec) {
            std::cerr << "Error: Could not create upload directory: " << ec.message() << std::endl;
            return false;
        }
    }

    // --- Security Check: Path validation ---
    std::filesystem::path input_path(imagePath);
    // canonical() resolves '..' and symlinks to get the real path.
    std::filesystem::path canonical_path = std::filesystem::weakly_canonical(input_path, ec);

    if (ec) {
        std::cerr << "Error resolving path: " << ec.message() << std::endl;
        return false;
    }
    
    // Check if the resolved path is inside our designated upload_dir.
    std::string canon_str = canonical_path.string();
    std::string upload_dir_str = upload_dir.string();

    if (canon_str.rfind(upload_dir_str, 0) != 0) {
        std::cerr << "Error: Path traversal attempt detected. Access denied for: " << imagePath << std::endl;
        return false;
    }

    // --- Sanity Checks ---
    if (!std::filesystem::exists(canonical_path, ec) || !std::filesystem::is_regular_file(canonical_path, ec)) {
        std::cerr << "Error: The specified path is not a regular file: " << canonical_path << std::endl;
        return false;
    }

    // Open and read the image file
    std::ifstream file(canonical_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file: " << canonical_path << std::endl;
        return false;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size)) {
        std::cerr << "Error: Could not read file: " << canonical_path << std::endl;
        return false;
    }
    file.close();

    // Encode the data to Base64
    std::string encoded_data = base64_encode(buffer);

    // Define the output file path
    std::filesystem::path output_path = upload_dir / (canonical_path.filename().string() + ".b64");

    // Write the encoded data to the new file
    std::ofstream out_file(output_path);
    if (!out_file.is_open()) {
        std::cerr << "Error: Could not create output file: " << output_path << std::endl;
        return false;
    }
    out_file << encoded_data;
    out_file.close();

    std::cout << "Image successfully encoded and saved to: " << output_path << std::endl;
    return true;
}

void setup_test_environment() {
    std::cout << "--- Setting up test environment ---" << std::endl;
    try {
        std::filesystem::create_directories("uploads");
        std::ofstream("uploads/test_image.jpg") << "dummy image data";
        std::ofstream("sensitive.txt") << "secret data";
        std::cout << "Test environment created." << std::endl;
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Failed to set up test environment: " << e.what() << std::endl;
    }
    std::cout << "------------------------------------" << std::endl;
}

int main() {
    setup_test_environment();

    std::cout << "\n--- Running Test Cases ---" << std::endl;
    
    // Test Case 1: Valid image file
    std::cout << "\n[Test 1] Processing a valid image file..." << std::endl;
    upload_image("uploads/test_image.jpg");

    // Test Case 2: File does not exist
    std::cout << "\n[Test 2] Processing a non-existent file..." << std::endl;
    upload_image("uploads/not_found.jpg");

    // Test Case 3: Path traversal attempt
    std::cout << "\n[Test 3] Processing with a path traversal attempt..." << std::endl;
    upload_image("uploads/../sensitive.txt");
    
    // Test Case 4: Accessing a file outside the allowed directory
    std::cout << "\n[Test 4] Processing a file outside the upload directory..." << std::endl;
    upload_image("sensitive.txt");

    // Test Case 5: Path is a directory, not a file
    std::cout << "\n[Test 5] Processing a directory path..." << std::endl;
    upload_image("uploads");
    
    std::cout << "\n--- Test Cases Finished ---" << std::endl;
    
    return 0;
}