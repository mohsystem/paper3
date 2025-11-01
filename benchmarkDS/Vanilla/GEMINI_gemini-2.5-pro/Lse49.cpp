#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>
#include <sys/stat.h> // For stat
#include <vector>

// Note: This implementation requires the 'zip' and 'unzip' command-line utilities
// to be installed and available in the system's PATH. This is a common approach
// in C/C++ when a third-party library is not desired for simplicity.

/**
 * Unzips an archive using the system's 'unzip' command.
 * @param zip_path The path to the .zip file.
 * @param extract_to_dir The directory to extract the files into.
 */
void unzip(const std::string& zip_path, const std::string& extract_to_dir) {
    // Create the destination directory if it doesn't exist
    std::string mkdir_command = "mkdir -p " + extract_to_dir;
    system(mkdir_command.c_str());

    // Build the unzip command
    // -o: overwrite files without prompting
    // -d: destination directory
    std::string command = "unzip -o \"" + zip_path + "\" -d \"" + extract_to_dir + "\"";
    
    std::cout << "Executing: " << command << std::endl;
    int result = system(command.c_str());
    if (result != 0) {
        std::cerr << "Warning: system command exited with non-zero status: " << result << std::endl;
    }
}

// --- Test Case Helper Functions ---

/**
 * Creates a test zip file using the system's 'zip' command.
 */
void create_test_zip(const std::string& zip_path, int test_num) {
    // Create dummy files to be zipped
    std::string dir_name = "temp_zip_dir_" + std::to_string(test_num);
    system(("mkdir -p " + dir_name + "/subdir").c_str());

    std::string file1_name = dir_name + "/test_file_" + std::to_string(test_num) + ".txt";
    std::string file2_name = dir_name + "/subdir/test_file_" + std::to_string(test_num) + ".txt";

    std::ofstream file1(file1_name);
    file1 << "This is content for test case " << test_num;
    file1.close();
    
    std::ofstream file2(file2_name);
    file2 << "This is content for test case " << test_num;
    file2.close();

    // Create zip archive
    std::string command = "zip -r " + zip_path + " " + dir_name;
    system(command.c_str());

    // Clean up temp files
    std::string rm_command = "rm -rf " + dir_name;
    system(rm_command.c_str());
}

/**
 * Checks if a file exists.
 */
bool file_exists(const std::string& path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

int main() {
    std::string zip_file_path = "archive.zip";
    std::string extract_dir = "/tmp/unpack";

    for (int i = 1; i <= 5; ++i) {
        std::cout << "--- Running Test Case " << i << " ---" << std::endl;
        bool success = true;

        // 1. Setup
        system(("rm -f " + zip_file_path).c_str());
        system(("rm -rf " + extract_dir).c_str());
        create_test_zip(zip_file_path, i);
        std::cout << "Created " << zip_file_path << " for test " << i << std::endl;

        // 2. Execute
        unzip(zip_file_path, extract_dir);

        // 3. Verify
        std::string expected_file1 = extract_dir + "/temp_zip_dir_" + std::to_string(i) + "/test_file_" + std::to_string(i) + ".txt";
        std::string expected_file2 = extract_dir + "/temp_zip_dir_" + std::to_string(i) + "/subdir/test_file_" + std::to_string(i) + ".txt";
        
        if (!file_exists(expected_file1) || !file_exists(expected_file2)) {
            std::cerr << "Verification failed: Extracted files not found." << std::endl;
            success = false;
        } else {
             std::ifstream file_stream(expected_file1);
             std::string content((std::istreambuf_iterator<char>(file_stream)), std::istreambuf_iterator<char>());
             std::string expected_content = "This is content for test case " + std::to_string(i);
             if (content != expected_content) {
                std::cerr << "Verification failed: File content mismatch." << std::endl;
                success = false;
             }
        }

        // 4. Cleanup
        system(("rm -f " + zip_file_path).c_str());
        system(("rm -rf " + extract_dir).c_str());

        std::cout << "Test Case " << i << ": " << (success ? "PASS" : "FAIL") << std::endl;
    }

    return 0;
}