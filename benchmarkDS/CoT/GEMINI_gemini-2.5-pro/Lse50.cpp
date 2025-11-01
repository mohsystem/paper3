#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <cstdlib> // For system()
#include <filesystem> // C++17 for path manipulation

// Requires libarchive. Link with -larchive
// On Debian/Ubuntu: sudo apt-get install libarchive-dev
// On macOS (Homebrew): brew install libarchive
#include <archive.h>
#include <archive_entry.h>

/**
 * Extracts a TAR archive to a specified directory using libarchive.
 * Relies on libarchive's built-in security features.
 *
 * @param tarFilePath Path to the .tar file.
 * @param destDirPath Path to the destination directory.
 * @return True on success, false on failure.
 */
bool extractTar(const std::string& tarFilePath, const std::string& destDirPath) {
    // Create destination directory if it doesn't exist
    std::filesystem::create_directories(destDirPath);
    
    struct archive *a;
    struct archive *ext;
    struct archive_entry *entry;
    int flags;
    int r;

    // Set flags for secure extraction
    flags = ARCHIVE_EXTRACT_TIME;
    flags |= ARCHIVE_EXTRACT_PERM;
    flags |= ARCHIVE_EXTRACT_ACL;
    flags |= ARCHIVE_EXTRACT_FFLAGS;
    // Security: Prevent .., absolute paths, and symlink traversals
    flags |= ARCHIVE_EXTRACT_SECURE_NODOTDOT; 
    flags |= ARCHIVE_EXTRACT_SECURE_SYMLINKS;
    flags |= ARCHIVE_EXTRACT_SECURE_NOABSOLUTEPATHS;

    a = archive_read_new();
    archive_read_support_format_all(a);
    archive_read_support_filter_all(a);

    ext = archive_write_disk_new();
    archive_write_disk_set_options(ext, flags);
    archive_write_disk_set_standard_lookup(ext);

    if ((r = archive_read_open_filename(a, tarFilePath.c_str(), 10240))) {
        std::cerr << "Error opening archive: " << archive_error_string(a) << std::endl;
        archive_read_free(a);
        archive_write_free(ext);
        return false;
    }

    bool success = true;
    for (;;) {
        r = archive_read_next_header(a, &entry);
        if (r == ARCHIVE_EOF) {
            break;
        }
        if (r < ARCHIVE_OK) {
            std::cerr << "Error reading header: " << archive_error_string(a) << std::endl;
        }
        if (r < ARCHIVE_WARN) {
            success = false;
            break;
        }

        // Prepend destination path to the entry's path
        std::filesystem::path current_entry_path = archive_entry_pathname(entry);
        std::filesystem::path full_dest_path = std::filesystem::path(destDirPath) / current_entry_path;
        archive_entry_set_pathname(entry, full_dest_path.c_str());

        r = archive_write_header(ext, entry);
        if (r < ARCHIVE_OK) {
            std::cerr << "Error writing header: " << archive_error_string(ext) << std::endl;
        } else if (archive_entry_size(entry) > 0) {
            const void *buff;
            size_t size;
#if ARCHIVE_VERSION_NUMBER >= 3000000
            int64_t offset;
#else
            off_t offset;
#endif
            for (;;) {
                r = archive_read_data_block(a, &buff, &size, &offset);
                if (r == ARCHIVE_EOF) break;
                if (r < ARCHIVE_OK) {
                     std::cerr << "Error reading data block: " << archive_error_string(a) << std::endl;
                     success = false;
                     break;
                }
                r = archive_write_data_block(ext, buff, size, offset);
                if (r < ARCHIVE_OK) {
                    std::cerr << "Error writing data block: " << archive_error_string(ext) << std::endl;
                    success = false;
                    break;
                }
            }
            if (!success) break;
        }
        r = archive_write_finish_entry(ext);
        if (r < ARCHIVE_OK) {
            std::cerr << "Error finishing entry: " << archive_error_string(ext) << std::endl;
        }
        if (r < ARCHIVE_WARN) {
            success = false;
            break;
        }
    }

    archive_read_close(a);
    archive_read_free(a);
    archive_write_close(ext);
    archive_write_free(ext);

    return success;
}

// Helper to create test files
void setupTestFiles() {
    std::filesystem::create_directories("test_data/subdir");
    std::ofstream("test_data/file1.txt") << "hello";
    std::ofstream("test_data/subdir/file2.txt") << "world";
    std::ofstream("test_data/evil.txt") << "malicious";
}

// Helper to run a system command
void runCommand(const std::string& cmd) {
    int result = system(cmd.c_str());
    if (result != 0) {
        std::cerr << "Command failed: " << cmd << std::endl;
    }
}

int main() {
    setupTestFiles();

    std::vector<std::pair<std::string, std::string>> testCases = {
        {"test_case_1_normal", "tar -cf test_case_1_normal.tar -C test_data file1.txt subdir/file2.txt"},
        {"test_case_2_empty_tar", "tar -cf test_case_2_empty_tar.tar --files-from /dev/null"},
        {"test_case_3_nested_dirs", "tar -cf test_case_3_nested_dirs.tar --transform 's,^,a/b/c/,' -C test_data file1.txt"},
        {"test_case_4_malicious_path_rel", "tar -cf test_case_4_malicious_path_rel.tar --transform 's,^,../../,' -C test_data evil.txt"},
        {"test_case_5_malicious_path_abs", "tar -cf test_case_5_malicious_path_abs.tar -C test_data --transform 's,^,/tmp/,' evil.txt"}
    };

    for (const auto& test : testCases) {
        const std::string& testName = test.first;
        const std::string& createCmd = test.second;
        std::string tarFile = testName + ".tar";
        std::string unpackDir = "/tmp/unpack_cpp_" + testName;

        std::cout << "--- Running " << testName << " ---" << std::endl;
        
        // 1. Create TAR file using system tar command
        runCommand(createCmd);

        // 2. Run extraction
        std::cout << "Extracting " << tarFile << " to " << unpackDir << std::endl;
        if (extractTar(tarFile, unpackDir)) {
            std::cout << "Extraction finished." << std::endl;
        } else {
            std::cerr << "Extraction failed (might be an expected failure for a malicious archive)." << std::endl;
        }

        // 3. Cleanup
        std::filesystem::remove(tarFile);
        if (std::filesystem::exists(unpackDir)) {
            std::filesystem::remove_all(unpackDir);
        }
        std::cout << "Cleaned up " << tarFile << " and " << unpackDir << std::endl;
        std::cout << std::endl;
    }

    // Global cleanup
    std::filesystem::remove_all("test_data");

    return 0;
}