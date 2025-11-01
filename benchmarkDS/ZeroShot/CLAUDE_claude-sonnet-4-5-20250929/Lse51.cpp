
#include <iostream>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <archive.h>
#include <archive_entry.h>
#include <cstring>

class Lse51 {
public:
    static bool extractTarArchive(const std::string& tarFilePath, const std::string& destDirectory) {
        struct archive *a;
        struct archive *ext;
        struct archive_entry *entry;
        int r;
        
        // Create destination directory
        mkdir(destDirectory.c_str(), 0755);
        
        a = archive_read_new();
        archive_read_support_format_tar(a);
        archive_read_support_filter_gzip(a);
        archive_read_support_filter_bzip2(a);
        archive_read_support_filter_none(a);
        
        ext = archive_write_disk_new();
        archive_write_disk_set_options(ext, ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM);
        
        if ((r = archive_read_open_filename(a, tarFilePath.c_str(), 10240))) {
            std::cerr << "Error opening archive: " << archive_error_string(a) << std::endl;
            return false;
        }
        
        while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
            std::string fullPath = destDirectory + "/" + archive_entry_pathname(entry);
            
            // Security check: prevent path traversal
            if (fullPath.find("..") != std::string::npos) {
                std::cerr << "Path traversal detected, skipping entry" << std::endl;
                continue;
            }
            
            archive_entry_set_pathname(entry, fullPath.c_str());
            
            r = archive_write_header(ext, entry);
            if (r == ARCHIVE_OK) {
                const void *buff;
                size_t size;
                int64_t offset;
                
                while ((r = archive_read_data_block(a, &buff, &size, &offset)) == ARCHIVE_OK) {
                    archive_write_data_block(ext, buff, size, offset);
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
};

int main() {
    std::string testFiles[] = {
        "test1.tar",
        "test2.tar.gz",
        "test3.tar.bz2",
        "test4.tgz",
        "test5.tar"
    };
    
    for (int i = 0; i < 5; i++) {
        if (Lse51::extractTarArchive(testFiles[i], "/tmp/unpack")) {
            std::cout << "Successfully extracted: " << testFiles[i] << std::endl;
        } else {
            std::cerr << "Failed to extract: " << testFiles[i] << std::endl;
        }
    }
    
    return 0;
}
