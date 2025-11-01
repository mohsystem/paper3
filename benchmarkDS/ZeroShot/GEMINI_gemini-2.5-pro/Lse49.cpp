// This code requires the libzip library.
// To compile: g++ Lse49.cpp -o Lse49 -lzip -std=c++17
// To run: ./Lse49

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <system_error>
#include <zip.h>

namespace fs = std::filesystem;

class Lse49 {
public:
    /**
     * Unzips an archive to a destination directory, preventing path traversal attacks.
     * @param zipFilePath Path to the .zip file.
     * @param destDirPath Path to the destination directory.
     */
    static void unzip(const std::string& zipFilePath, const std::string& destDirPath) {
        // Ensure destination directory exists and get its canonical path
        std::error_code ec;
        fs::create_directories(destDirPath, ec);
        if (ec) {
            throw std::runtime_error("Could not create destination directory: " + ec.message());
        }
        const fs::path destDirCanonical = fs::canonical(destDirPath, ec);
        if (ec) {
            throw std::runtime_error("Could not get canonical path of destination: " + ec.message());
        }

        int err = 0;
        zip* za = zip_open(zipFilePath.c_str(), 0, &err);
        if (!za) {
            throw std::runtime_error("Failed to open zip archive. Code: " + std::to_string(err));
        }

        zip_int64_t num_entries = zip_get_num_entries(za, 0);
        for (zip_int64_t i = 0; i < num_entries; i++) {
            const char* name = zip_get_name(za, i, 0);
            if (name == nullptr) {
                std::cerr << "Warning: Could not get name for entry " << i << std::endl;
                continue;
            }

            fs::path destFilePath = destDirCanonical / name;

            // Security Check for Path Traversal (Zip Slip)
            fs::path destFileCanonical = fs::weakly_canonical(destFilePath, ec);
            if (ec) {
                 std::cerr << "Warning: Could not get canonical path for " << name << ". Skipping." << std::endl;
                 continue;
            }

            if (destFileCanonical.string().find(destDirCanonical.string()) != 0) {
                 std::cerr << "Skipping malicious path: " << name << std::endl;
                 continue;
            }

            // If it's a directory (ends with /), create it
            if (name[strlen(name) - 1] == '/') {
                fs::create_directories(destFileCanonical, ec);
            } else {
                 // Ensure parent directory exists
                fs::create_directories(destFileCanonical.parent_path(), ec);

                zip_stat_t zs;
                zip_stat_index(za, i, 0, &zs);

                zip_file* zf = zip_fopen_index(za, i, 0);
                if (!zf) {
                    std::cerr << "Warning: Could not open file in zip: " << name << std::endl;
                    continue;
                }

                std::vector<char> buffer(zs.size);
                zip_fread(zf, buffer.data(), zs.size);
                zip_fclose(zf);
                
                std::ofstream of(destFileCanonical, std::ios::binary);
                of.write(buffer.data(), buffer.size());
            }
        }
        zip_close(za);
    }
};

// Helper function to create a test zip file using libzip
void createTestZip(const std::string& zipFileName, int testCase) {
    int error = 0;
    zip* za = zip_open(zipFileName.c_str(), ZIP_CREATE | ZIP_TRUNCATE, &error);
    if (!za) {
        throw std::runtime_error("Failed to create zip archive. Code: " + std::to_string(error));
    }

    std::string content1 = "content of file1";
    zip_source_t* source1 = zip_source_buffer(za, content1.c_str(), content1.length(), 0);
    zip_file_add(za, "file1.txt", source1, ZIP_FL_OVERWRITE);

    std::string content2 = "content of file2";
    zip_source_t* source2 = zip_source_buffer(za, content2.c_str(), content2.length(), 0);
    zip_file_add(za, "dir1/file2.txt", source2, ZIP_FL_OVERWRITE);

    if (testCase > 1) {
        std::string content3 = "malicious content";
        zip_source_t* source3 = zip_source_buffer(za, content3.c_str(), content3.length(), 0);
        zip_file_add(za, "../../evil.txt", source3, ZIP_FL_OVERWRITE);
    }
    
    zip_close(za);
}

// Helper to clean up test files
void cleanup(const std::string& zipFileName, const std::string& unpackDir) {
    std::error_code ec;
    fs::remove(zipFileName, ec);
    fs::remove_all(unpackDir, ec);
    fs::remove("/tmp/evil.txt", ec);
}

int main() {
    const std::string zipFileName = "archive.zip";
    const std::string unpackDir = "/tmp/unpack";

    for (int i = 1; i <= 5; ++i) {
        std::cout << "--- Running Test Case " << i << " ---" << std::endl;
        try {
            // Setup
            createTestZip(zipFileName, i);
            std::cout << "Created " << zipFileName << std::endl;

            // Execute
            Lse49::unzip(zipFileName, unpackDir);
            std::cout << "Successfully ran unzip on " << unpackDir << std::endl;

            // Verify
            if (fs::exists(fs::path(unpackDir) / "file1.txt")) std::cout << "Verified: file1.txt exists." << std::endl;
            if (fs::exists(fs::path(unpackDir) / "dir1/file2.txt")) std::cout << "Verified: dir1/file2.txt exists." << std::endl;
            if (fs::exists("/tmp/evil.txt")) {
                std::cerr << "SECURITY VULNERABILITY: Traversal file created at /tmp/evil.txt" << std::endl;
            } else {
                std::cout << "Security check passed: Traversal file was not created." << std::endl;
            }

        } catch (const std::runtime_error& e) {
            std::cerr << "Caught expected exception: " << e.what() << std::endl;
        }
        
        // Teardown
        std::cout << "Cleaning up..." << std::endl;
        cleanup(zipFileName, unpackDir);
        std::cout << "---------------------------\n" << std::endl;
    }

    return 0;
}