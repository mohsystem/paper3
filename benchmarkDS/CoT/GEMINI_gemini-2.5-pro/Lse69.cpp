#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iterator>
#include <algorithm>
#include <cstdlib> // For system()

// --- Start of Base64 encoding utility ---
// This implementation is a common, public-domain one.
// It's included here to make the file self-contained.
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

            for (i = 0; (i < 4); i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];

        while ((i++ < 3))
            ret += '=';
    }

    return ret;
}
// --- End of Base64 encoding utility ---


/**
 * Reads an image file, encodes it to Base64, and writes the result to an output file.
 *
 * @param imagePath The path to the input image file.
 * @param outputPath The path to the output file for the Base64 string.
 * @return The path to the output file on success, an empty string on failure.
 */
std::string upload_image(const std::string& imagePath, const std::string& outputPath) {
    // Security: Basic path traversal check. A robust solution would involve
    // canonicalizing the path and validating against a safe base directory.
    if (imagePath.find("..") != std::string::npos || outputPath.find("..") != std::string::npos) {
        std::cerr << "Error: Path traversal attempt detected." << std::endl;
        return "";
    }

    // RAII for ifstream ensures the file is closed even if exceptions are thrown.
    std::ifstream inFile(imagePath, std::ios::binary | std::ios::ate);
    if (!inFile.is_open()) {
        std::cerr << "Error: Could not open input file: " << imagePath << std::endl;
        return "";
    }

    // Read the file content into a vector. This approach is not suitable for very large files,
    // as it could lead to resource exhaustion. A streaming approach is safer.
    std::streamsize size = inFile.tellg();
    inFile.seekg(0, std::ios::beg);
    std::vector<char> buffer(size);
    if (size > 0 && !inFile.read(buffer.data(), size)) {
        std::cerr << "Error: Could not read from input file: " << imagePath << std::endl;
        return "";
    }
    
    std::string encoded_data = base64_encode(buffer);

    std::ofstream outFile(outputPath);
    if (!outFile.is_open()) {
        size_t last_slash = outputPath.find_last_of("/\\");
        if (last_slash != std::string::npos) {
             std::string dir_path = outputPath.substr(0, last_slash);
             // Security: Using system() can be risky. Here it is used for test setup convenience.
             // In production, use platform-specific APIs or C++17 <filesystem>.
             #if defined(_WIN32)
                 std::string dir_cmd = "mkdir \"" + dir_path + "\"";
             #else
                 std::string dir_cmd = "mkdir -p \"" + dir_path + "\"";
             #endif
             system(dir_cmd.c_str());
             
             outFile.open(outputPath);
             if(!outFile.is_open()){
                 std::cerr << "Error: Could not open output file after creating directory: " << outputPath << std::endl;
                 return "";
             }
        } else {
            std::cerr << "Error: Could not open output file: " << outputPath << std::endl;
            return "";
        }
    }

    outFile << encoded_data;
    if (!outFile.good()) {
        std::cerr << "Error: Failed to write to output file: " << outputPath << std::endl;
        outFile.close();
        return "";
    }
    
    return outputPath;
}


// --- Main for Test Cases ---
void create_dummy_file(const std::string& path, const std::string& content) {
    std::ofstream file(path, std::ios::binary);
    if (file.is_open()) {
        file.write(content.c_str(), content.length());
    }
}

// Lse69 is the conceptual name from the prompt, not used in C++ main.
int main() {
    std::string temp_dir = "temp_test_cpp";
    #if defined(_WIN32)
        system("if exist temp_test_cpp ( rmdir /s /q temp_test_cpp )");
        system("mkdir temp_test_cpp");
    #else
        system("rm -rf temp_test_cpp && mkdir -p temp_test_cpp");
    #endif

    std::string dummy_image_path = temp_dir + "/test_image.jpg";
    std::string empty_image_path = temp_dir + "/empty.jpg";

    create_dummy_file(dummy_image_path, std::string("\xff\xd8\xff\xe0\x01\x02\x03\x04", 8));
    create_dummy_file(empty_image_path, "");

    std::cout << "--- C++ Test Cases ---" << std::endl;

    // Test Case 1: Successful upload
    std::cout << "Test Case 1: Normal valid image path" << std::endl;
    std::string output_path1 = temp_dir + "/output1.txt";
    std::string result1 = upload_image(dummy_image_path, output_path1);
    std::cout << (!result1.empty() ? "Success, output at: " + result1 : "Failed") << std::endl;
    if(!result1.empty()){
        std::ifstream res_file(result1);
        std::string content((std::istreambuf_iterator<char>(res_file)), std::istreambuf_iterator<char>());
        std::cout << "Content: " << content << std::endl;
    }
    std::cout << std::endl;

    // Test Case 2: Non-existent image path
    std::cout << "Test Case 2: Non-existent image path" << std::endl;
    std::string output_path2 = temp_dir + "/output2.txt";
    std::string result2 = upload_image("non_existent_image.jpg", output_path2);
    std::cout << (!result2.empty() ? "Success, output at: " + result2 : "Failed") << std::endl;
    std::cout << std::endl;

    // Test Case 3: Output to non-existent directory
    std::cout << "Test Case 3: Output to non-existent directory" << std::endl;
    std::string output_path3 = temp_dir + "/new_dir/output3.txt";
    std::string result3 = upload_image(dummy_image_path, output_path3);
    std::cout << (!result3.empty() ? "Success, output at: " + result3 : "Failed") << std::endl;
    std::cout << std::endl;

    // Test Case 4: Path traversal attempt
    std::cout << "Test Case 4: Path traversal attempt" << std::endl;
    std::string result4 = upload_image(dummy_image_path, "../output4.txt");
    std::cout << (!result4.empty() ? "Success, output at: " + result4 : "Failed") << std::endl;
    std::cout << std::endl;

    // Test Case 5: Empty input file
    std::cout << "Test Case 5: Empty input file" << std::endl;
    std::string output_path5 = temp_dir + "/output5.txt";
    std::string result5 = upload_image(empty_image_path, output_path5);
    std::cout << (!result5.empty() ? "Success, output at: " + result5 : "Failed") << std::endl;
    if(!result5.empty()){
        std::ifstream res_file(result5);
        std::string content((std::istreambuf_iterator<char>(res_file)), std::istreambuf_iterator<char>());
        std::cout << "Content: " << content << std::endl;
    }
    std::cout << std::endl;

    return 0;
}