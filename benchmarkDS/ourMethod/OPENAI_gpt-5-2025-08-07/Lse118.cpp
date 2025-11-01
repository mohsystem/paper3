#include <iostream>
#include <string>
#include <random>
#include <vector>
#include <cstring>
#include <cerrno>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <stdio.h>

static bool is_valid_filename(const std::string& name) {
    if (name.empty() || name.size() > 255) return false;
    if (name == "." || name == "..") return false;
    for (char c : name) {
        if (c == '/' || c == '\\') return false;
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '.' || c == '_' || c == '-')) return false;
    }
    return true;
}

static std::string generate_and_write(const std::string& baseDir, const std::string& filename) {
    // Validate filename
    if (!is_valid_filename(filename)) {
        throw std::runtime_error("Invalid filename");
    }

    // Open and validate base directory
    int dirfd = open(baseDir.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (dirfd < 0) throw std::runtime_error("Failed to open base directory");

    struct stat dst {};
    if (fstat(dirfd, &dst) != 0 || !S_ISDIR(dst.st_mode)) {
        close(dirfd);
        throw std::runtime_error("Invalid base directory");
    }

    // Generate random floats and concatenate
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    float f1 = dist(gen), f2 = dist(gen), f3 = dist(gen);
    std::string content = std::to_string(f1) + std::to_string(f2) + std::to_string(f3);

    // Create a unique temp file in baseDir
    std::string tmpname;
    int tmpfd = -1;
    for (int attempt = 0; attempt < 20; ++attempt) {
        tmpname = "tmp-" + std::to_string(getpid()) + "-" + std::to_string(rd()) + ".tmp";
        tmpfd = openat(dirfd, tmpname.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW, 0600);
        if (tmpfd >= 0) break;
        if (errno != EEXIST) {
            close(dirfd);
            throw std::runtime_error("Failed to create temp file");
        }
    }
    if (tmpfd < 0) {
        close(dirfd);
        throw std::runtime_error("Failed to create temp file");
    }

    // Write content
    ssize_t to_write = static_cast<ssize_t>(content.size());
    const char* buf = content.c_str();
    ssize_t written_total = 0;
    while (written_total < to_write) {
        ssize_t w = write(tmpfd, buf + written_total, to_write - written_total);
        if (w < 0) {
            int e = errno;
            close(tmpfd);
            unlinkat(dirfd, tmpname.c_str(), 0);
            close(dirfd);
            throw std::runtime_error("Write failed");
        }
        written_total += w;
    }

    // fsync and close temp file
    (void)fsync(tmpfd);
    (void)close(tmpfd);

    // Rename temp file to target
    if (renameat(dirfd, tmpname.c_str(), dirfd, filename.c_str()) != 0) {
        unlinkat(dirfd, tmpname.c_str(), 0);
        close(dirfd);
        throw std::runtime_error("Rename failed");
    }

    // fsync directory
    (void)fsync(dirfd);
    close(dirfd);

    return content;
}

int main() {
    // Create a temporary directory for tests
    char tmpl[] = "/tmp/safe-out-XXXXXX";
    char* dirpath = mkdtemp(tmpl);
    if (!dirpath) {
        std::cerr << "Failed to create temp dir\n";
        return 1;
    }
    std::string baseDir(dirpath);
    std::cout << "Base dir: " << baseDir << "\n";

    try {
        std::string r1 = generate_and_write(baseDir, "case1.txt");
        std::cout << "case1 -> " << r1 << "\n";
        std::string r2 = generate_and_write(baseDir, "case2.txt");
        std::cout << "case2 -> " << r2 << "\n";
        std::string r3 = generate_and_write(baseDir, "case3.txt");
        std::cout << "case3 -> " << r3 << "\n";
        std::string r4 = generate_and_write(baseDir, "case4.txt");
        std::cout << "case4 -> " << r4 << "\n";
        std::string r5 = generate_and_write(baseDir, "case5.txt");
        std::cout << "case5 -> " << r5 << "\n";
    } catch (const std::exception& ex) {
        std::cerr << "Error\n";
        return 1;
    }
    return 0;
}