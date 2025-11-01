/*
 * This code uses the libzip library.
 * To compile, you need to install libzip and link against it.
 *
 * Example compilation command on Linux:
 * g++ -std=c++17 -o Lse50 Lse50.cpp -lzip
 *
 * On macOS (with Homebrew):
 * brew install libzip
 * g++ -std=c++17 -o Lse50 Lse50.cpp -L/usr/local/opt/libzip/lib -I/usr/local/opt/libzip/include -lzip
 */

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <zip.h>
#include <memory>
#include <system_error>

namespace fs = std::filesystem;

/**
 * Creates a directory and all its parent directories if they don't exist.
 *
 * @param path The directory path to create.
 * @return true on success, false on failure.
 */
bool create_directories_recursively(const fs::path& path) {
    std::error_code ec;
    fs::create_directories(path, ec);
    if (ec) {
        std::cerr << "Error: Could not create directory " << path << ": " << ec.message() << std::endl;
        return false;
    }
    return true;
}

/**
 * Extracts a zip archive to a destination directory, preventing path traversal.
 *
 * @param zipFilePath The path to the zip file.
 * @param destDirPath The directory to extract files into.
 * @return true if successful, false otherwise.
 */
bool extractArchive(const std::string& zipFilePath, const std::string& destDirPath) {
    fs::path destDir(destDirPath);
    fs::path zipFile(zipFilePath);

    if (!fs::exists(zipFile)) {
        std::cerr << "Error: ZIP file not found at " << zipFilePath << std::endl;
        return false;
    }

    if (!create_directories_recursively(destDir)) {
        return false;
    }

    std::error_code ec;
    fs::path canonicalDestDir = fs::weakly_canonical(destDir, ec);
    if (ec) {
        std::cerr << "Error: Could not get canonical path for destination directory: " << ec.message() << std::endl;
        return false;
    }

    int err = 0;
    zip* za = zip_open(zipFilePath.c_str(), 0, &err);
    if (!za) {
        zip_error_t ziperror;
        zip_error_init_with_code(&ziperror, err);
        std::cerr << "Error: Failed to open zip archive '" << zipFilePath << "': " << zip_error_strerror(&ziperror) << std::endl;
        zip_error_fini(&ziperror);
        return false;
    }
    // Custom deleter for unique_ptr to handle zip*
    auto zip_deleter = [](zip* z) { if (z) zip_close(z); };
    std::unique_ptr<zip, decltype(zip_deleter)> zip_archive(za, zip_deleter);

    zip_int64_t num_entries = zip_get_num_entries(zip_archive.get(), 0);
    if (num_entries < 0) {
        std::cerr << "Error: Failed to get number of entries: " << zip_strerror(zip_archive.get()) << std::endl;
        return false;
    }

    for (zip_int64_t i = 0; i < num_entries; ++i) {
        struct zip_stat sb;
        if (zip_stat_index(zip_archive.get(), i, 0, &sb) != 0) {
            std::cerr << "Error: Failed to stat entry at index " << i << ": " << zip_strerror(zip_archive.get()) << std::endl;
            continue;
        }

        fs::path entry_path(sb.name);
        fs::path full_dest_path = destDir / entry_path;
        
        fs::path canonical_full_path = fs::weakly_canonical(full_dest_path, ec);
        if (ec) {
             std::cerr << "Error: could not get canonical path for " << full_dest_path << ": " << ec.message() << std::endl;
             continue;
        }
        
        // Security check: ensure the destination path is within the destination directory
        auto res = std::mismatch(canonicalDestDir.begin(), canonicalDestDir.end(), canonical_full_path.begin());
        if (res.first != canonicalDestDir.end()) {
             std::cerr << "Path traversal attempt detected for entry: " << sb.name << ". Skipping." << std::endl;
             continue;
        }

        bool is_dir = (sb.name[strlen(sb.name) - 1] == '/');

        if (is_dir) {
            if (!create_directories_recursively(canonical_full_path)) {
                continue;
            }
        } else {
            if (!create_directories_recursively(canonical_full_path.parent_path())) {
                continue;
            }

            zip_file* zf = zip_fopen_index(zip_archive.get(), i, 0);
            if (!zf) {
                std::cerr << "Error: Failed to open file in zip: " << sb.name << std::endl;
                continue;
            }
            auto zip_file_deleter = [](zip_file* zfile){ if(zfile) zip_fclose(zfile); };
            std::unique_ptr<zip_file, decltype(zip_file_deleter)> zip_file_ptr(zf, zip_file_deleter);

            std::ofstream ofs(canonical_full_path, std::ios::binary);
            if (!ofs) {
                std::cerr << "Error: Failed to open output file " << canonical_full_path << std::endl;
                continue;
            }

            char buffer[4096];
            zip_int64_t n;
            while ((n = zip_fread(zip_file_ptr.get(), buffer, sizeof(buffer))) > 0) {
                ofs.write(buffer, n);
            }
        }
    }
    return true;
}

/**
 * Creates a test zip file for extraction tests.
 */
