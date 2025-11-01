// Requires libarchive.
// Compile with: g++ -std=c++17 your_file.cpp -larchive -o your_executable
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <cstdlib> // For system()
#include <memory>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <archive.h>
#include <archive_entry.h>

/**
 * Securely extracts a tar archive to a destination directory.
 * Prevents path traversal by ensuring all extracted files are within the destination directory.
 *
 * @param tar_path Path to the tar archive.
 * @param dest_dir Path to the destination directory.
 * @return True on success, false on failure.
 */
bool secure_extract_tar(const std::filesystem::path& tar_path, const std::filesystem::path& dest_dir) {
    if (!std::filesystem::is_regular_file(tar_path)) {
        std::cerr << "Error: Tar file is not a regular file: " << tar_path << std::endl;
        return false;
    }

    std::error_code ec;
    std::filesystem::create_directories(dest_dir, ec);
    if (ec) {
        std::cerr << "Error: Could not create destination directory: " << dest_dir << std::endl;
        return false;
    }
    
    // Canonical path of the destination directory is our security boundary
    auto canon_dest_dir = std::filesystem::canonical(dest_dir, ec);
    if (ec) {
        std::cerr << "Error: Could not get canonical path for destination: " << dest_dir << std::endl;
        return false;
    }

    struct archive* a = archive_read_new();
    // RAII for archive pointer
    std::unique_ptr<struct archive, decltype(&archive_read_free)> archive_ptr(a, &archive_read_free);
    
    archive_read_support_filter_all(a);
    archive_read_support_format_all(a);

    if (archive_read_open_filename(a, tar_path.c_str(), 10240) != ARCHIVE_OK) {
        std::cerr << "Error opening archive: " << archive_error_string(a) << std::endl;
        return false;
    }

    struct archive_entry* entry;
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        const char* entry_pathname_c = archive_entry_pathname(entry);
        if (entry_pathname_c == nullptr) continue;

        std::filesystem::path entry_path(entry_pathname_c);
        // Build the full destination path and normalize it to resolve "." and ".." lexically
        std::filesystem::path full_dest_path = (canon_dest_dir / entry_path).lexically_normal();

        // Security Check: Ensure the normalized path is still within our canonical destination directory.
        auto res = std::mismatch(canon_dest_dir.begin(), canon_dest_dir.end(), full_dest_path.begin());
        if (res.first != canon_dest_dir.end()) {
             std::cerr << "Error: Attempted path traversal in tar file: " << entry_pathname_c << std::endl;
             return false;
        }

        mode_t file_type = archive_entry_filetype(entry);
        if (S_ISDIR(file_type)) {
            std::filesystem::create_directories(full_dest_path, ec);
            if (ec) {
                std::cerr << "Error creating directory: " << full_dest_path << " : " << ec.message() << std::endl;
                return false;
            }
        } else if (S_ISREG(file_type)) {
            std::filesystem::path parent_path = full_dest_path.parent_path();
            if (!std::filesystem::exists(parent_path)) {
                std::filesystem::create_directories(parent_path, ec);
                if (ec) {
                     std::cerr << "Error creating parent directory: " << parent_path << " : " << ec.message() << std::endl;
                     return false;
                }
            }
            
            int fd = open(full_dest_path.c_str(), O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
            if (fd < 0) {
                std::cerr << "Error opening file for writing: " << full_dest_path << std::endl;
                return false;
            }

            const void* buff;
            size_t size;
            la_int64_t offset;
            bool success = true;
            while (archive_read_data_block(a, &buff, &size, &offset) == ARCHIVE_OK) {
                if (write(fd, buff, size) != (ssize_t)size) {
                    std::cerr << "Error writing to file: " << full_dest_path << std::endl;
                    success = false;
                    break;
                }
            }
            close(fd);
            if (!success) return false;

        } else {
            // Silently ignore other file types (symlinks, etc.) for security
            std::cout << "Skipping non-regular file/directory: " << entry_pathname_c << std::endl;
        }
    }

    return true;
}

// Helper to create test directories and files for tar creation
void setup_test_files(const std::filesystem::path& base_dir, const std::string& structure) {
    if (structure == "simple") {
        std::filesystem::create_directory(base_dir / "dir1");
        std::ofstream(base_dir / "file1.txt") << "content1";
        std::ofstream(base_dir / "dir1" / "file2.txt") << "content2";
    } else if (structure == "simple2") {
        std::ofstream(base_dir / "file3.txt") << "content3";
    }
}

void create_test_archives(const std::filesystem::path& temp_dir) {
    std::filesystem::path creation_dir = temp_dir / "creation";
    std::filesystem::create_directory(creation_dir);

    // Test 1: simple .tar.gz
    setup_test_files(creation_dir, "simple");
    std::string cmd1 = "tar -czf " + (temp_dir / "test1.tar.gz").string() + " -C " + creation_dir.string() + " .";
    system(cmd1.c_str());
    std::filesystem::remove_all(creation_dir);
    std::filesystem::create_directory(creation_dir);

    // Test 2: simple .tar.bz2
    setup_test_files(creation_dir, "simple2");
    std::string cmd2 = "tar -cjf " + (temp_dir / "test2.tar.bz2").string() + " -C " + creation_dir.string() + " .";
    system(cmd2.c_str());
    std::filesystem::remove_all(creation_dir);
    std::filesystem::create_directory(creation_dir);
    
    // Test 3 & 4 & 5 (malicious) - need libarchive to create them properly. A simple tar command will do.
    std::string cmd3 = "tar -czf " + (temp_dir / "test3_traversal.tar.gz").string() + " --transform 's|pwned.txt|../../../pwned.txt|' -C " + creation_dir.string() + " .";
    std::ofstream(creation_dir / "pwned.txt") << "malicious";
    system(cmd3.c_str());

    std::string cmd4 = "tar -czf " + (temp_dir / "test4_absolute.tar.gz").string() + " --transform 's|absolute_pwned.txt|/tmp/absolute_pwned.txt|' -C " + creation_dir.string() + " .";
    std::ofstream(creation_dir / "absolute_pwned.txt") << "malicious";
    system(cmd4.c_str());

    std::string cmd5 = "ln -s file1.txt " + (creation_dir / "link_to_file1").string() + "; tar -czf " + (temp_dir / "test5_symlink.tar.gz").string() + " -C " + creation_dir.string() + " link_to_file1";
    system(cmd5.c_str());
    
    std::filesystem::remove_all(creation_dir);
}


