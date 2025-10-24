#include <iostream>
#include <string>
#include <filesystem>
#include <vector>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>

static bool ensure_dir(const std::filesystem::path& p) {
    std::error_code ec;
    if (std::filesystem::exists(p, ec)) {
        return std::filesystem::is_directory(p, ec);
    }
    return std::filesystem::create_directories(p, ec);
}

std::string createExecutableScript(const std::string& dirPath, const std::string& content, bool overwrite) {
    std::string defaultContent = "#!/bin/sh\necho \"Hello from script.sh\"\n";
    std::string data = content.empty() ? defaultContent : content;
    if (data.empty() || data.back() != '\n') data.push_back('\n');

    try {
        std::filesystem::path base = dirPath.empty() ? std::filesystem::current_path() : std::filesystem::path(dirPath);
        base = std::filesystem::absolute(base);
        if (!ensure_dir(base)) return "";

        std::filesystem::path finalPath = base / "script.sh";
        finalPath = std::filesystem::weakly_canonical(finalPath.parent_path()) / finalPath.filename();

        // Ensure finalPath is within base
        if (finalPath.parent_path() != std::filesystem::weakly_canonical(base)) {
            return "";
        }

        struct stat st;
        if (!overwrite && lstat(finalPath.c_str(), &st) == 0) {
            return "";
        }

        // Create temp file using mkstemp for safety
        std::string tmpTemplate = (base / "script.sh.tmp-XXXXXX").string();
        std::vector<char> tmpl(tmpTemplate.begin(), tmpTemplate.end());
        tmpl.push_back('\0');
        int fd = mkstemp(tmpl.data());
        if (fd == -1) {
            return "";
        }

        // Set permissions to 0700
        fchmod(fd, 0700);

        // Write data
        ssize_t total = 0;
        const char* buf = data.c_str();
        ssize_t len = static_cast<ssize_t>(data.size());
        while (total < len) {
            ssize_t w = write(fd, buf + total, len - total);
            if (w <= 0) {
                close(fd);
                unlink(tmpl.data());
                return "";
            }
            total += w;
        }
        fsync(fd);
        close(fd);

        // Atomic rename (overwrites by default on POSIX)
        if (!overwrite) {
            if (lstat(finalPath.c_str(), &st) == 0) {
                unlink(tmpl.data());
                return "";
            }
        }
        if (rename(tmpl.data(), finalPath.c_str()) != 0) {
            unlink(tmpl.data());
            return "";
        }

        // Ensure final permissions
        chmod(finalPath.c_str(), 0700);

        // Confirm not a symlink
        if (lstat(finalPath.c_str(), &st) != 0) {
            return "";
        }
        if (!S_ISREG(st.st_mode)) {
            unlink(finalPath.c_str());
            return "";
        }

        return finalPath.string();
    } catch (...) {
        return "";
    }
}

int main() {
    try {
        std::filesystem::path d1 = std::filesystem::temp_directory_path() / "t101_cpp_1";
        std::filesystem::path d2 = std::filesystem::temp_directory_path() / "t101_cpp_2";
        std::filesystem::create_directories(d1);
        std::filesystem::create_directories(d2);

        std::string r1 = createExecutableScript(d1.string(), "#!/bin/sh\necho Case1", false);
        std::cout << "Test1: " << (r1.empty() ? "null" : r1) << std::endl;

        std::string r2 = createExecutableScript(d1.string(), "#!/bin/sh\necho Case2", false);
        std::cout << "Test2: " << (r2.empty() ? "null" : r2) << std::endl;

        std::string r3 = createExecutableScript(d1.string(), "#!/bin/sh\necho Case3 Overwrite", true);
        std::cout << "Test3: " << (r3.empty() ? "null" : r3) << std::endl;

        std::string r4 = createExecutableScript(d2.string(), "echo Case4 Content only", false);
        std::cout << "Test4: " << (r4.empty() ? "null" : r4) << std::endl;

        std::string r5 = createExecutableScript(".", "echo Case5 in CWD", true);
        std::cout << "Test5: " << (r5.empty() ? "null" : r5) << std::endl;
    } catch (...) {
        std::cout << "Setup failed" << std::endl;
    }
    return 0;
}