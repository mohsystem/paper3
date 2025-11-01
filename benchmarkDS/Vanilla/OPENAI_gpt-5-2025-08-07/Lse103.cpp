#include <iostream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

int open_file_with_flags(const std::string& path, bool create, bool trunc, bool excl, bool write_only) {
    int flags = 0;

    // Access mode
    if (trunc || write_only) {
        flags |= O_WRONLY;
    } else {
        flags |= O_RDONLY;
    }

    // Creation and control flags
    if (create) flags |= O_CREAT;
    if (trunc)  flags |= O_TRUNC;
    if (excl && create) flags |= O_EXCL;

    mode_t mode = 0644;
    int fd = open(path.c_str(), flags, mode);
    if (fd == -1) {
        return -errno;
    }
    close(fd);
    return 0;
}

void prepare_file(const std::string& path, bool create) {
    if (!create) {
        unlink(path.c_str());
        return;
    }
    int fd = open(path.c_str(), O_WRONLY | O_CREAT, 0644);
    if (fd != -1) {
        const char* data = "seed";
        write(fd, data, 4);
        close(fd);
    }
}

int main() {
    // Test 1: Create new file with O_CREAT | O_WRONLY
    std::string f1 = "cpp_t1_create.txt";
    prepare_file(f1, false);
    std::cout << "T1: " << open_file_with_flags(f1, true, false, false, true) << std::endl;

    // Test 2: Truncate existing file with O_TRUNC | O_WRONLY
    std::string f2 = "cpp_t2_trunc.txt";
    prepare_file(f2, true);
    std::cout << "T2: " << open_file_with_flags(f2, false, true, false, true) << std::endl;

    // Test 3: O_CREAT | O_EXCL on existing file should fail
    std::string f3 = "cpp_t3_excl.txt";
    prepare_file(f3, true);
    std::cout << "T3: " << open_file_with_flags(f3, true, false, true, true) << std::endl;

    // Test 4: Open non-existent file without O_CREAT should fail
    std::string f4 = "cpp_t4_no_create.txt";
    prepare_file(f4, false);
    std::cout << "T4: " << open_file_with_flags(f4, false, false, false, true) << std::endl;

    // Test 5: Open existing file O_WRONLY
    std::string f5 = "cpp_t5_open_existing.txt";
    prepare_file(f5, true);
    std::cout << "T5: " << open_file_with_flags(f5, false, false, false, true) << std::endl;

    return 0;
}