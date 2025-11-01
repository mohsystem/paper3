#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <system_error>
#include <stdexcept>

// For unique filename generation
#include <random>
#include <sstream>
#include <iomanip>

namespace fs = std::filesystem;

static const long long MAX_FILE_SIZE = 10 * 1024 * 1024; // 10 MB

// A simple Base64 implementation
std::string base64_encode(const std::vector<char>& in) {
    const std::string_view chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
    std::string out;
    out.reserve(((in.size() / 3) + (in.size() % 3 > 0)) * 4);
    int val = 0, valb = -6;
    for (unsigned char c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) {
        out.push_back(chars[((val << 8) >> (valb + 8)) & 0x3F]);
    }
    while (out.size() % 4) {
        out.push_back('=');
    }
    return out;
}

bool upload_image(const std::string& input_base_dir, const std::string& input_filename, const std::string& output_base_dir, const std::string& output_filename) {
    // Rule #5: Validate paths to prevent traversal
    std::error_code ec;
    fs::path input_base_path = fs::weakly_canonical(input_base_dir, ec);
    if (ec) {
        std::cerr << "Error: Invalid input base directory: " << input_base_dir << std::endl;
        return false;
    }
    fs::path output_base_path = fs::weakly_canonical(output_base_dir, ec);
     if (ec) {
        std::cerr << "Error: Invalid output base directory: " << output_base_dir << std::endl;
        return false;
    }

    fs::path input_path = fs::weakly_canonical(input_base_path / input_filename, ec);
    if (ec) {
        std::cerr << "Error: Input file does not exist or path is invalid: " << (input_base_path / input_filename) << std::endl;
        return false;
    }

    fs::path output_path = (output_base_path / output_filename).lexically_normal();
    
    auto check_prefix = [](const fs::path& path, const fs::path& base) {
        auto path_it = path.begin();
        auto base_it = base.begin();
        while (path_it != path.end() && base_it != base.end()) {
            if (*path_it != *base_it) return false;
            ++path_it;
            ++base_it;
        }
        return base_it == base.end();
    };

    if (!check_prefix(input_path, input_base_path)) {
        std::cerr << "Error: Input path is outside the allowed directory." << std::endl;
        return false;
    }
    if (!check_prefix(output_path, output_base_path)) {
        std::cerr << "Error: Output path is outside the allowed directory." << std::endl;
        return false;
    }
    
    // Rule #6: Check for symlinks before opening (acknowledging TOCTOU race condition on non-POSIX systems)
    if (fs::is_symlink(input_path, ec)) {
        std::cerr << "Error: Input path must not be a symbolic link." << std::endl;
        return false;
    }
    if (!fs::is_regular_file(input_path, ec)) {
        std::cerr << "Error: Input path is not a regular file." << std::endl;
        return false;
    }

    // Open file and read content
    std::ifstream file(input_path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open input file: " << input_path << std::endl;
        return false;
    }

    // Rule #3: Check file size after opening
    file.seekg(0, std::ios::end);
    std::streampos size = file.tellg();
    if (size > MAX_FILE_SIZE) {
        std::cerr << "Error: File size exceeds the maximum limit." << std::endl;
        return false;
    }
    if (size == 0) {
        std::cerr << "Error: Input file is empty." << std::endl;
        return false;
    }
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size)) {
        std::cerr << "Error: Failed to read from input file." << std::endl;
        return false;
    }

    // Base64 encode
    std::string encoded_content = base64_encode(buffer);

    // Rule #6: Safe and atomic write
    fs::path temp_path;
    try {
        fs::create_directories(output_path.parent_path(), ec);
        if (ec) {
            throw std::runtime_error("Could not create output directory: " + ec.message());
        }

        // Generate a unique temporary filename
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distrib(0, 0xFFFFFF);
        std::stringstream ss;
        ss << output_filename << "." << std::hex << distrib(gen) << ".tmp";
        temp_path = output_base_path / ss.str();

        std::ofstream temp_file(temp_path, std::ios::binary | std::ios::out);
        if (!temp_file) {
            throw std::runtime_error("Could not create temporary file.");
        }
        temp_file << encoded_content;
        temp_file.close();

        // Atomically rename the file
        fs::rename(temp_path, output_path, ec);
        if (ec) {
             throw std::runtime_error("Could not rename temporary file: " + ec.message());
        }
    } catch (const std::exception& e) {
        std::cerr << "Error writing output file: " << e.what() << std::endl;
        if (!temp_path.empty()) {
            fs::remove(temp_path, ec);
        }
        return false;
    }
    return true;
}

// Setup and run test cases
void run_tests() {
    const std::string input_base = "test_images_cpp";
    const std::string output_base = "test_output_cpp";

    fs::create_directories(input_base);
    fs::create_directories(output_base);

    fs::path valid_image_path = fs::path(input_base) / "valid_image.jpg";
    fs::path large_image_path = fs::path(input_base) / "large_image.jpg";
    fs::path symlink_image_path = fs::path(input_base) / "symlink.jpg";

    {
        std::ofstream(valid_image_path, std::ios::binary).write("\x01\x02\x03\x04\x05", 5);
        std::ofstream large_file(large_image_path, std::ios::binary);
        std::vector<char> large_data(MAX_FILE_SIZE + 1, 0);
        large_file.write(large_data.data(), large_data.size());
    }

    std::error_code ec;
    fs::create_symlink(valid_image_path, symlink_image_path, ec);
    bool symlink_supported = !ec;
    if (!symlink_supported) {
        std::cout << "Skipping symlink test: Not supported on this system." << std::endl;
    }
    
    std::cout << "--- Running C++ Test Cases ---" << std::endl;

    // 1. Valid upload
    std::cout << "1. Testing valid upload... ";
    bool success = upload_image(input_base, "valid_image.jpg", output_base, "encoded.txt");
    std::cout << (success ? "SUCCESS" : "FAILED") << std::endl;

    // 2. File too large
    std::cout << "2. Testing file too large... ";
    success = upload_image(input_base, "large_image.jpg", output_base, "encoded_large.txt");
    std::cout << (!success ? "SUCCESS (Rejected as expected)" : "FAILED") << std::endl;
    
    // 3. Symbolic link
    if (symlink_supported) {
        std::cout << "3. Testing symbolic link... ";
        success = upload_image(input_base, "symlink.jpg", output_base, "encoded_symlink.txt");
        std::cout << (!success ? "SUCCESS (Rejected as expected)" : "FAILED") << std::endl;
    }

    // 4. Path traversal
    std::cout << "4. Testing path traversal... ";
    success = upload_image(input_base, "../test_traversal.jpg", output_base, "encoded_traversal.txt");
    std::cout << (!success ? "SUCCESS (Rejected as expected)" : "FAILED") << std::endl;
    
    // 5. Non-existent file
    std::cout << "5. Testing non-existent file... ";
    success = upload_image(input_base, "not_real.jpg", output_base, "encoded_not_real.txt");
    std::cout << (!success ? "SUCCESS (Rejected as expected)" : "FAILED") << std::endl;

    // Cleanup
    fs::remove_all(input_base);
    fs::remove_all(output_base);
    std::cout << "--- C++ Tests Complete ---" << std::endl;
}

int main() {
    try {
        run_tests();
    } catch (const std::exception& e) {
        std::cerr << "A critical error occurred: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}