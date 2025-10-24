#include <iostream>
#include <string>
#include <filesystem>
#include <vector>
#include <cerrno>

#if defined(_WIN32)
  #include <io.h>
  #include <fcntl.h>
  #include <sys/stat.h>
#else
  #include <unistd.h>
  #include <fcntl.h>
  #include <sys/stat.h>
#endif

// Step 1: Problem understanding:
// Create "script.sh" in a given directory with provided content and make it executable.

// Step 2: Security requirements:
// - Use O_CREAT|O_EXCL to avoid overwrite.
// - Use O_NOFOLLOW when available to avoid symlinks.
// - Use least privilege permissions (0700).
// - fsync to ensure durability.

bool create_script_file(const std::string& directory, const std::string& content, bool set_executable) {
    if (directory.empty()) return false;

    std::filesystem::path dir = std::filesystem::weakly_canonical(std::filesystem::path(directory));
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    if (ec) return false;

    std::filesystem::path target = dir / "script.sh";

#if defined(_WIN32)
    int flags = _O_CREAT | _O_EXCL | _O_WRONLY | _O_BINARY;
    int mode = _S_IREAD | _S_IWRITE; // Windows doesn't have exec bit; create with rw for owner
    int fd = _open(target.string().c_str(), flags, mode);
    if (fd == -1) return false;

    const std::string& data = content;
    size_t total = 0;
    while (total < data.size()) {
        int written = _write(fd, data.data() + total, static_cast<unsigned int>(data.size() - total));
        if (written <= 0) {
            _close(fd);
            return false;
        }
        total += static_cast<size_t>(written);
    }
    // No direct fsync equivalent; _commit flushes
    if (_commit(fd) == -1) {
        _close(fd);
        return false;
    }
    _close(fd);
    // Best-effort: mark as readable/executable concept doesn't apply on Windows
    return true;
#else
    int flags = O_CREAT | O_EXCL | O_WRONLY;
    #ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
    #endif
    mode_t mode = set_executable ? (S_IRUSR | S_IWUSR | S_IXUSR) : (S_IRUSR | S_IWUSR);

    int fd = ::open(target.c_str(), flags, mode);
    if (fd == -1) {
        return false;
    }

    const std::string& data = content;
    size_t total = 0;
    while (total < data.size()) {
        ssize_t written = ::write(fd, data.data() + total, data.size() - total);
        if (written <= 0) {
            ::close(fd);
            return false;
        }
        total += static_cast<size_t>(written);
    }

    if (::fsync(fd) == -1) {
        ::close(fd);
        return false;
    }

    if (set_executable) {
        struct stat st{};
        if (fstat(fd, &st) == 0) {
            mode_t newMode = st.st_mode | S_IXUSR;
            fchmod(fd, newMode);
        }
    }

    ::close(fd);
    return true;
#endif
}

// Step 4 and 5: Code review & Secure code output: Addressed in implementation.

int main() {
    for (int i = 1; i <= 5; ++i) {
        std::filesystem::path base = std::filesystem::temp_directory_path() / ("task101_cpp_test_" + std::to_string(i));
        std::error_code ec;
        std::filesystem::create_directories(base, ec);
        std::string content = "#!/bin/sh\necho \"Hello from C++ test " + std::to_string(i) + "\"\n";
        bool ok = create_script_file(base.string(), content, true);
        auto script = base / "script.sh";
        std::cout << "Test " << i << " dir: " << base << "\n";
        std::cout << "  Created: " << (ok ? "true" : "false") << "\n";
        std::cout << "  Exists: " << (std::filesystem::exists(script) ? "true" : "false") << "\n";
#if defined(_WIN32)
        std::cout << "  Executable: n/a on Windows\n";
#else
        std::cout << "  Executable: " << ((access(script.c_str(), X_OK) == 0) ? "true" : "false") << "\n";
#endif
    }
    return 0;
}