#include <iostream>
#include <string>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <cerrno>

bool secure_open_write(const std::string& path, bool create, bool trunc, bool wr_only, bool excl) {
    if (excl && !create) {
        return false;
    }

    int flags = 0;

#ifdef O_CLOEXEC
    flags |= O_CLOEXEC;
#endif
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif

    flags |= wr_only ? O_WRONLY : O_RDWR;

    if (create) flags |= O_CREAT;
    if (excl)   flags |= O_EXCL;
    if (trunc)  flags |= O_TRUNC;

    mode_t mode = S_IRUSR | S_IWUSR; // 0600
    int fd = open(path.c_str(), flags, mode);
    if (fd < 0) {
        return false;
    }

    struct stat st;
    if (fstat(fd, &st) != 0) {
        close(fd);
        return false;
    }
    if (!S_ISREG(st.st_mode)) {
        close(fd);
        return false;
    }

    close(fd);
    return true;
}

int main() {
    const char* dir = "lse103_tmp_cpp";
    mkdir(dir, 0700);

    std::string f1 = std::string(dir) + "/a.txt";
    std::string f2 = std::string(dir) + "/nonexist.txt";

    std::cout << (secure_open_write(f1, true,  false, true, true)  ? "true" : "false") << std::endl;
    std::cout << (secure_open_write(f1, true,  false, true, true)  ? "true" : "false") << std::endl;
    std::cout << (secure_open_write(f1, false, true,  true, false) ? "true" : "false") << std::endl;
    std::cout << (secure_open_write(f1, false, false, true, false) ? "true" : "false") << std::endl;
    std::cout << (secure_open_write(f2, false, false, true, false) ? "true" : "false") << std::endl;

    return 0;
}