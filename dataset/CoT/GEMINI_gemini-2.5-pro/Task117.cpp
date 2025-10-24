/*
 * IMPORTANT: This code requires the 'libarchive' library.
 *
 * To compile on Linux/macOS:
 * g++ -std=c++17 Task117.cpp -o Task117 -larchive
 *
 * To install libarchive:
 * - On Debian/Ubuntu: sudo apt-get install libarchive-dev
 * - On RedHat/CentOS: sudo yum install libarchive-devel
 * - On macOS (Homebrew): brew install libarchive
 */
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <archive.h>
#include <archive_entry.h>

namespace fs = std::filesystem;

/**
 * Extracts an archive (ZIP, TAR, etc.) to a destination directory.
 * Uses libarchive to support multiple formats and ensures extraction is safe
 * from path traversal attacks.
 *
 * @param archivePath Path to the archive file.
 * @param destDir     Path to the destination directory.
 * @return True if successful, false otherwise.
 */
bool extractArchive(const std::string& archivePath, const std::string& destDir) {
    fs::path destPath(destDir);
    // Create destination directory if it doesn't exist.
    if (!fs::exists(destPath)) {
        if (!fs::create_directories(destPath)) {
            std::cerr << "Error: Could not create destination directory " << destDir << std::endl;
            return false;
        }
    }
    // Get canonical path for security checks
    const fs::path canonicalDestPath = fs::canonical(destPath);

    struct archive *a;
    struct archive_entry *entry;
    int r;
    bool success = true;

    a = archive_read_new();
    archive_read_support_filter_all(a);
    archive_read_support_format_all(a);

    r = archive_read_open_filename(a, archivePath.c_str(), 10240);
    if (r != ARCHIVE_OK) {
        std::cerr << "Error opening archive: " << archive_error_string(a) << std::endl;
        archive_read_free(a);
        return false;
    }

    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        const char* current_file_c = archive_entry_pathname(entry);
        fs::path entryPath = fs::path(current_file_c);
        fs::path fullOutputPath = destPath / entryPath;

        // --- Security Check for Path Traversal ---
        fs::path canonicalOutputPath = fs::weakly_canonical(fullOutputPath);
        // Check if the resulting path is inside the destination directory.
        // We do this by comparing the canonical paths as strings.
        if (canonicalOutputPath.string().find(canonicalDestPath.string()) != 0) {
            std::cerr << "Path Traversal attempt detected. Skipping entry: " << current_file_c << std::endl;
            continue;
        }

        // Set permissions from archive entry
        archive_entry_set_pathname(entry, fullOutputPath.c_str());
        r = archive_read_extract(a, entry, ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM | ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS);
        if (r != ARCHIVE_OK) {
            std::cerr << "Error extracting " << current_file_c << ": " << archive_error_string(a) << std::endl;
            success = false;
        }
    }

    archive_read_close(a);
    archive_read_free(a);
    return success;
}

void createTestFiles() {
    std::cout << "--------------------------------------------------------\n";
    std::cout << "This C++ program requires test archive files to exist.\n";
    std::cout << "Please create them manually or using a script.\n\n";
    std::cout << "Example commands to create test files:\n";
    std::cout << "  mkdir -p temp_for_zip/dir1\n";
    std::cout << "  echo 'content' > temp_for_zip/dir1/file1.txt\n";
    std::cout << "  (cd temp_for_zip && zip -r ../test.zip .)\n";
    std::cout << "  (cd temp_for_zip && tar -czf ../test.tar.gz .)\n";
    std::cout << "  zip malicious.zip ../../malicious_file.txt\n";
    std::cout << "  rm -rf temp_for_zip\n";
    std::cout << "--------------------------------------------------------\n" << std::endl;
}


int main() {
    const std::string baseTestDir = "cpp_test_area";
    fs::create_directories(baseTestDir);

    createTestFiles();

    // --- Test Case 1: Normal ZIP Extraction ---
    // Assumes test.zip exists with 'dir1/file1.txt' inside.
    std::cout << "--- Test Case 1: Normal ZIP Extraction ---" << std::endl;
    std::string normalZip = "test.zip";
    std::string extractDir1 = baseTestDir + "/extract1";
    if (fs::exists(normalZip)) {
        bool success1 = extractArchive(normalZip, extractDir1);
        std::cout << "Extraction successful: " << std::boolalpha << success1 << std::endl;
        std::cout << "Exists 'extract1/dir1/file1.txt': " << fs::exists(fs::path(extractDir1) / "dir1/file1.txt") << std::endl;
    } else {
        std::cout << "Skipping test, file not found: " << normalZip << std::endl;
    }
    std::cout << std::endl;

    // --- Test Case 2: Normal TAR Extraction ---
    // Assumes test.tar.gz exists with 'dir1/file1.txt' inside.
    std::cout << "--- Test Case 2: Normal TAR Extraction ---" << std::endl;
    std::string normalTar = "test.tar.gz";
    std::string extractDir2 = baseTestDir + "/extract2";
    if (fs::exists(normalTar)) {
        bool success2 = extractArchive(normalTar, extractDir2);
        std::cout << "Extraction successful: " << std::boolalpha << success2 << std::endl;
        std::cout << "Exists 'extract2/dir1/file1.txt': " << fs::exists(fs::path(extractDir2) / "dir1/file1.txt") << std::endl;
    } else {
        std::cout << "Skipping test, file not found: " << normalTar << std::endl;
    }
    std::cout << std::endl;

    // --- Test Case 3: Malicious ZIP with Path Traversal ---
    // Assumes malicious.zip exists with a '../../malicious_file.txt' entry.
    std::cout << "--- Test Case 3: Malicious ZIP Extraction ---" << std::endl;
    std::string maliciousZip = "malicious.zip";
    std::string extractDir3 = baseTestDir + "/extract3";
    if (fs::exists(maliciousZip)) {
        bool success3 = extractArchive(maliciousZip, extractDir3);
        std::cout << "Extraction operation finished: " << std::boolalpha << success3 << std::endl;
        std::cout << "Malicious file created outside test dir: " << fs::exists("malicious_file.txt") << std::endl;
    } else {
        std::cout << "Skipping test, file not found: " << maliciousZip << std::endl;
    }
    std::cout << std::endl;

    // --- Test Case 4: Extracting to a non-existent directory ---
    std::cout << "--- Test Case 4: Extract to Non-Existent Directory ---" << std::endl;
    std::string extractDir4 = baseTestDir + "/new_dir_extract";
    if (fs::exists(normalZip)) {
        bool success4 = extractArchive(normalZip, extractDir4);
        std::cout << "Extraction successful: " << std::boolalpha << success4 << std::endl;
        std::cout << "Exists 'new_dir_extract/dir1/file1.txt': " << fs::exists(fs::path(extractDir4) / "dir1/file1.txt") << std::endl;
    } else {
        std::cout << "Skipping test, file not found: " << normalZip << std::endl;
    }
    std::cout << std::endl;

    // --- Test Case 5: Archive path does not exist ---
    std::cout << "--- Test Case 5: Archive Path Not Found ---" << std::endl;
    std::string nonExistentZip = "notfound.zip";
    std::string extractDir5 = baseTestDir + "/extract5";
    bool success5 = extractArchive(nonExistentZip, extractDir5);
    std::cout << "Extraction successful: " << std::boolalpha << success5 << std::endl;
    std::cout << std::endl;

    // Cleanup
    std::cout << "Cleaning up test directory..." << std::endl;
    fs::remove_all(baseTestDir);
    std::cout << "Cleanup complete. (Manual cleanup of test archives may be needed)" << std::endl;

    return 0;
}