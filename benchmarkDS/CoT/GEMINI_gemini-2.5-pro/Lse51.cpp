#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>

// Required external library: libarchive
// On Debian/Ubuntu: sudo apt-get install libarchive-dev
// On macOS (Homebrew): brew install libarchive
// Compile with: g++ your_source_file.cpp -o your_executable -larchive -std=c++17
#include <archive.h>
#include <archive_entry.h>

namespace fs = std::filesystem;

/**
 * Copies data from the archive to a file on disk.
 */
static int copy_data(struct archive *ar, std::ofstream& os) {
    const void *buff;
    size_t size;
    la_int64_t offset;
    int r;

    for (;;) {
        r = archive_read_data_block(ar, &buff, &size, &offset);
        if (r == ARCHIVE_EOF) {
            return ARCHIVE_OK;
        }
        if (r < ARCHIVE_OK) {
            return r;
        }
        os.write(static_cast<const char*>(buff), size);
        if (!os) {
            return ARCHIVE_FATAL;
        }
    }
}

/**
 * Safely extracts a tar archive, preventing path traversal attacks.
 * Uses libarchive to support various formats.
 *
 * @param tarPath The path to the tar archive.
 * @param destDir The destination directory for extraction.
 * @return true if extraction was successful, false otherwise.
 */
bool extractTar(const std::string& tarPath, const std::string& destDir) {
    if (!fs::exists(tarPath)) {
        std::cerr << "Error: Archive file not found: " << tarPath << std::endl;
        return false;
    }

    struct archive *a;
    struct archive_entry *entry;
    int r;

    a = archive_read_new();
    archive_read_support_format_all(a);
    archive_read_support_filter_all(a);
    
    r = archive_read_open_filename(a, tarPath.c_str(), 10240);
    if (r != ARCHIVE_OK) {
        std::cerr << "Error opening archive: " << archive_error_string(a) << std::endl;
        archive_read_free(a);
        return false;
    }

    try {
        fs::path destPath(destDir);
        fs::create_directories(destPath);
        destPath = fs::canonical(destPath); // Get absolute, real path

        std::cout << "Extracting " << tarPath << " to " << destPath << std::endl;

        while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
            const char* entry_pathname_c = archive_entry_pathname(entry);
            fs::path entryPath = destPath / entry_pathname_c;
            
            // Security Check: Path Traversal
            // Normalize the path to resolve ".." and "."
            fs::path lexicallyNormalPath = entryPath.lexically_normal();
            auto [root, rel] = std::mismatch(destPath.begin(), destPath.end(), lexicallyNormalPath.begin());

            if (root != destPath.end()) {
                std::cerr << "SECURITY: Skipping potentially malicious entry: " << entry_pathname_c << std::endl;
                continue;
            }

            // Security Check: Absolute paths in archive
            if (fs::path(entry_pathname_c).is_absolute()) {
                std::cerr << "SECURITY: Skipping absolute path entry: " << entry_pathname_c << std::endl;
                continue;
            }
            
            std::cout << "  -> " << entry_pathname_c << std::endl;
            
            if (archive_entry_filetype(entry) == AE_IFDIR) {
                fs::create_directories(lexicallyNormalPath);
            } else if (archive_entry_filetype(entry) == AE_IFREG) {
                fs::create_directories(lexicallyNormalPath.parent_path());
                std::ofstream outFile(lexicallyNormalPath, std::ios::binary);
                if (!outFile) {
                    std::cerr << "Error: Could not open file for writing: " << lexicallyNormalPath << std::endl;
                    continue;
                }
                copy_data(a, outFile);
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
        archive_read_close(a);
        archive_read_free(a);
        return false;
    }

    archive_read_close(a);
    archive_read_free(a);
    return true;
}

int main() {
    // --- Test Setup ---
    // These test archives must be created beforehand.
    // You can generate them using the provided Python script.
    std::string testDataBaseDir = "/tmp/lse51_test_data/";
    std::vector<std::string> testArchives = {
        testDataBaseDir + "test.tar",
        testDataBaseDir + "test.tar.gz",
        testDataBaseDir + "test.tar.bz2",
        testDataBaseDir + "malicious_traversal.tar",
        testDataBaseDir + "malicious_absolute.tar"
    };
    std::string unpackDir = "/tmp/unpack_cpp";

    std::cout << "--- C++ Tar Extraction Tests ---" << std::endl;
    std::cout << "NOTE: Test archives are expected in " << testDataBaseDir << std::endl;
    std::cout << "You can generate them by running the Python code in this file.\n" << std::endl;

    for (size_t i = 0; i < testArchives.size(); ++i) {
        std::cout << "\n--- Test Case " << (i + 1) << " ---" << std::endl;
        try {
            if (fs::exists(unpackDir)) {
                fs::remove_all(unpackDir);
            }
            extractTar(testArchives[i], unpackDir);
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Failed to clean directory: " << e.what() << std::endl;
        }
    }
    
    std::cout << "\n--- C++ Tests Complete ---" << std::endl;
    return 0;
}