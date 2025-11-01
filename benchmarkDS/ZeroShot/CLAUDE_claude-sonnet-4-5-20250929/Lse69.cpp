
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

std::string base64_encode(const std::vector<unsigned char>& data) {
    const std::string base64_chars = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
    
    std::string encoded;
    int val = 0;
    int valb = -6;
    
    for (unsigned char c : data) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            encoded.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    
    if (valb > -6) {
        encoded.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    }
    
    while (encoded.size() % 4) {
        encoded.push_back('=');
    }
    
    return encoded;
}

std::string upload_image(const std::string& input_image_path, const std::string& output_file_path) {
    try {
        // Read the image file
        std::ifstream image_file(input_image_path, std::ios::binary);
        if (!image_file) {
            return "Error: Cannot open input file";
        }
        
        std::vector<unsigned char> image_data((std::istreambuf_iterator<char>(image_file)),
                                               std::istreambuf_iterator<char>());
        image_file.close();
        
        // Encode the image using base64
        std::string encoded_image = base64_encode(image_data);
        
        // Write the encoded image to a file
        std::ofstream output_file(output_file_path);
        if (!output_file) {
            return "Error: Cannot open output file";
        }
        
        output_file << encoded_image;
        output_file.close();
        
        return "Image uploaded and encoded successfully";
    } catch (const std::exception& e) {
        return std::string("Error: ") + e.what();
    }
}

int main() {
    // Test case 1
    std::cout << upload_image("test1.jpg", "encoded1.txt") << std::endl;
    
    // Test case 2
    std::cout << upload_image("test2.png", "encoded2.txt") << std::endl;
    
    // Test case 3
    std::cout << upload_image("test3.gif", "encoded3.txt") << std::endl;
    
    // Test case 4
    std::cout << upload_image("test4.bmp", "encoded4.txt") << std::endl;
    
    // Test case 5
    std::cout << upload_image("test5.jpeg", "encoded5.txt") << std::endl;
    
    return 0;
}