int main() {
    char archives_dir_template[] = "/tmp/cpp_tar_tests_XXXXXX";
    char* archives_dir_name = mkdtemp(archives_dir_template);
    if (archives_dir_name == nullptr) {
        std::cerr << "Failed to create temp directory for archives." << std::endl;
        return 1;
    }
    const std::filesystem::path test_archives_dir(archives_dir_name);

    std::cout << "Setting up test archives in: " << test_archives_dir << std::endl;
    create_test_archives(test_archives_dir);
    
    // Test Case 1: Simple gzip tar
    char unpack_dir1_template[] = "/tmp/cpp_unpack1_XXXXXX";
    const std::filesystem::path temp_dir1(mkdtemp(unpack_dir1_template));
    std::cout << "\n--- Test Case 1: Simple .tar.gz ---" << std::endl;
    bool success1 = secure_extract_tar(test_archives_dir / "test1.tar.gz", temp_dir1);
    std::cout << "Extraction success: " << std::boolalpha << success1 << std::endl;
    std::cout << "Check file1.txt exists: " << std::filesystem::exists(temp_dir1 / "file1.txt") << std::endl;
    std::cout << "Check dir1/file2.txt exists: " << std::filesystem::exists(temp_dir1 / "dir1/file2.txt") << std::endl;

    // Test Case 2: Simple bzip2 tar
    char unpack_dir2_template[] = "/tmp/cpp_unpack2_XXXXXX";
    const std::filesystem::path temp_dir2(mkdtemp(unpack_dir2_template));
    std::cout << "\n--- Test Case 2: Simple .tar.bz2 ---" << std::endl;
    bool success2 = secure_extract_tar(test_archives_dir / "test2.tar.bz2", temp_dir2);
    std::cout << "Extraction success: " << std::boolalpha << success2 << std::endl;
    std::cout << "Check file3.txt exists: " << std::filesystem::exists(temp_dir2 / "file3.txt") << std::endl;

    // Test Case 3: Path Traversal
    char unpack_dir3_template[] = "/tmp/cpp_unpack3_XXXXXX";
    const std::filesystem::path temp_dir3(mkdtemp(unpack_dir3_template));
    std::cout << "\n--- Test Case 3: Path Traversal (should fail) ---" << std::endl;
    bool success3 = secure_extract_tar(test_archives_dir / "test3_traversal.tar.gz", temp_dir3);
    std::cout << "Extraction success: " << std::boolalpha << success3 << std::endl;
    std::filesystem::path pwned_file = std::filesystem::canonical(temp_dir3 / ".." / "pwned.txt");
    std::cout << "Check for traversal file " << pwned_file << " exists: " << std::filesystem::exists(pwned_file) << std::endl;
    
    // Test Case 4: Absolute Path
    char unpack_dir4_template[] = "/tmp/cpp_unpack4_XXXXXX";
    const std::filesystem::path temp_dir4(mkdtemp(unpack_dir4_template));
    std::cout << "\n--- Test Case 4: Absolute Path (should fail) ---" << std::endl;
    bool success4 = secure_extract_tar(test_archives_dir / "test4_absolute.tar.gz", temp_dir4);
    std::cout << "Extraction success: " << std::boolalpha << success4 << std::endl;
    std::cout << "Check for absolute file /tmp/absolute_pwned.txt exists: " << std::filesystem::exists("/tmp/absolute_pwned.txt") << std::endl;
    if (std::filesystem::exists("/tmp/absolute_pwned.txt")) std::filesystem::remove("/tmp/absolute_pwned.txt");

    // Test Case 5: Symlink
    char unpack_dir5_template[] = "/tmp/cpp_unpack5_XXXXXX";
    const std::filesystem::path temp_dir5(mkdtemp(unpack_dir5_template));
    std::cout << "\n--- Test Case 5: Symlink (should be skipped) ---" << std::endl;
    bool success5 = secure_extract_tar(test_archives_dir / "test5_symlink.tar.gz", temp_dir5);
    std::cout << "Extraction success: " << std::boolalpha << success5 << std::endl;
    std::cout << "Check symlink exists: " << std::filesystem::is_symlink(temp_dir5 / "link_to_file1") << std::endl;

    // Cleanup
    std::filesystem::remove_all(temp_dir1);
    std::filesystem::remove_all(temp_dir2);
    std::filesystem::remove_all(temp_dir3);
    std::filesystem::remove_all(temp_dir4);
    std::filesystem::remove_all(temp_dir5);
    std::filesystem::remove_all(test_archives_dir);
    std::cout << "\nCleanup complete." << std::endl;

    return 0;
}