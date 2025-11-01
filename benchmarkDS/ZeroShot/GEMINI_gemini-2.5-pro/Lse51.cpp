// This code requires the 'libarchive' library.
// To compile: g++ -std=c++17 your_file_name.cpp -o Lse51 -larchive
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <archive.h>
#include <archive_entry.h>

const std::string TEST_DIR = "/tmp/unpack_cpp_test";

/**
 * Creates a test TAR file using libarchive.
 */
void create_test_tar(const std::string& archive_name, const std::vector<std::pair<std::string, std::string>>& files) {
    struct archive *a = archive_write_new();
    archive_write_set_format_pax_restricted(a); 
    archive_write_open_filename(a, archive_name.c_str());

    for (const auto& file_pair : files) {
        struct archive_entry *entry = archive_entry_new();
        archive_entry_set_pathname(entry, file_pair.first.c_str());
        archive_entry_set_size(entry, file_pair.second.size());
        archive_entry_set_filetype(entry, AE_IFREG);
        archive_entry_set_perm(entry, 0644);
        archive_write_header(a, entry);
        archive_write_data(a, file_pair.second.c_str(), file_pair.second.size());
        archive_entry_free(entry);
    }
    archive_write_close(a);
    archive_write_free(a);
}

/**
 * Sets up the test environment.
 */
void setup_test_environment() {
    std::filesystem::remove_all(TEST_DIR);
    std::filesystem::create_directories(TEST_DIR);

    create_test_tar("good.tar", {{"file1.txt", "content1"}, {"dir1/file2.txt", "content2"}});
    create_test_tar("bad_traversal.tar", {{"../../tmp/evil.txt", "malicious"}});
    create_test_tar("bad_absolute.tar", {{"/tmp/evil_absolute.txt", "malicious"}});
    
    // Create a corrupt tar file
    std::ofstream("corrupt.tar") << "this is not a tar file";
}

/**
 * Cleans up the test environment.
 */
void cleanup_test_environment() {
    std::filesystem::remove_all(TEST_DIR);
    std::filesystem::remove("good.tar");
    std::filesystem::remove("bad_traversal.tar");
    std::filesystem::remove("bad_absolute.tar");
    std::filesystem::remove("corrupt.tar");
}

/**
 * Securely extracts a tar archive.
 * This is a C++ RAII-style equivalent of a 'with' statement for resource management.
 */
bool secure_untar(const std::string& tar_path, const std::string& extract_path_str) {
    struct archive *a;
    struct archive_entry *entry;
    int r;

    std::filesystem::path extract_path = std::filesystem::absolute(extract_path_str);

    a = archive_read_new();
    archive_read_support_filter_all(a);
    archive_read_support_format_all(a);

    r = archive_read_open_filename(a, tar_path.c_str(), 10240);
    if (r != ARCHIVE_OK) {
        std::cerr << "Error opening archive: " << archive_error_string(a) << std::endl;
        archive_read_free(a);
        return false;
    }

    bool success = true;
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        const char* current_file = archive_entry_pathname(entry);
        std::filesystem::path dest_path = extract_path / current_file;
        dest_path = std::filesystem::weakly_canonical(dest_path);

        // Security Check: Ensure the destination path is within the extraction directory
        auto [root, rel] = std::mismatch(extract_path.begin(), extract_path.end(), dest_path.begin());
        if (root != extract_path.end()) {
             std::cerr << "Security Error: Malicious entry detected (Path Traversal): " << current_file << std::endl;
             success = false;
             break;
        }

        archive_entry_set_pathname(entry, dest_path.c_str());
        r = archive_read_extract(a, entry, ARCHIVE_EXTRACT_TIME);
        if (r != ARCHIVE_OK) {
            std::cerr << "Error extracting " << current_file << ": " << archive_error_string(a) << std::endl;
            success = false;
            break;
        }
    }

    archive_read_close(a);
    archive_read_free(a);
    return success;
}

int main() {
    setup_test_environment();
    std::cout << "--- Running C++ Test Cases ---" << std::endl;

    // Test Case 1: Valid tar file
    std::cout << "\n[Test 1] Extracting a valid TAR file..." << std::endl;
    bool result1 = secure_untar("good.tar", TEST_DIR);
    std::cout << "Result: " << (result1 ? "SUCCESS" : "FAILURE") << std::endl;
    if (!result1 || !std::filesystem::exists(std::filesystem::path(TEST_DIR) / "file1.txt")) { return 1; }

    // Test Case 2: Path traversal attack
    std::cout << "\n[Test 2] Extracting a TAR with path traversal..." << std::endl;
    bool result2 = secure_untar("bad_traversal.tar", TEST_DIR);
    std::cout << "Result: " << (!result2 ? "SUCCESS (Attack Blocked)" : "FAILURE (Vulnerable)") << std::endl;
    if (result2 || std::filesystem::exists("/tmp/evil.txt")) { return 1; }
    
    // Test Case 3: Absolute path attack
    std::cout << "\n[Test 3] Extracting a TAR with an absolute path..." << std::endl;
    bool result3 = secure_untar("bad_absolute.tar", TEST_DIR);
    std::cout << "Result: " << (!result3 ? "SUCCESS (Attack Blocked)" : "FAILURE (Vulnerable)") << std::endl;
    if (result3 || std::filesystem::exists("/tmp/evil_absolute.txt")) { return 1; }

    // Test Case 4: Non-existent file
    std::cout << "\n[Test 4] Attempting to extract a non-existent TAR file..." << std::endl;
    bool result4 = secure_untar("non_existent.tar", TEST_DIR);
    std::cout << "Result: " << (!result4 ? "SUCCESS (Handled gracefully)" : "FAILURE") << std::endl;
    if (result4) { return 1; }

    // Test Case 5: Corrupt tar file
    std::cout << "\n[Test 5] Extracting a corrupt TAR file..." << std::endl;
    bool result5 = secure_untar("corrupt.tar", TEST_DIR);
    std::cout << "Result: " << (!result5 ? "SUCCESS (Handled gracefully)" : "FAILURE") << std::endl;
    if (result5) { return 1; }

    cleanup_test_environment();
    std::cout << "\n--- C++ Tests Complete, Cleanup Done ---" << std::endl;
    return 0;
}