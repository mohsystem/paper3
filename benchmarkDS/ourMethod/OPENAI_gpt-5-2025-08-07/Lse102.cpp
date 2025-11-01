#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <iostream>
#include <string>

#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif
#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif
#ifndef O_DIRECTORY
#define O_DIRECTORY 0
#endif

// Writes the string "important_config" into a file named "important_config" inside baseDir.
// Returns true on success.
bool write_important_config(const std::string& baseDir) {
    if (baseDir.empty()) {
        return false;
    }

    int dfd = open(baseDir.c_str(), O_RDONLY | O_CLOEXEC | O_NOFOLLOW | O_DIRECTORY);
    if (dfd < 0) {
        // Attempt without O_DIRECTORY for portability, validate after open
        dfd = open(baseDir.c_str(), O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
        if (dfd < 0) return false;
        struct stat dst {};
        if (fstat(dfd, &dst) != 0 || !S_ISDIR(dst.st_mode)) {
            close(dfd);
            return false;
        }
    }

    int fd = -1;
    fd = openat(dfd, "important_config", O_RDWR | O_CREAT | O_CLOEXEC | O_NOFOLLOW, 0644);
    if (fd < 0) {
        close(dfd);
        return false;
    }

    struct stat st {};
    if (fstat(fd, &st) != 0 || !S_ISREG(st.st_mode)) {
        close(fd);
        close(dfd);
        return false;
    }

    if (ftruncate(fd, 0) != 0) {
        close(fd);
        close(dfd);
        return false;
    }

    const char* content = "important_config";
    size_t len = std::strlen(content);
    ssize_t written = 0;
    while ((size_t)written < len) {
        ssize_t n = write(fd, content + written, len - written);
        if (n <= 0) {
            close(fd);
            close(dfd);
            return false;
        }
        written += n;
    }

    // Best-effort fsync
    (void)fsync(fd);

    close(fd);
    close(dfd);
    return true;
}

int main() {
    // 5 test cases
    // Create temporary directories for tests
    char tmpl1[] = "/tmp/cpp_icase1_XXXXXX";
    char* dir1 = mkdtemp(tmpl1);
    std::cout << "Test1: " << (dir1 ? (write_important_config(dir1) ? "true" : "false") : "false") << std::endl;

    // Test2: write again to same dir
    std::cout << "Test2: " << (dir1 ? (write_important_config(dir1) ? "true" : "false") : "false") << std::endl;

    // Test3: baseDir is a file
    char tfile[] = "/tmp/cpp_icase3_XXXXXX";
    int tfd = mkstemp(tfile);
    if (tfd >= 0) close(tfd);
    std::cout << "Test3: " << (write_important_config(tfile) ? "true" : "false") << std::endl;

    // Test4: non-existent directory
    std::string nonexist = std::string(dir1 ? dir1 : "/tmp") + "/nonexistent_subdir_do_not_create";
    std::cout << "Test4: " << (write_important_config(nonexist) ? "true" : "false") << std::endl;

    // Test5: valid subdir
    char tmpl5[] = "/tmp/cpp_icase5_XXXXXX";
    char* dir5 = mkdtemp(tmpl5);
    std::string sub = std::string(dir5 ? dir5 : "/tmp") + "/conf";
    mkdir(sub.c_str(), 0700);
    std::cout << "Test5: " << (write_important_config(sub) ? "true" : "false") << std::endl;

    return 0;
}