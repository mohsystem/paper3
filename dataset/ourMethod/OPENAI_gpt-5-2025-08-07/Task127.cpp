#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <system_error>

#if defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#endif

namespace fs = std::filesystem;

static std::string safeMsg(const std::string& m) {
    std::string s = m;
    for (char& c : s) {
        if (c == '\r' || c == '\n' || c == '\t') c = ' ';
    }
    return s;
}

static bool is_or_has_symlink(const fs::path& target, const fs::path& stop_at_inclusive) {
    std::error_code ec;
    fs::path abs = fs::weakly_canonical(target, ec);
    if (ec) abs = fs::absolute(target, ec);
    fs::path stop = fs::weakly_canonical(stop_at_inclusive, ec);
    if (ec) stop = fs::absolute(stop_at_inclusive, ec);

    fs::path p = abs;
    while (true) {
        std::error_code e2;
        fs::file_status st = fs::symlink_status(p, e2);
        if (e2) return true;
        if (fs::is_symlink(st)) return true;
        if (p == stop || p.has_parent_path() == false) break;
        fs::path parent = p.parent_path();
        if (parent == p) break;
        p = parent;
    }
    return false;
}

static bool starts_with_path(const fs::path& p, const fs::path& base) {
    auto p_it = p.begin();
    auto b_it = base.begin();
    for (; b_it != base.end(); ++b_it, ++p_it) {
        if (p_it == p.end() || *p_it != *b_it) return false;
    }
    return true;
}

static std::string copyToTemp(const std::string& baseDir, const std::string& userPath, const std::string& tempDir) {
    // Validate inputs
    if (baseDir.empty() || userPath.empty() || tempDir.empty()) {
        return "ERROR: Empty input parameter";
    }
    if (userPath.size() > 4096) {
        return "ERROR: Path too long";
    }
    if (userPath.find('\0') != std::string::npos) {
        return "ERROR: Invalid character in path";
    }

    std::error_code ec;
    fs::path base = fs::absolute(fs::path(baseDir), ec);
    if (ec) return "ERROR: Invalid base directory";
    base = fs::weakly_canonical(base, ec);
    if (ec) base = fs::absolute(base);

    fs::path tmpd = fs::absolute(fs::path(tempDir), ec);
    if (ec) return "ERROR: Invalid temp directory";
    tmpd = fs::weakly_canonical(tmpd, ec);
    if (ec) tmpd = fs::absolute(tmpd);

    if (!fs::exists(base)) return "ERROR: Base directory does not exist";
    if (!fs::is_directory(base)) return "ERROR: Base path is not a directory";
    if (is_or_has_symlink(base, base)) return "ERROR: Base directory or its parents contain a symlink";

    fs::path userP(userPath);
    if (userP.is_absolute()) return "ERROR: Path must be relative to base directory";

    fs::path resolved = fs::weakly_canonical(base / userP, ec);
    if (ec) resolved = fs::absolute(base / userP);

    // Ensure resolved stays within base (lexical)
    fs::path normalized = fs::path(base / userP).lexically_normal();
    if (!starts_with_path(normalized, base)) {
        return "ERROR: Path escapes base directory";
    }

    if (is_or_has_symlink(resolved, base)) return "ERROR: Source path contains a symlink";
    if (!fs::exists(resolved)) return "ERROR: Source file does not exist";
    if (!fs::is_regular_file(resolved)) return "ERROR: Source is not a regular file";

    // Ensure temp directory exists
    fs::create_directories(tmpd, ec);
    if (ec) return std::string("ERROR: Cannot create temp directory: ") + safeMsg(ec.message());
    if (!fs::is_directory(tmpd)) return "ERROR: Temp path is not a directory";
    if (is_or_has_symlink(tmpd, tmpd)) return "ERROR: Temp directory or its parents contain a symlink";

    // Create secure temp file
    fs::path tempPath;
#if defined(_WIN32)
    // Windows: use GetTempFileName in specified directory is awkward; create a unique name.
    // We'll loop to create a unique file with CreateFile.
    for (int i = 0; i < 100; ++i) {
        std::string name = "copy_" + std::to_string(::GetTickCount64()) + "_" + std::to_string(i) + ".tmp";
        fs::path candidate = tmpd / name;
        HANDLE h = CreateFileA(candidate.string().c_str(),
                               GENERIC_WRITE | GENERIC_READ,
                               0,
                               NULL,
                               CREATE_NEW,
                               FILE_ATTRIBUTE_TEMPORARY,
                               NULL);
        if (h != INVALID_HANDLE_VALUE) {
            // Write copy
            HANDLE src = CreateFileA(resolved.string().c_str(),
                                     GENERIC_READ,
                                     FILE_SHARE_READ,
                                     NULL,
                                     OPEN_EXISTING,
                                     FILE_ATTRIBUTE_NORMAL,
                                     NULL);
            if (src == INVALID_HANDLE_VALUE) {
                CloseHandle(h);
                DeleteFileA(candidate.string().c_str());
                return "ERROR: Cannot open source file";
            }
            // Copy buffer
            const size_t BUF = 8192;
            std::vector<char> buf(BUF);
            DWORD r = 0, w = 0;
            BOOL ok = TRUE;
            while (ok && ReadFile(src, buf.data(), (DWORD)BUF, &r, NULL) && r > 0) {
                if (!WriteFile(h, buf.data(), r, &w, NULL) || w != r) {
                    ok = FALSE;
                    break;
                }
            }
            if (!ok) {
                CloseHandle(src);
                CloseHandle(h);
                DeleteFileA(candidate.string().c_str());
                return "ERROR: I/O error during copy";
            }
            // Flush
            FlushFileBuffers(h);
            CloseHandle(src);
            CloseHandle(h);
            tempPath = candidate;
            break;
        }
    }
    if (tempPath.empty()) {
        return "ERROR: Cannot create temp file";
    }
    return tempPath.string();
#else
    // POSIX path: open source with O_NOFOLLOW, lock, create temp with mkstemp
    int src_fd = -1;
    {
        int flags = O_RDONLY;
#ifdef O_NOFOLLOW
        flags |= O_NOFOLLOW;
#endif
#ifdef O_CLOEXEC
        flags |= O_CLOEXEC;
#endif
        src_fd = ::open(resolved.c_str(), flags);
        if (src_fd < 0) {
            return "ERROR: Cannot open source file";
        }
        // Shared lock
        (void)flock(src_fd, LOCK_SH);
    }

    // Build template for mkstemp
    std::string tmpl = (tmpd / "copy_XXXXXX").string();
    std::vector<char> ctmpl(tmpl.begin(), tmpl.end());
    ctmpl.push_back('\0');
    int dst_fd = ::mkstemp(ctmpl.data());
    if (dst_fd < 0) {
        ::close(src_fd);
        return "ERROR: Cannot create temp file";
    }
    // Restrictive perms 0600
    (void)fchmod(dst_fd, S_IRUSR | S_IWUSR);
    // Exclusive lock on destination
    (void)flock(dst_fd, LOCK_EX);

    // Copy loop
    const size_t BUF = 8192;
    std::vector<char> buf(BUF);
    ssize_t r = 0;
    while (true) {
        r = ::read(src_fd, buf.data(), BUF);
        if (r < 0) {
            int e = errno;
            ::close(src_fd);
            ::close(dst_fd);
            ::unlink(ctmpl.data());
            return std::string("ERROR: Read failed: ") + safeMsg(std::string(strerror(e)));
        } else if (r == 0) {
            break;
        }
        ssize_t off = 0;
        while (off < r) {
            ssize_t w = ::write(dst_fd, buf.data() + off, (size_t)(r - off));
            if (w < 0) {
                int e = errno;
                ::close(src_fd);
                ::close(dst_fd);
                ::unlink(ctmpl.data());
                return std::string("ERROR: Write failed: ") + safeMsg(std::string(strerror(e)));
            }
            off += w;
        }
    }
    // Flush
    (void)fsync(dst_fd);
    ::close(src_fd);
    ::close(dst_fd);

    tempPath = fs::path(ctmpl.data());
    return tempPath.string();
#endif
}

