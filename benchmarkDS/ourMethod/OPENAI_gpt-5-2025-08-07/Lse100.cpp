#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

static int create_important_config(const std::string &base_dir) {
    if (base_dir.empty()) {
        return -1;
    }

    // Open the base directory without following symlinks
    int dir_flags = O_RDONLY;
#ifdef O_DIRECTORY
    dir_flags |= O_DIRECTORY;
#endif
#ifdef O_CLOEXEC
    dir_flags |= O_CLOEXEC;
#endif
#ifdef O_NOFOLLOW
    dir_flags |= O_NOFOLLOW;
#endif

    int dirfd = open(base_dir.c_str(), dir_flags);
    if (dirfd < 0) {
        // ELOOP often indicates a symlink encountered with O_NOFOLLOW
        return -2;
    }

    struct stat dst;
    if (fstat(dirfd, &dst) != 0) {
        close(dirfd);
        return -3;
    }
    if (!S_ISDIR(dst.st_mode)) {
        close(dirfd);
        return -4;
    }

    const char *filename = "important_config";
    int flags = O_WRONLY | O_CREAT | O_EXCL;
#ifdef O_CLOEXEC
    flags |= O_CLOEXEC;
#endif
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif

    int fd = openat(dirfd, filename, flags, 0600);
    if (fd < 0) {
        int e = errno;
        close(dirfd);
        if (e == EEXIST) {
            return 1;
        }
        if (e == ELOOP) {
            return -5;
        }
        return -6;
    }

    struct stat fst;
    if (fstat(fd, &fst) != 0) {
        int saved = errno;
        close(fd);
        close(dirfd);
        errno = saved;
        return -7;
    }
    if (!S_ISREG(fst.st_mode)) {
        close(fd);
        close(dirfd);
        return -8;
    }

    // Persist metadata
    (void)fsync(fd);
    close(fd);

    // Best-effort fsync of directory
    (void)fsync(dirfd);
    close(dirfd);

    return 0;
}

static void print_result(int r) {
    std::cout << "Result: " << r << std::endl;
}

#include <cstdlib>
#include <cstdio>

static std::string make_temp_dir(const std::string &prefix) {
    std::string tmpl = prefix + "XXXXXX";
    std::vector<char> buf(tmpl.begin(), tmpl.end());
    buf.push_back('\0');
    char *res = mkdtemp(buf.data());
    if (!res) {
        return "";
    }
    return std::string(res);
}

static bool make_dir(const std::string &path) {
    if (mkdir(path.c_str(), 0700) == 0) return true;
    if (errno == EEXIST) return true;
    return false;
}

int main() {
    std::string base = make_temp_dir("/tmp/secure_base_cpp_");
    if (base.empty()) {
        std::cout << "Setup error" << std::endl;
        return 0;
    }
    // Test 1
    print_result(create_important_config(base));
    // Test 2 (exists)
    print_result(create_important_config(base));
    // Test 3 nestedA
    std::string nestedA = base + "/nestedA";
    make_dir(nestedA);
    print_result(create_important_config(nestedA));
    // Test 4 nestedB
    std::string nestedB = base + "/nestedB";
    make_dir(nestedB);
    print_result(create_important_config(nestedB));
    // Test 5: symlink to nestedA (best-effort)
#if defined(__unix__) || defined(__APPLE__)
    std::string linkPath = base + "/link_to_nestedA";
    unlink(linkPath.c_str());
    if (symlink(nestedA.c_str(), linkPath.c_str()) == 0) {
        print_result(create_important_config(linkPath));
    } else {
        print_result(create_important_config(base));
    }
#else
    print_result(create_important_config(base));
#endif
    return 0;
}