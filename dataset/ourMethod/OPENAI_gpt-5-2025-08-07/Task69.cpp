#include <iostream>
#include <string>
#include <vector>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>

// Utility: Split a relative path into components, rejecting '.', '..', empty segments, backslashes, and absolute paths
static bool split_relative_path(const std::string &relPath, std::vector<std::string> &out) {
    if (relPath.empty()) return false;
    if (relPath.size() > 4096) return false; // sanity limit
    if (relPath[0] == '/') return false;     // absolute path not allowed
    for (char c : relPath) {
        if (c == '\\' || c == '\0') return false; // reject Windows-style and NUL
    }
    std::string cur;
    for (size_t i = 0; i < relPath.size(); ++i) {
        char c = relPath[i];
        if (c == '/') {
            if (cur.empty()) return false; // reject empty component (e.g., "a//b")
            if (cur == "." || cur == "..") return false;
            if (cur.size() > 255) return false;
            out.push_back(cur);
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    if (cur.empty()) return false;
    if (cur == "." || cur == "..") return false;
    if (cur.size() > 255) return false;
    out.push_back(cur);
    return true;
}

// Open a directory component under parent dirfd without following symlinks
static int open_dir_component_nofollow(int parent_fd, const std::string &name) {
    int fd = openat(parent_fd, name.c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
    return fd; // caller checks for -1
}

// Ensure the final target is a regular file and not a symlink by opening with O_NOFOLLOW and fstat
static int verify_final_regular_file(int dirfd, const std::string &name) {
    int fd = openat(dirfd, name.c_str(), O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (fd == -1) {
        return -1; // ELOOP -> symlink, ENOENT, etc.
    }
    struct stat st;
    if (fstat(fd, &st) != 0) {
        int e = errno;
        close(fd);
        errno = e;
        return -1;
    }
    close(fd);
    if (!S_ISREG(st.st_mode)) {
        errno = EISDIR; // not a regular file
        return -1;
    }
    return 0;
}

// Secure delete: baseDir is a base directory; relPath is a relative path to the file under baseDir
// Returns 0 on success, -1 on failure with errno set.
int delete_file_secure(const std::string &baseDir, const std::string &relPath) {
    if (baseDir.empty() || baseDir.size() > 4096) {
        errno = EINVAL;
        return -1;
    }
    std::vector<std::string> parts;
    if (!split_relative_path(relPath, parts)) {
        errno = EINVAL;
        return -1;
    }

    // Open base directory securely
    int dirfd = open(baseDir.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (dirfd == -1) return -1;

    // Traverse all but the last component as directories with O_NOFOLLOW
    for (size_t i = 0; i + 1 < parts.size(); ++i) {
        int nextfd = open_dir_component_nofollow(dirfd, parts[i]);
        if (nextfd == -1) {
            int e = errno;
            close(dirfd);
            errno = e;
            return -1;
        }
        close(dirfd);
        dirfd = nextfd;
    }

    // Verify final is a regular file (not symlink, not directory)
    const std::string &finalName = parts.back();
    if (verify_final_regular_file(dirfd, finalName) != 0) {
        int e = errno;
        close(dirfd);
        errno = e;
        return -1;
    }

    // Attempt to delete using unlinkat relative to the opened parent directory
    if (unlinkat(dirfd, finalName.c_str(), 0) != 0) {
        int e = errno;
        close(dirfd);
        errno = e;
        return -1;
    }

    close(dirfd);
    return 0;
}

// Helpers for tests
static bool ensure_dir(const std::string &path) {
    if (mkdir(path.c_str(), 0700) == 0) return true;
    if (errno == EEXIST) {
        struct stat st{};
        if (stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) return true;
    }
    return false;
}

static bool ensure_file(const std::string &path) {
    int fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0600);
    if (fd == -1) return false;
    const char *msg = "x";
    ssize_t w = write(fd, msg, 1);
    int ok = (w == 1);
    close(fd);
    return ok;
}

static bool ensure_symlink(const std::string &target, const std::string &linkpath) {
    // Remove existing if any
    unlink(linkpath.c_str());
    return symlink(target.c_str(), linkpath.c_str()) == 0;
}

static void print_result(const std::string &name, int rc) {
    if (rc == 0) {
        std::cout << name << ": success\n";
    } else {
        std::cout << name << ": failure (" << strerror(errno) << ")\n";
    }
}

int main(int argc, char *argv[]) {
    if (argc >= 3) {
        std::string base = argv[1];
        std::string rel = argv[2];
        int rc = delete_file_secure(base, rel);
        if (rc == 0) {
            std::cout << "Deleted: " << rel << "\n";
            return 0;
        } else {
            std::cerr << "Error: " << strerror(errno) << "\n";
            return 1;
        }
    }

    // Self-tests (5 test cases)
    std::string base = "sandbox_cpp";
    ensure_dir(base);
    // Test 1: delete simple file
    ensure_file(base + "/a.txt");
    print_result("Test1 delete a.txt", delete_file_secure(base, "a.txt"));

    // Test 2: nested file
    ensure_dir(base + "/dir1");
    ensure_file(base + "/dir1/b.txt");
    print_result("Test2 delete dir1/b.txt", delete_file_secure(base, "dir1/b.txt"));

    // Test 3: reject symlink
    ensure_file(base + "/a.txt");
    ensure_symlink("a.txt", base + "/link_to_a");
    print_result("Test3 reject symlink link_to_a", delete_file_secure(base, "link_to_a"));

    // Test 4: reject path traversal
    print_result("Test4 reject '..' traversal", delete_file_secure(base, "../evil.txt"));

    // Test 5: reject directory deletion
    ensure_dir(base + "/dir1/subdir");
    print_result("Test5 reject directory", delete_file_secure(base, "dir1/subdir"));

    return 0;
}