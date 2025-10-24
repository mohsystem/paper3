#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <filesystem>
#include <system_error>
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

namespace fs = std::filesystem;

static const size_t MAX_SIZE = 1024 * 1024; // 1 MiB

struct UploadResult {
    bool success;
    std::string message;
    std::string savedPath;
};

static bool isValidName(const std::string& name) {
    static const std::regex re("^[A-Za-z0-9._-]{1,100}$");
    return std::regex_match(name, re);
}

UploadResult uploadFile(const std::string& baseDir, const std::string& originalName, const std::vector<uint8_t>& data) {
    if (baseDir.empty()) {
        return {false, "Base directory is required.", ""};
    }
    if (originalName.empty()) {
        return {false, "Filename is required.", ""};
    }
    if (!isValidName(originalName)) {
        return {false, "Invalid filename. Allowed: A-Z, a-z, 0-9, ., _, - with length 1..100.", ""};
    }
    if (data.empty()) {
        return {false, "File is empty.", ""};
    }
    if (data.size() > MAX_SIZE) {
        return {false, "File too large. Max " + std::to_string(MAX_SIZE) + " bytes.", ""};
    }

    try {
        std::error_code ec;
        fs::create_directories(baseDir, ec);
        if (ec) {
            return {false, "Failed to create base directory: " + ec.message(), ""};
        }
        fs::path basePath = fs::canonical(fs::path(baseDir), ec);
        if (ec) {
            return {false, "Failed to resolve base directory: " + ec.message(), ""};
        }

        std::string baseName = originalName;
        std::string ext;
        auto pos = originalName.find_last_of('.');
        if (pos != std::string::npos && pos > 0 && pos < originalName.size() - 1) {
            baseName = originalName.substr(0, pos);
            ext = originalName.substr(pos);
        }

        for (int counter = 0; counter < 10000; ++counter) {
            std::string candidateName = (counter == 0) ? (baseName + ext) : (baseName + "(" + std::to_string(counter) + ")" + ext);
            fs::path finalPath = basePath / candidateName;

            // Ensure parent directory is the base path
            fs::path parent = finalPath.parent_path();
            fs::path parentCanonical = fs::canonical(parent, ec);
            if (ec || parentCanonical != basePath) {
                continue;
            }

            int flags = O_CREAT | O_EXCL | O_WRONLY;
#ifdef O_NOFOLLOW
            flags |= O_NOFOLLOW;
#endif
            int fd = ::open(finalPath.c_str(), flags, 0600);
            if (fd == -1) {
                if (errno == EEXIST) {
                    continue; // try next suffix
                } else if (errno == ENOENT) {
                    return {false, "Parent directory missing.", ""};
                } else if (errno == ELOOP) {
                    // Symlink encountered
                    continue;
                } else {
                    return {false, std::string("Open error: ") + std::strerror(errno), ""};
                }
            }

            // Write data
            ssize_t total = 0;
            const uint8_t* ptr = data.data();
            ssize_t left = static_cast<ssize_t>(data.size());
            bool ok = true;
            while (left > 0) {
                ssize_t w = ::write(fd, ptr + total, static_cast<size_t>(left));
                if (w < 0) {
                    ok = false;
                    break;
                }
                total += w;
                left -= w;
            }

            if (ok) {
                if (::fsync(fd) != 0) {
                    ok = false;
                }
            }

            int savedErrno = errno;
            ::close(fd);
            errno = savedErrno;

            if (!ok) {
                ::unlink(finalPath.c_str());
                return {false, std::string("Write/fsync error: ") + std::strerror(errno), ""};
            }

            struct stat st {};
            if (::lstat(finalPath.c_str(), &st) != 0) {
                ::unlink(finalPath.c_str());
                return {false, "Stat error after write.", ""};
            }
            if (!S_ISREG(st.st_mode)) {
                ::unlink(finalPath.c_str());
                return {false, "Upload failed: not a regular file.", ""};
            }

            return {true, "Upload successful.", finalPath.string()};
        }
        return {false, "Could not allocate a unique filename.", ""};
    } catch (const std::exception& e) {
        return {false, std::string("Unexpected error: ") + e.what(), ""};
    }
}

static void printResult(const std::string& label, const UploadResult& r) {
    std::cout << label << ": success=" << (r.success ? "true" : "false")
              << " message=" << r.message
              << " savedPath=" << r.savedPath << "\n";
}

int main() {
    std::string base = "uploads_cpp";

    std::vector<uint8_t> data1{'H','e','l','l','o',',',' ','w','o','r','l','d','!'};
    std::vector<uint8_t> data2{'A','n','o','t','h','e','r',' ','c','o','n','t','e','n','t'};
    std::vector<uint8_t> data3(MAX_SIZE + 1, 0x41);
    std::vector<uint8_t> data4{'S','m','a','l','l'};
    std::vector<uint8_t> data5{'D','u','p','l','i','c','a','t','e'};

    auto r1 = uploadFile(base, "hello.txt", data1);
    printResult("Test 1", r1);

    auto r2 = uploadFile(base, "bad/evil.txt", data2);
    printResult("Test 2", r2);

    auto r3 = uploadFile(base, "big.bin", data3);
    printResult("Test 3", r3);

    auto r4 = uploadFile(base, "../secret.txt", data4);
    printResult("Test 4", r4);

    auto r5a = uploadFile(base, "hello.txt", data5);
    printResult("Test 5a", r5a);
    auto r5b = uploadFile(base, "hello.txt", data5);
    printResult("Test 5b", r5b);

    return 0;
}