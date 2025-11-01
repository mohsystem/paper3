// Needs C++17 for std::filesystem.
// Compile with: g++ this_file.cpp -o Lse49 -std=c++17 -lzip
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <zip.h> // From libzip library

/**
 * The prompt requests to "Open the archive.zip file as a tarfile".
 * This is technically incorrect as .zip and .tar are different archive formats.
 * This function correctly processes a .zip file using the libzip library.
 * It requires libzip to be installed (e.g., 'sudo apt-get install libzip-dev' on Debian/Ubuntu).
 */
bool unzip(const std::string& zipFilePath, const std::string& destDirectory) {
    namespace fs = std::filesystem;

    if (!fs::exists(zipFilePath)) {
        std::cerr << "Error: Zip file not found at " << zipFilePath << std::endl;
        return false;
    }

    int err = 0;
    zip* za = zip_open(zipFilePath.c_str(), 0, &err);
    if (za == nullptr) {
        zip_error_t ziperror;
        zip_error_init_with_code(&ziperror, err);
        std::cerr << "Error opening zip file " << zipFilePath << ": " << zip_error_strerror(&ziperror) << std::endl;
        zip_error_fini(&ziperror);
        return false;
    }

    try {
        fs::create_directories(destDirectory);
        fs::path destPath(destDirectory);
        fs::path canonicalDestPath = fs::weakly_canonical(destPath);

        zip_int64_t num_entries = zip_get_num_entries(za, 0);
        for (zip_int64_t i = 0; i < num_entries; i++) {
            const char* name = zip_get_name(za, i, 0);
            if (name == nullptr) {
                std::cerr << "Error reading entry name at index " << i << std::endl;
                continue;
            }

            fs::path entryPath = canonicalDestPath / name;
            fs::path canonicalEntryPath = fs::weakly_canonical(entryPath);

            // SECURITY CHECK: Prevent Path Traversal (Zip Slip)
            // Check if the resolved path of the entry is within the destination directory.
            // A robust way is to check if the canonical destination path is a prefix of the canonical entry path.
            if (canonicalEntryPath.string().rfind(canonicalDestPath.string(), 0) != 0) {
                 std::cerr << "Security Error: Path traversal attempt detected for entry '" << name << "'. Skipping." << std::endl;
                 continue;
            }

            std::string s_name = name;
            if (!s_name.empty() && s_name.back() == '/') {
                fs::create_directories(entryPath);
            } else {
                zip_stat_t zs;
                if (zip_stat_index(za, i, 0, &zs) == 0) {
                    zip_file_t* zf = zip_fopen_index(za, i, 0);
                    if (!zf) {
                        std::cerr << "Error opening file in zip: " << name << std::endl;
                        continue;
                    }

                    if (entryPath.has_parent_path()) {
                        fs::create_directories(entryPath.parent_path());
                    }

                    std::ofstream ofs(entryPath, std::ios::binary);
                    if (!ofs) {
                        std::cerr << "Error creating output file: " << entryPath.string() << std::endl;
                        zip_fclose(zf);
                        continue;
                    }

                    char buffer[4096];
                    zip_int64_t n;
                    while ((n = zip_fread(zf, buffer, sizeof(buffer))) > 0) {
                        ofs.write(buffer, n);
                    }
                    zip_fclose(zf);
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
        zip_close(za);
        return false;
    }

    zip_close(za);
    return true;
}

class Lse49 {
public:
    static void runTests() {
        std::cout << "Running C++ Unzip Test Cases..." << std::endl;
        std::cout << "NOTE: This requires manually creating .zip files for testing." << std::endl;
        
        namespace fs = std::filesystem;
        fs::path baseTestDir = fs::temp_directory_path() / "cpp_unzip_tests_lse49";
        fs::create_directories(baseTestDir);
        std::cout << "Test files should be placed in: " << baseTestDir << std::endl;

        std::cout << "\nInstructions for creating test files (run in shell):\n"
                  << "cd " << baseTestDir << "\n"
                  << "echo 'content1' > file1.txt; zip archive1.zip file1.txt; rm file1.txt\n"
                  << "mkdir dir1; echo 'content2' > dir1/file2.txt; zip -r archive2.zip dir1; rm -r dir1\n"
                  << "mkdir emptydir; zip -r archive3.zip emptydir; rm -r emptydir\n"
                  << "echo 'test' > test.txt; zip archive4.zip test.txt; rm test.txt\n"
                  << "zip archive5.zip -d file_to_remove >/dev/null 2>&1 || touch archive5.zip # create empty zip\n";

        // Test Case 1
        std::cout << "\n--- Test Case 1: Simple zip ---" << std::endl;
        std::string zip1 = (baseTestDir / "archive1.zip").string();
        std::string unpack1 = (baseTestDir / "unpack1").string();
        if(unzip(zip1, unpack1)) {
            std::cout << "Unzip successful. Verify: " << (fs::path(unpack1) / "file1.txt").string() << std::endl;
        }

        // Test Case 2
        std::cout << "\n--- Test Case 2: Zip with subdirectory ---" << std::endl;
        std::string zip2 = (baseTestDir / "archive2.zip").string();
        std::string unpack2 = (baseTestDir / "unpack2").string();
        if(unzip(zip2, unpack2)) {
            std::cout << "Unzip successful. Verify: " << (fs::path(unpack2) / "dir1" / "file2.txt").string() << std::endl;
        }

        // Test Case 3
        std::cout << "\n--- Test Case 3: Zip with empty directory ---" << std::endl;
        std::string zip3 = (baseTestDir / "archive3.zip").string();
        std::string unpack3 = (baseTestDir / "unpack3").string();
        if(unzip(zip3, unpack3)) {
            std::cout << "Unzip successful. Verify: " << (fs::path(unpack3) / "emptydir").string() << std::endl;
        }

        // Test Case 4
        std::cout << "\n--- Test Case 4: Unzip to non-existent directory ---" << std::endl;
        std::string zip4 = (baseTestDir / "archive4.zip").string();
        std::string unpack4 = (baseTestDir / "unpack4").string();
        if(unzip(zip4, unpack4)) {
            std::cout << "Unzip successful. Verify: " << (fs::path(unpack4) / "test.txt").string() << std::endl;
        }

        // Test Case 5
        std::cout << "\n--- Test Case 5: Unzip empty zip ---" << std::endl;
        std::string zip5 = (baseTestDir / "archive5.zip").string();
        std::string unpack5 = (baseTestDir / "unpack5").string();
        if(unzip(zip5, unpack5)) {
            std::cout << "Unzip successful. Verify directory '" << unpack5 << "' is empty." << std::endl;
        }
        
        std::cout << "\nC++ tests complete. Please manually clean up " << baseTestDir << std::endl;
    }
};

int main() {
    Lse49::runTests();
    return 0;
}