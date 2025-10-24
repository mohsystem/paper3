#include <cstdio>
#include <cstring>
#include <string>
#include <iostream>
#include <vector>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <random>
#include <filesystem>

#if defined(_WIN32)
#include <io.h>
#define PATH_SEP "\\"
#else
#define PATH_SEP "/"
#endif

namespace fs = std::filesystem;

static bool is_symlink_nofollow(const fs::path &p) {
    std::error_code ec;
    auto st = fs::symlink_status(p, ec);
    if (ec) return false;
    return fs::is_symlink(st);
}

static bool ensure_secure_dir(const fs::path &dir) {
    std::error_code ec;
    if (!fs::exists(dir, ec)) {
        if (!fs::create_directories(dir, ec)) {
            return false;
        }
#if !defined(_WIN32)
        ::chmod(dir.c_str(), 0700);
#endif
    }
    if (is_symlink_nofollow(dir)) {
        return false;
    }
    if (!fs::is_directory(dir, ec)) {
        return false;
    }
#if !defined(_WIN32)
    ::chmod(dir.c_str(), 0700);
#endif
    return true;
}

static std::string random_hex(size_t nbytes = 16) {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<unsigned int> dist(0, 255);
    std::string s;
    s.reserve(nbytes * 2);
    const char *hex = "0123456789abcdef";
    for (size_t i = 0; i < nbytes; ++i) {
        unsigned int v = dist(gen) & 0xFFu;
        s.push_back(hex[(v >> 4) & 0xF]);
        s.push_back(hex[v & 0xF]);
    }
    return s;
}

bool create_executable_script(const std::string &base_dir_str, const std::string &content) {
    if (base_dir_str.empty() || base_dir_str.size() > 4096) return false;

    fs::path base_dir = fs::absolute(fs::path(base_dir_str)).lexically_normal();

    if (!ensure_secure_dir(base_dir)) return false;

    fs::path final_path = (base_dir / "script.sh").lexically_normal();

    // Refuse overwrite if target exists (including symlink)
    std::error_code ec;
    if (fs::exists(final_path, ec)) {
        return false;
    }

    // Create temp file in same directory securely
    fs::path temp_path = base_dir / (".script.sh.tmp-" + random_hex(16));
    int fd = ::open(temp_path.c_str(), O_CREAT | O_EXCL | O_WRONLY
#if defined(O_CLOEXEC)
        | O_CLOEXEC
#endif
        , 0600);
    if (fd < 0) {
        // Retry once with another name
        temp_path = base_dir / (".script.sh.tmp-" + random_hex(16));
        fd = ::open(temp_path.c_str(), O_CREAT | O_EXCL | O_WRONLY
#if defined(O_CLOEXEC)
            | O_CLOEXEC
#endif
            , 0600);
        if (fd < 0) {
            return false;
        }
    }

    // Write content
    const char *buf = content.c_str();
    size_t remaining = content.size();
    while (remaining > 0) {
        ssize_t w = ::write(fd, buf, remaining);
        if (w < 0) {
            int err = errno;
            ::close(fd);
            ::unlink(temp_path.c_str());
            (void)err;
            return false;
        }
        buf += w;
        remaining -= static_cast<size_t>(w);
    }
    ::fsync(fd);
    ::close(fd);

#if !defined(_WIN32)
    ::chmod(temp_path.c_str(), 0700);
#endif

    // Publish atomically without overwrite:
    bool published = false;
#if !defined(_WIN32)
    if (::link(temp_path.c_str(), final_path.c_str()) == 0) {
        ::unlink(temp_path.c_str());
        published = true;
    } else {
        // Fallback: rename (atomic within same dir); ensure not overwriting
        if (!fs::exists(final_path, ec)) {
            if (::rename(temp_path.c_str(), final_path.c_str()) == 0) {
                published = true;
            } else {
                ::unlink(temp_path.c_str());
            }
        } else {
            ::unlink(temp_path.c_str());
        }
    }
#else
    // On Windows, std::filesystem::rename fails if target exists; good for no-overwrite
    try {
        fs::rename(temp_path, final_path);
        published = true;
    } catch (...) {
        std::error_code ec2;
        fs::remove(temp_path, ec2);
        published = false;
    }
#endif

    if (!published) {
        return false;
    }

#if !defined(_WIN32)
    ::chmod(final_path.c_str(), 0700);
#endif

    return true;
}

static std::string tmp_base(const std::string &suffix) {
#if defined(_WIN32)
    const char *t = std::getenv("TEMP");
    if (!t) t = ".";
#else
    const char *t = std::getenv("TMPDIR");
    if (!t) t = "/tmp";
#endif
    fs::path base = fs::path(t);
    return (base / ("Task101_" + suffix)).string();
}

int main() {
    std::string content = "#!/bin/sh\necho \"Hello from script\"\n";

    // Test 1: Valid base directory
    std::string t1 = tmp_base("test1");
    std::cout << "Test1: " << (create_executable_script(t1, content) ? "true" : "false") << "\n";

    // Test 2: Non-existing nested directory
    std::string t2 = (fs::path(tmp_base("test2")) / "nested").string();
    std::cout << "Test2: " << (create_executable_script(t2, content) ? "true" : "false") << "\n";

    // Test 3: Path normalization
    fs::path base3a = fs::path(tmp_base("test3")) / "a";
    fs::path t3 = base3a / ".." / "b";
    std::cout << "Test3: " << (create_executable_script(t3.string(), content) ? "true" : "false") << "\n";

    // Test 4: Pre-existing script.sh
    std::string t4 = tmp_base("test4");
    try {
        fs::create_directories(t4);
        fs::path pre = fs::path(t4) / "script.sh";
        if (!fs::exists(pre)) {
            int fd = ::open(pre.c_str(), O_CREAT | O_WRONLY
#if defined(O_CLOEXEC)
                | O_CLOEXEC
#endif
                , 0600);
            if (fd >= 0) {
                const char *s = "#!/bin/sh\necho preexisting\n";
                (void)::write(fd, s, std::strlen(s));
                ::close(fd);
            }
        }
    } catch (...) {}
    std::cout << "Test4 (exists): " << (create_executable_script(t4, content) ? "true" : "false") << "\n";

    // Test 5: Base path is a file
    std::string t5file = tmp_base("test5_as_file");
    {
        int fd = ::open(t5file.c_str(), O_CREAT | O_WRONLY
#if defined(O_CLOEXEC)
            | O_CLOEXEC
#endif
            , 0600);
        if (fd >= 0) {
            const char *s = "not a dir";
            (void)::write(fd, s, std::strlen(s));
            ::close(fd);
        }
    }
    std::cout << "Test5 (base is file): " << (create_executable_script(t5file, content) ? "true" : "false") << "\n";

    return 0;
}