static void writeText(const fs::path& p, const std::string& s) {
    std::error_code ec;
    fs::create_directories(p.parent_path(), ec);
    std::ofstream f(p, std::ios::binary | std::ios::trunc);
    f << s;
}

int main() {
    // Setup test environment
    std::error_code ec;
    fs::path base = fs::temp_directory_path() / fs::path("base_dir_cpp_" + std::to_string(std::rand()));
    fs::create_directories(base, ec);
    fs::path tempOut = fs::temp_directory_path() / fs::path("temp_out_cpp_" + std::to_string(std::rand()));
    fs::create_directories(tempOut, ec);

    fs::path f1 = base / "file1.txt";
    writeText(f1, "Hello world 1");
    fs::path sub = base / "subdir";
    fs::create_directories(sub, ec);
    fs::path f2 = sub / "file2.txt";
    writeText(f2, "Hello world 2");

    // Attempt to create a symlink (may fail on Windows)
    try {
        fs::path link = base / "link_to_f1";
        std::error_code e2;
        fs::remove(link, e2);
        fs::create_symlink("file1.txt", link, e2);
    } catch (...) {}

    std::cout << "Test1: " << copyToTemp(base.string(), "file1.txt", tempOut.string()) << "\n";
    std::cout << "Test2: " << copyToTemp(base.string(), "subdir/file2.txt", tempOut.string()) << "\n";
    std::cout << "Test3: " << copyToTemp(base.string(), "../../etc/passwd", tempOut.string()) << "\n";
    std::cout << "Test4: " << copyToTemp(base.string(), "does_not_exist.txt", tempOut.string()) << "\n";
    std::cout << "Test5: " << copyToTemp(base.string(), (base / "file1.txt").string(), tempOut.string()) << "\n";
    return 0;
}