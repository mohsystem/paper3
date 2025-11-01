// To compile: g++ -std=c++17 main.cpp -lzip -o unzip_app
#include <iostream>
#include <string>
#include <vector>
#include <zip.h>
#include <filesystem>
#include <fstream>
#include <system_error>

namespace fs = std::filesystem;

bool unzipArchive(const std::string& archivePath, const std::string& extractDir) {
    fs::path archive_fs_path(archivePath);
    if (!fs::exists(archive_fs_path) || !fs::is_regular_file(archive_fs_path)) {
        std::cerr << "Error: Archive file not found or is not a regular file: " << archivePath << std::endl;
        return false;
    }

    fs::path extract_fs_path(extractDir);
    std::error_code ec;
    fs::create_directories(extract_fs_path, ec);
    if (ec) {
        std::cerr << "Error: Cannot create extraction directory " << extractDir << ": " << ec.message() << std::endl;
        return false;
    }
    
    fs::path canonical_extract_dir = fs::weakly_canonical(extract_fs_path, ec);
    if (ec) {
        std::cerr << "Error: Cannot get canonical path for " << extractDir << ": " << ec.message() << std::endl;
        return false;
    }

    int err = 0;
    zip* za = zip_open(archivePath.c_str(), 0, &err);
    if (!za) {
        zip_error_t ziperror;
        zip_error_init_with_code(&ziperror, err);
        std::cerr << "Error: Failed to open archive " << archivePath << ": " << zip_error_strerror(&ziperror) << std::endl;
        zip_error_fini(&ziperror);
        return false;
    }

    zip_int64_t num_entries = zip_get_num_entries(za, 0);
    if (num_entries < 0) {
        std::cerr << "Error: Failed to get number of entries from " << archivePath << std::endl;
        zip_close(za);
        return false;
    }

    for (zip_int64_t i = 0; i < num_entries; i++) {
        const char* name = zip_get_name(za, i, 0);
        if (name == NULL) {
            std::cerr << "Error: Failed to get name of entry " << i << std::endl;
            continue;
        }

        fs::path dest_path = extract_fs_path / name;
        fs::path canonical_dest_path = fs::weakly_canonical(dest_path, ec);
        if (ec) {
            std::cerr << "Error: could not form canonical path for " << name << ": " << ec.message() << std::endl;
            zip_close(za);
            return false;
        }
        
        std::string canon_dest_str = canonical_dest_path.string();
        std::string canon_base_str = canonical_extract_dir.string();

        if (canon_dest_str.rfind(canon_base_str, 0) != 0) {
            std::cerr << "Error: Path traversal attempt detected for entry '" << name << "'" << std::endl;
            zip_close(za);
            return false;
        }
        
        if (std::string(name).back() == '/') {
            fs::create_directories(canonical_dest_path, ec);
            if (ec) {
                 std::cerr << "Error: Could not create directory " << canonical_dest_path << ": " << ec.message() << std::endl;
            }
        } else {
            zip_file* zf = zip_fopen_index(za, i, 0);
            if (!zf) {
                std::cerr << "Error: Failed to open file in archive: " << name << std::endl;
                continue;
            }
            
            fs::path parent_path = canonical_dest_path.parent_path();
            if (!fs::exists(parent_path)) {
                fs::create_directories(parent_path, ec);
                if (ec) {
                     std::cerr << "Error: Could not create parent directory " << parent_path << ": " << ec.message() << std::endl;
                     zip_fclose(zf);
                     continue;
                }
            }

            std::ofstream ofs(canonical_dest_path, std::ios::binary);
            if (!ofs.is_open()) {
                 std::cerr << "Error: Could not open output file " << canonical_dest_path << std::endl;
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

    zip_close(za);
    return true;
}

bool createTestZip(const std::string& zipPath, const std::vector<std::pair<std::string, std::string>>& files, bool isMalicious) {
    int error = 0;
    zip* za = zip_open(zipPath.c_str(), ZIP_CREATE | ZIP_TRUNCATE, &error);
    if (!za) return false;

    for (size_t i = 0; i < files.size(); ++i) {
        std::string name = files[i].first;
        const std::string& content = files[i].second;

        if (isMalicious && i == files.size() - 1) {
            name = "../" + name;
        }

        zip_source_t* source = zip_source_buffer(za, content.c_str(), content.length(), 0);
        if (!source) { zip_close(za); return false; }

        if (zip_file_add(za, name.c_str(), source, ZIP_FL_OVERWRITE) < 0) {
            zip_source_free(source);
            zip_close(za);
            return false;
        }
    }
    zip_close(za);
    return true;
}


int main() {
    fs::path baseTmpDir = fs::temp_directory_path();

    // Test Case 1: Valid archive
    std::cout << "--- Test Case 1: Valid Archive ---" << std::endl;
    fs::path archivePath1 = baseTmpDir / "archive1.zip";
    fs::path unpackDir1 = baseTmpDir / "unpack1";
    createTestZip(archivePath1.string(), {{"file1.txt", "content1"}, {"dir1/file2.txt", "content2"}}, false);
    std::cout << "Unzipping " << archivePath1 << " to " << unpackDir1 << std::endl;
    if (unzipArchive(archivePath1.string(), unpackDir1.string())) {
        std::cout << "Test Case 1: Success." << std::endl;
    } else {
        std::cerr << "Test Case 1: Failed." << std::endl;
    }
    fs::remove_all(unpackDir1);
    fs::remove(archivePath1);

    // Test Case 2: Malicious archive (path traversal)
    std::cout << "\n--- Test Case 2: Malicious Archive (Path Traversal) ---" << std::endl;
    fs::path archivePath2 = baseTmpDir / "malicious.zip";
    fs::path unpackDir2 = baseTmpDir / "unpack2";
    createTestZip(archivePath2.string(), {{"good.txt", "good"}, {"evil.txt", "evil"}}, true);
    std::cout << "Unzipping " << archivePath2 << " to " << unpackDir2 << std::endl;
    if (!unzipArchive(archivePath2.string(), unpackDir2.string())) {
        std::cout << "Test Case 2: Success (correctly identified traversal)." << std::endl;
    } else {
        std::cerr << "Test Case 2: Failed (did not block traversal)." << std::endl;
    }
    fs::remove_all(unpackDir2);
    fs::remove(archivePath2);

    // Test Case 3: Non-existent archive
    std::cout << "\n--- Test Case 3: Non-existent Archive ---" << std::endl;
    fs::path archivePath3 = baseTmpDir / "nonexistent.zip";
    fs::path unpackDir3 = baseTmpDir / "unpack3";
    std::cout << "Unzipping " << archivePath3 << " to " << unpackDir3 << std::endl;
    if (!unzipArchive(archivePath3.string(), unpackDir3.string())) {
        std::cout << "Test Case 3: Success (correctly handled error)." << std::endl;
    } else {
        std::cerr << "Test Case 3: Failed." << std::endl;
    }

    // Test Case 4: No permission directory
    std::cout << "\n--- Test Case 4: No Permission Directory ---" << std::endl;
    fs::path archivePath4 = baseTmpDir / "archive4.zip";
    fs::path unpackDir4 = "/root/unpack4";
    createTestZip(archivePath4.string(), {{"file1.txt", "content"}}, false);
    std::cout << "Unzipping " << archivePath4 << " to " << unpackDir4 << std::endl;
    if (!unzipArchive(archivePath4.string(), unpackDir4.string())) {
        std::cout << "Test Case 4: Success (correctly handled permission error)." << std::endl;
    } else {
        std::cerr << "Test Case 4: Failed or was skipped." << std::endl;
    }
    fs::remove(archivePath4);

    // Test Case 5: Empty archive
    std::cout << "\n--- Test Case 5: Empty Archive ---" << std::endl;
    fs::path archivePath5 = baseTmpDir / "empty.zip";
    fs::path unpackDir5 = baseTmpDir / "unpack5";
    createTestZip(archivePath5.string(), {}, false);
    std::cout << "Unzipping " << archivePath5 << " to " << unpackDir5 << std::endl;
    if (unzipArchive(archivePath5.string(), unpackDir5.string())) {
        std::cout << "Test Case 5: Success." << std::endl;
    } else {
        std::cerr << "Test Case 5: Failed." << std::endl;
    }
    fs::remove_all(unpackDir5);
    fs::remove(archivePath5);

    return 0;
}