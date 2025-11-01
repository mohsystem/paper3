#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

int open_file_with_flags(const char* path, int create, int trunc, int excl, int write_only) {
    int flags = 0;

    // Access mode
    if (trunc || write_only) {
        flags |= O_WRONLY;
    } else {
        flags |= O_RDONLY;
    }

    // Creation and other flags
    if (create) flags |= O_CREAT;
    if (trunc)  flags |= O_TRUNC;
    if (excl && create) flags |= O_EXCL;

    mode_t mode = 0644;
    int fd = open(path, flags, mode);
    if (fd == -1) {
        return -errno;
    }
    close(fd);
    return 0;
}

void prepare_file(const char* path, int create) {
    if (!create) {
        unlink(path);
        return;
    }
    int fd = open(path, O_WRONLY | O_CREAT, 0644);
    if (fd != -1) {
        const char* data = "seed";
        write(fd, data, 4);
        close(fd);
    }
}

int main() {
    // Test 1: Create new file with O_CREAT | O_WRONLY
    const char* f1 = "c_t1_create.txt";
    prepare_file(f1, 0);
    printf("T1: %d\n", open_file_with_flags(f1, 1, 0, 0, 1));

    // Test 2: Truncate existing file with O_TRUNC | O_WRONLY
    const char* f2 = "c_t2_trunc.txt";
    prepare_file(f2, 1);
    printf("T2: %d\n", open_file_with_flags(f2, 0, 1, 0, 1));

    // Test 3: O_CREAT | O_EXCL on existing file should fail
    const char* f3 = "c_t3_excl.txt";
    prepare_file(f3, 1);
    printf("T3: %d\n", open_file_with_flags(f3, 1, 0, 1, 1));

    // Test 4: Open non-existent file without O_CREAT should fail
    const char* f4 = "c_t4_no_create.txt";
    prepare_file(f4, 0);
    printf("T4: %d\n", open_file_with_flags(f4, 0, 0, 0, 1));

    // Test 5: Open existing file O_WRONLY
    const char* f5 = "c_t5_open_existing.txt";
    prepare_file(f5, 1);
    printf("T5: %d\n", open_file_with_flags(f5, 0, 0, 0, 1));

    return 0;
}