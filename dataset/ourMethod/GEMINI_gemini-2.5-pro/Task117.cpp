#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <zip.h>

// To compile: g++ -std=c++17 Task117.cpp -o Task117 -lzip
// You need to have libzip installed (e.g., `sudo apt-get install libzip-dev` on Debian/Ubuntu).

/*
Helper script to create test zip files using Python (run this once):
---------------------------------------------------------------------
import zipfile, os
# For normal test
if not os.path.exists('cpp_test_temp'): os.makedirs('cpp_test_temp')
with zipfile.ZipFile('cpp_test_temp/test.zip', 'w') as zf:
    zf.writestr('file1.txt', 'content1')
    zf.writestr('dir1/file2.txt', 'content2')
    zf.writestr('dir1/emptydir/', '')
# For traversal test
with zipfile.ZipFile('cpp_test_temp/traversal.zip', 'w') as zf:
    zf.writestr('../../evil.txt', 'evil content')
*/


/**
 * Extracts a zip file to a specified destination directory.
 * Protects against path traversal attacks.
 *
 * @param zipFilePath Path to the zip file.
 * @param destDirectory Directory where files will be extracted.
 * @return true if extraction was successful, false otherwise.
 */
bool extractZip(const std::string& zipFilePath, const std::string& destDirectory) {
    namespace fs = std::filesystem;

    fs::path destDirPath(destDirectory);
    try {
        fs::create_directories(destDirPath);
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error creating destination directory: " << e.what() << std::endl;
        return false;
    }
    
    const auto destDirCanonical = fs::weakly_canonical(destDirPath);

    int err = 0;
    zip* z = zip_open(zipFilePath.c_str(), 0, &err);
    if (!z) {
        zip_error_t error;
        zip_error_init_with_code(&error, err);
        std::cerr << "Error opening zip file '" << zipFilePath << "': " << zip_error_strerror(&error) << std::endl;
        zip_error_fini(&error);
        return false;
    }

    zip_int64_t num_entries = zip_get_num_entries(z, 0);
    for (zip_int64_t i = 0; i < num_entries; i++) {
        const char* name = zip_get_name(z, i, 0);
        if (name == nullptr) continue;

        fs::path entryPath = destDirCanonical / name;
        auto entryPathCanonical = fs::weakly_canonical(entryPath);

        // Security Check: Path Traversal
        if (entryPathCanonical.string().rfind(destDirCanonical.string(), 0) != 0) {
            std::cerr << "Path traversal attempt detected. Skipping entry: " << name << std::endl;
            continue;
        }

        // Check if it's a directory (ends with /)
        std::string name_str(name);
        if (!name_str.empty() && name_str.back() == '/') {
            try {
                fs::create_directories(entryPathCanonical);
            } catch (const fs::filesystem_error& e) {
                std::cerr << "Error creating directory: " << e.what() << std::endl;
            }
        } else {
            // It's a file, ensure parent directory exists
            try {
                if(entryPathCanonical.has_parent_path()) {
                    fs::create_directories(entryPathCanonical.parent_path());
                }
            } catch (const fs::filesystem_error& e) {
                 std::cerr << "Error creating parent directory: " << e.what() << std::endl;
                 continue;
            }

            zip_stat_t sb;
            if (zip_stat_index(z, i, 0, &sb) == 0) {
                zip_file* zf = zip_fopen_index(z, i, 0);
                if (!zf) continue;

                std::ofstream ofs(entryPathCanonical, std::ios::binary);
                if (ofs.is_open()) {
                    std::vector<char> buffer(8192);
                    zip_int64_t bytes_read;
                    while ((bytes_read = zip_fread(zf, buffer.data(), buffer.size())) > 0) {
                        ofs.write(buffer.data(), bytes_read);
                    }
                }
                zip_fclose(zf);
            }
        }
    }

    zip_close(z);
    return true;
}

void runTests() {
    namespace fs = std::filesystem;
    std::cout << "--- Running C++ ZIP Extraction Tests ---" << std::endl;
    std::cout << "Note: Make sure test zip files exist in 'cpp_test_temp/'" << std::endl;

    fs::path tempDir = "cpp_test_temp";
    fs::path testZip = tempDir / "test.zip";
    fs::path traversalZip = tempDir / "traversal.zip";
    
    // Test Case 1: Normal extraction
    std::cout << "\n[Test 1] Normal Extraction" << std::endl;
    fs::path out1 = tempDir / "out1";
    fs::remove_all(out1);
    bool success1 = extractZip(testZip.string(), out1.string());
    std::cout << "Result: " << (success1 && fs::exists(out1 / "dir1" / "file2.txt") ? "PASS" : "FAIL") << std::endl;
    
    // Test Case 2: Extraction to a non-existent directory
    std::cout << "\n[Test 2] Extract to New Directory" << std::endl;
    fs::path out2 = tempDir / "out2_new";
    fs::remove_all(out2);
    bool success2 = extractZip(testZip.string(), out2.string());
    std::cout << "Result: " << (success2 && fs::exists(out2 / "file1.txt") ? "PASS" : "FAIL") << std::endl;

    // Test Case 3: Path traversal attack
    std::cout << "\n[Test 3] Path Traversal Attack" << std::endl;
    fs::path out3 = tempDir / "out3";
    fs::remove_all(out3);
    extractZip(traversalZip.string(), out3.string());
    fs::path evilFile = "evil.txt"; // It would be created in the current dir
    std::cout << "Result: " << (!fs::exists(evilFile) ? "PASS" : "FAIL") << std::endl;
    if (fs::exists(evilFile)) fs::remove(evilFile);

    // Test Case 4: Non-existent archive file
    std::cout << "\n[Test 4] Non-existent Archive" << std::endl;
    fs::path out4 = tempDir / "out4";
    bool success4 = extractZip("nonexistent.zip", out4.string());
    std::cout << "Result: " << (!success4 ? "PASS" : "FAIL") << std::endl;

    // Test Case 5: Output is a file
    std::cout << "\n[Test 5] Output is a File" << std::endl;
    fs::path outFile = tempDir / "output_file.txt";
    fs::remove_all(outFile);
    { std::ofstream ofs(outFile); ofs << "file"; }
    bool success5 = extractZip(testZip.string(), outFile.string());
    std::cout << "Result: " << (!success5 ? "PASS" : "FAIL") << std::endl;
    
    // Optional: cleanup test directories
    // fs::remove_all(out1);
    // fs::remove_all(out2);
    // fs::remove_all(out3);
    // fs::remove(outFile);
}

int main() {
    runTests();
    return 0;
}