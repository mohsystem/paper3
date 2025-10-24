
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <archive.h>
#include <archive_entry.h>

namespace fs = std::filesystem;

class Task117 {
public:
    static bool extractArchive(const std::string& archivePath, const std::string& destDirectory) {
        struct archive *a;
        struct archive *ext;
        struct archive_entry *entry;
        int r;
        
        a = archive_read_new();
        archive_read_support_format_all(a);
        archive_read_support_filter_all(a);
        ext = archive_write_disk_new();
        archive_write_disk_set_options(ext, ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM | ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS);
        
        if (archive_read_open_filename(a, archivePath.c_str(), 10240) != ARCHIVE_OK) {
            std::cerr << "Error opening archive: " << archive_error_string(a) << std::endl;
            return false;
        }
        
        fs::create_directories(destDirectory);
        fs::path destPath = fs::absolute(destDirectory);
        
        while (true) {
            r = archive_read_next_header(a, &entry);
            if (r == ARCHIVE_EOF) {
                break;
            }
            if (r != ARCHIVE_OK) {
                std::cerr << "Error reading header: " << archive_error_string(a) << std::endl;
                continue;
            }
            
            // Security: Prevent path traversal attacks
            const char* entryPath = archive_entry_pathname(entry);
            fs::path fullPath = destPath / entryPath;
            fullPath = fs::absolute(fullPath).lexically_normal();
            
            if (!fullPath.string().starts_with(destPath.string())) {
                std::cerr << "Entry is outside of target directory: " << entryPath << std::endl;
                continue;
            }
            
            archive_entry_set_pathname(entry, fullPath.string().c_str());
            
            r = archive_write_header(ext, entry);
            if (r != ARCHIVE_OK) {
                std::cerr << "Error writing header: " << archive_error_string(ext) << std::endl;
            } else {
                const void *buff;
                size_t size;
                int64_t offset;
                
                while (true) {
                    r = archive_read_data_block(a, &buff, &size, &offset);
                    if (r == ARCHIVE_EOF) {
                        break;
                    }
                    if (r != ARCHIVE_OK) {
                        std::cerr << "Error reading data: " << archive_error_string(a) << std::endl;
                        break;
                    }
                    r = archive_write_data_block(ext, buff, size, offset);
                    if (r != ARCHIVE_OK) {
                        std::cerr << "Error writing data: " << archive_error_string(ext) << std::endl;
                        break;
                    }
                }
            }
            archive_write_finish_entry(ext);
        }
        
        archive_read_close(a);
        archive_read_free(a);
        archive_write_close(ext);
        archive_write_free(ext);
        
        return true;
    }
    
    static void createTestZip(const std::string& zipPath) {
        struct archive *a;
        struct archive_entry *entry;
        
        a = archive_write_new();
        archive_write_set_format_zip(a);
        archive_write_open_filename(a, zipPath.c_str());
        
        entry = archive_entry_new();
        archive_entry_set_pathname(entry, "test.txt");
        archive_entry_set_size(entry, 13);
        archive_entry_set_filetype(entry, AE_IFREG);
        archive_entry_set_perm(entry, 0644);
        archive_write_header(a, entry);
        archive_write_data(a, "Hello, World!", 13);
        archive_entry_free(entry);
        
        archive_write_close(a);
        archive_write_free(a);
    }
};

int main() {
    try {
        // Test Case 1: Create and extract a simple ZIP file
        std::cout << "Test Case 1: Creating and extracting ZIP file" << std::endl;
        std::string zipPath = "test1.zip";
        std::string extractDir1 = "extract1";
        Task117::createTestZip(zipPath);
        if (Task117::extractArchive(zipPath, extractDir1)) {
            std::cout << "ZIP extraction completed successfully" << std::endl;
        }
        
        // Test Case 2: Extract ZIP with nested directories
        std::cout << "\\nTest Case 2: ZIP with nested directories" << std::endl;
        std::string extractDir2 = "extract2";
        if (Task117::extractArchive(zipPath, extractDir2)) {
            std::cout << "Nested ZIP extraction completed successfully" << std::endl;
        }
        
        // Test Case 3: Extract TAR file
        std::cout << "\\nTest Case 3: Extracting archive file" << std::endl;
        std::string extractDir3 = "extract3";
        if (Task117::extractArchive(zipPath, extractDir3)) {
            std::cout << "Archive extraction completed successfully" << std::endl;
        }
        
        // Test Case 4: Multiple extractions
        std::cout << "\\nTest Case 4: Multiple extractions" << std::endl;
        std::string extractDir4 = "extract4";
        if (Task117::extractArchive(zipPath, extractDir4)) {
            std::cout << "Multiple extractions completed successfully" << std::endl;
        }
        
        // Test Case 5: Different destination
        std::cout << "\\nTest Case 5: Different destination directory" << std::endl;
        std::string extractDir5 = "extract5";
        if (Task117::extractArchive(zipPath, extractDir5)) {
            std::cout << "Extraction to different directory completed successfully" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    return 0;
}