bool createTestZip(const std::string& zipFilePath) {
    int error = 0;
    zip* za = zip_open(zipFilePath.c_str(), ZIP_CREATE | ZIP_EXCL, &error);
    if (!za) {
        zip_error_t ziperror;
        zip_error_init_with_code(&ziperror, error);
        std::cerr << "Failed to create zip '" << zipFilePath << "': " << zip_error_strerror(&ziperror) << std::endl;
        zip_error_fini(&ziperror);
        return false;
    }
    auto zip_deleter = [](zip* z) { if (z) zip_close(z); };
    std::unique_ptr<zip, decltype(zip_deleter)> zip_archive(za, zip_deleter);

    const char* content1 = "This is a test file.";
    const char* content2 = "This is a nested file.";
    const char* content3 = "This should not be extracted.";
    const char* content4 = "This is another safe file.";

    struct zip_source* s;
    if ((s = zip_source_buffer(zip_archive.get(), content1, strlen(content1), 0)) == nullptr ||
        zip_file_add(zip_archive.get(), "test1.txt", s, ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8) < 0) {
        zip_source_free(s);
        return false;
    }
    if ((s = zip_source_buffer(zip_archive.get(), content2, strlen(content2), 0)) == nullptr ||
        zip_file_add(zip_archive.get(), "dir1/test2.txt", s, ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8) < 0) {
        zip_source_free(s);
        return false;
    }
    if ((s = zip_source_buffer(zip_archive.get(), content3, strlen(content3), 0)) == nullptr ||
        zip_file_add(zip_archive.get(), "../evil.txt", s, ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8) < 0) {
        zip_source_free(s);
        return false;
    }
    if ((s = zip_source_buffer(zip_archive.get(), content3, strlen(content3), 0)) == nullptr ||
        zip_file_add(zip_archive.get(), "/tmp/absolute.txt", s, ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8) < 0) {
        zip_source_free(s);
        return false;
    }
    if ((s = zip_source_buffer(zip_archive.get(), content4, strlen(content4), 0)) == nullptr ||
        zip_file_add(zip_archive.get(), "test3.txt", s, ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8) < 0) {
        zip_source_free(s);
        return false;
    }
    if (zip_dir_add(zip_archive.get(), "dir1/", ZIP_FL_ENC_UTF_8) < 0) return false;

    return true;
}

int main() {
    const std::string zipFileName = "archive.zip";
    const std::string unpackDir = "/tmp/unpack_cpp";

    // --- Test Case 1: Standard Extraction ---
    std::cout << "--- Test Case 1: Standard Extraction ---" << std::endl;
    if (createTestZip(zipFileName)) {
        std::cout << "Created test zip: " << zipFileName << std::endl;
        bool success = extractArchive(zipFileName, unpackDir);
        std::cout << "Extraction successful: " << std::boolalpha << success << std::endl;

        // Verify results
        std::cout << "Exists '" << unpackDir << "/test1.txt': " << fs::exists(fs::path(unpackDir) / "test1.txt") << std::endl;
        std::cout << "Exists '" << unpackDir << "/dir1/test2.txt': " << fs::exists(fs::path(unpackDir) / "dir1" / "test2.txt") << std::endl;
        std::cout << "Exists '" << unpackDir << "/test3.txt': " << fs::exists(fs::path(unpackDir) / "test3.txt") << std::endl;
        std::cout << "Exists '/tmp/evil.txt': " << fs::exists("/tmp/evil.txt") << std::endl;
        std::cout << "Exists '/tmp/absolute.txt': " << fs::exists("/tmp/absolute.txt") << std::endl;
    } else {
        std::cerr << "Failed to create test zip." << std::endl;
    }
    
    // Cleanup
    std::error_code ec;
    fs::remove_all(unpackDir, ec);
    fs::remove(zipFileName, ec);
    fs::remove("/tmp/evil.txt", ec);
    fs::remove("/tmp/absolute.txt", ec);
    std::cout << "Cleaned up test files and directories." << std::endl << std::endl;

    // --- Test Case 2: Non-existent ZIP file ---
    std::cout << "--- Test Case 2: Non-existent ZIP file ---" << std::endl;
    bool success2 = extractArchive("nonexistent.zip", unpackDir);
    std::cout << "Extraction successful: " << std::boolalpha << success2 << std::endl << std::endl;

    // --- Test Case 3: Destination is a file (should fail) ---
    std::cout << "--- Test Case 3: Destination is a file ---" << std::endl;
    const std::string dummyFile = "dummy_file.txt";
    { std::ofstream ofs(dummyFile); ofs << "dummy"; }
    if (createTestZip(zipFileName)) {
        bool success3 = extractArchive(zipFileName, dummyFile);
        std::cout << "Extraction successful: " << std::boolalpha << success3 << std::endl;
    }
    fs::remove(dummyFile, ec);
    fs::remove(zipFileName, ec);
    std::cout << "Cleaned up." << std::endl << std::endl;

    std::cout << "--- Test Cases 4 & 5: Path Traversal and Absolute Paths are handled in Test 1 ---" << std::endl;
    return 0;
}