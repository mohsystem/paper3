#include <iostream>
#include <string>
#include <filesystem>
#include <vector>

// Required for libarchive
#include <archive.h>
#include <archive_entry.h>

/*
================================================================================
IMPORTANT: This code requires the 'libarchive' library.
To compile on a Linux-like system, you need to install it first:
  - On Debian/Ubuntu: sudo apt-get install libarchive-dev
  - On Fedora/CentOS: sudo dnf install libarchive-devel
  - On macOS (with Homebrew): brew install libarchive

Then, compile this file using a C++17 compatible compiler (for std::filesystem):
  g++ -std=c++17 Task117.cpp -o Task117 -larchive
================================================================================
*/

class Task117 {
public:
    /**
     * Extracts an archive file (ZIP, TAR, etc.) to a destination directory.
     * @param archivePath Path to the archive file.
     * @param destDir Path to the destination directory.
     * @return true on success, false on failure.
     */
    static bool extractArchive(const std::string& archivePath, const std::string& destDir) {
        // Ensure destination directory exists and get its canonical path
        std::error_code ec;
        std::filesystem::create_directories(destDir, ec);
        if (ec) {
            std::cerr << "Error creating destination directory: " << ec.message() << std::endl;
            return false;
        }
        const auto canonicalDestDir = std::filesystem::canonical(destDir, ec);
         if (ec) {
            std::cerr << "Error getting canonical path for destination: " << ec.message() << std::endl;
            return false;
        }

        struct archive* a = archive_read_new();
        struct archive_entry* entry;
        bool success = true;

        archive_read_support_filter_all(a);
        archive_read_support_format_all(a);

        if (archive_read_open_filename(a, archivePath.c_str(), 10240) != ARCHIVE_OK) {
            std::cerr << "Error opening archive: " << archive_error_string(a) << std::endl;
            archive_read_free(a);
            return false;
        }

        while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
            const char* entry_pathname_c = archive_entry_pathname(entry);
            std::filesystem::path entry_path(entry_pathname_c);

            // Security Check: Path Traversal (Zip Slip)
            auto full_path = canonicalDestDir / entry_path;
            auto clean_path = full_path.lexically_normal();
            
            auto [first1, first2] = std::mismatch(canonicalDestDir.begin(), canonicalDestDir.end(), clean_path.begin());

            if (first1 != canonicalDestDir.end()) {
                std::cerr << "Path Traversal security violation detected: " << entry_pathname_c << std::endl;
                success = false;
                break;
            }
            
            // Set the path for extraction to the sanitized path
            archive_entry_set_pathname(entry, clean_path.c_str());

            if (archive_read_extract(a, entry, ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM) != ARCHIVE_OK) {
                std::cerr << "Error extracting " << entry_pathname_c << ": " << archive_error_string(a) << std::endl;
                success = false;
                break;
            }
        }

        archive_read_close(a);
        archive_read_free(a);
        return success;
    }
};

int main() {
    std::cout << "C++ Archive Extractor - Note: This program expects you to create the files for testing.\n";
    std::cout << "Please create a directory 'cpp_test_area' and place some archives in it.\n";
    std::cout << "For example, create 'cpp_test_area/test.zip' and 'cpp_test_area/test.tar.gz'.\n\n";

    const std::string testDir = "cpp_test_area";
    const std::string outputDir = testDir + "/output";

    // Test Case 1: Extract a ZIP file
    std::cout << "--- Test Case 1: Extracting a valid ZIP file (e.g., 'cpp_test_area/test.zip') ---" << std::endl;
    if (Task117::extractArchive(testDir + "/test.zip", outputDir + "/zip_out")) {
        std::cout << "Success: ZIP file extracted.\n";
    } else {
        std::cout << "Failure: Could not extract ZIP file (does it exist?).\n";
    }

    // Test Case 2: Extract a TAR.GZ file
    std::cout << "\n--- Test Case 2: Extracting a valid TAR.GZ file (e.g., 'cpp_test_area/test.tar.gz') ---" << std::endl;
    if (Task117::extractArchive(testDir + "/test.tar.gz", outputDir + "/tar_out")) {
        std::cout << "Success: TAR.GZ file extracted.\n";
    } else {
        std::cout << "Failure: Could not extract TAR.GZ file (does it exist?).\n";
    }

    // Test Case 3: Attempt to extract a non-existent file
    std::cout << "\n--- Test Case 3: Non-existent archive ---" << std::endl;
    if (!Task117::extractArchive("non_existent_archive.zip", outputDir + "/non_existent_out")) {
        std::cout << "Success: Correctly handled non-existent archive.\n";
    } else {
        std::cout << "Failure: Program behaved unexpectedly.\n";
    }

    // Test Case 4: Invalid destination directory (e.g., a file)
    std::cout << "\n--- Test Case 4: Destination is a file ---" << std::endl;
    std::filesystem::create_directories(testDir);
    FILE* f = fopen((testDir + "/file_as_dest").c_str(), "w");
    if (f) fclose(f);
    if (!Task117::extractArchive(testDir + "/test.zip", testDir + "/file_as_dest")) {
        std::cout << "Success: Correctly handled destination being a file.\n";
    } else {
        std::cout << "Failure: Program behaved unexpectedly.\n";
    }

    // Test Case 5: Path Traversal (Zip Slip)
    std::cout << "\n--- Test Case 5: Path Traversal (Zip Slip) ---" << std::endl;
    std::cout << "This requires a specially crafted archive (e.g., 'malicious.zip' with a file path like '../../evil.txt').\n";
    if (!Task117::extractArchive("malicious.zip", outputDir + "/malicious_out")) {
        std::cout << "Success: Malicious archive extraction was blocked or failed as expected.\n";
    } else {
        std::cout << "Warning: Malicious archive extracted without error (check if files were created outside target dir).\n";
    }

    return 0;
}