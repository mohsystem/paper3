#include <bits/stdc++.h>
#if defined(__unix__) || defined(__APPLE__)
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif
#include <filesystem>

namespace fs = std::filesystem;

static const size_t MAX_LINE_LENGTH = 8192;
static const size_t MAX_REL_PATH_LENGTH = 4096;

static std::string trim(const std::string& s) {
    size_t i = 0, j = s.size();
    while (i < j && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
    while (j > i && std::isspace(static_cast<unsigned char>(s[j - 1]))) --j;
    return s.substr(i, j - i);
}

static fs::path resolveSafePath(const fs::path& baseDir, const std::string& relativePath) {
    if (!fs::exists(baseDir) || !fs::is_directory(baseDir)) {
        throw std::runtime_error("Base directory does not exist or is not a directory.");
    }
    if (fs::is_symlink(baseDir)) {
        throw std::runtime_error("Base directory must not be a symbolic link.");
    }
    if (relativePath.empty() || relativePath.size() > MAX_REL_PATH_LENGTH) {
        throw std::runtime_error("Invalid relative path length.");
    }
    if (relativePath.find('\0') != std::string::npos) {
        throw std::runtime_error("Invalid character in path.");
    }
    fs::path rel(relativePath);
    if (rel.is_absolute()) {
        throw std::runtime_error("Relative path must not be absolute.");
    }
    for (auto& part : rel) {
        if (part == "..") {
            throw std::runtime_error("Path traversal detected.");
        }
    }
    fs::path baseReal = fs::canonical(baseDir);
    fs::path current = baseReal;
    for (auto& part : rel.lexically_normal()) {
        if (part == ".") continue;
        current /= part;
        if (fs::exists(current) && fs::is_symlink(current)) {
            throw std::runtime_error("Symlink encountered in path.");
        }
    }
    fs::path normalized = fs::weakly_canonical(current);
    if (normalized.string().compare(0, baseReal.string().size(), baseReal.string()) != 0) {
        throw std::runtime_error("Resolved path escapes base directory.");
    }
    if (!fs::exists(normalized)) {
        throw std::runtime_error("Target file does not exist.");
    }
    if (fs::is_directory(normalized)) {
        throw std::runtime_error("Target is a directory, expected a regular file.");
    }
    if (fs::is_symlink(normalized)) {
        throw std::runtime_error("Target file must not be a symbolic link.");
    }
    return normalized;
}

static std::vector<std::pair<std::string, std::string>> readAndSortKeyValueFile(const std::string& baseDir, const std::string& relativePath, char delimiter) {
    if (delimiter == '\n' || delimiter == '\r') {
        throw std::runtime_error("Invalid delimiter.");
    }
    fs::path safePath = resolveSafePath(fs::path(baseDir), relativePath);

    std::vector<std::pair<std::string, std::string>> entries;

#if defined(__unix__) || defined(__APPLE__)
    int flags = O_RDONLY;
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif
#ifdef O_CLOEXEC
    flags |= O_CLOEXEC;
#endif
    int fd = open(safePath.c_str(), flags);
    if (fd < 0) {
        throw std::runtime_error("Failed to open file securely.");
    }
    struct stat st{};
    if (fstat(fd, &st) != 0 || !S_ISREG(st.st_mode)) {
        close(fd);
        throw std::runtime_error("Target is not a regular file.");
    }
    // Read file content safely
    std::string buf;
    buf.reserve(4096);
    char tmp[4096];
    ssize_t n;
    while ((n = read(fd, tmp, sizeof(tmp))) > 0) {
        buf.append(tmp, tmp + n);
        if (buf.size() > 16 * 1024 * 1024) { // 16MB safeguard
            close(fd);
            throw std::runtime_error("File too large.");
        }
    }
    close(fd);
    // Parse lines
    size_t start = 0;
    bool firstLine = true;
    while (start <= buf.size()) {
        size_t end = buf.find('\n', start);
        if (end == std::string::npos) end = buf.size();
        std::string line = (end > start) ? buf.substr(start, end - start) : std::string();
        if (line.size() > MAX_LINE_LENGTH) {
            throw std::runtime_error("Line too long.");
        }
        // Strip CR
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (firstLine && !line.empty() && static_cast<unsigned char>(line[0]) == 0xEF) {
            // Potential BOM
            if (line.size() >= 3 &&
                static_cast<unsigned char>(line[0]) == 0xEF &&
                static_cast<unsigned char>(line[1]) == 0xBB &&
                static_cast<unsigned char>(line[2]) == 0xBF) {
                line = line.substr(3);
            }
        }
        firstLine = false;
        std::string trimmed = trim(line);
        if (!trimmed.empty() && trimmed[0] != '#') {
            auto pos = trimmed.find(delimiter);
            if (pos != std::string::npos) {
                std::string key = trim(trimmed.substr(0, pos));
                std::string value = trim(trimmed.substr(pos + 1));
                if (!key.empty()) {
                    entries.emplace_back(key, value);
                }
            }
        }
        if (end == buf.size()) break;
        start = end + 1;
    }
#else
    // Fallback for non-POSIX: best-effort using ifstream after checks
    std::ifstream in(safePath, std::ios::in | std::ios::binary);
    if (!in) {
        throw std::runtime_error("Failed to open file.");
    }
    std::string line;
    bool firstLine = true;
    while (std::getline(in, line)) {
        if (line.size() > MAX_LINE_LENGTH) {
            throw std::runtime_error("Line too long.");
        }
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (firstLine && !line.empty() && static_cast<unsigned char>(line[0]) == 0xEF) {
            if (line.size() >= 3 &&
                static_cast<unsigned char>(line[0]) == 0xEF &&
                static_cast<unsigned char>(line[1]) == 0xBB &&
                static_cast<unsigned char>(line[2]) == 0xBF) {
                line = line.substr(3);
            }
        }
        firstLine = false;
        std::string trimmed = trim(line);
        if (!trimmed.empty() && trimmed[0] != '#') {
            auto pos = trimmed.find(delimiter);
            if (pos != std::string::npos) {
                std::string key = trim(trimmed.substr(0, pos));
                std::string value = trim(trimmed.substr(pos + 1));
                if (!key.empty()) {
                    entries.emplace_back(key, value);
                }
            }
        }
    }
#endif

    std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) {
        if (a.first != b.first) return a.first < b.first;
        return a.second < b.second;
    });
    return entries;
}

static void writeFile(const fs::path& base, const std::string& rel, const std::string& content) {
    fs::path p = base / rel;
    fs::create_directories(p.parent_path());
    std::ofstream out(p, std::ios::binary | std::ios::trunc);
    if (!out) {
        throw std::runtime_error("Failed to create test file: " + p.string());
    }
    out.write(content.data(), static_cast<std::streamsize>(content.size()));
    out.close();
}

int main() {
    try {
        fs::path base = fs::temp_directory_path() / fs::path("kv_safe_base_cpp_" + std::to_string(::time(nullptr)));
        fs::create_directories(base);
        writeFile(base, "t1.txt", "apple=3\nbanana=1\ncherry=2\n");
        writeFile(base, "t2.txt", "# comment\norange = 10\n banana = 5\nbanana = 2\n");
        writeFile(base, "dir/sub/t3.txt", "z=last\n a=first\nm=mid\n");
        writeFile(base, "t4.txt", "x=a=b=c\nn=:emptyBefore\n:badline\n");
        writeFile(base, "t5.txt", "key with spaces = value with spaces\nalpha= \n gamma=3\n");

        auto r1 = readAndSortKeyValueFile(base.string(), "t1.txt", '=');
        auto r2 = readAndSortKeyValueFile(base.string(), "t2.txt", '=');
        auto r3 = readAndSortKeyValueFile(base.string(), "dir/sub/t3.txt", '=');
        auto r4 = readAndSortKeyValueFile(base.string(), "t4.txt", '=');
        auto r5 = readAndSortKeyValueFile(base.string(), "t5.txt", '=');

        auto printResult = [](const std::string& title, const std::vector<std::pair<std::string, std::string>>& v) {
            std::cout << title << "\n";
            for (const auto& kv : v) {
                std::cout << kv.first << "=" << kv.second << "\n";
            }
            std::cout << "---\n";
        };

        printResult("Test 1:", r1);
        printResult("Test 2:", r2);
        printResult("Test 3:", r3);
        printResult("Test 4:", r4);
        printResult("Test 5:", r5);

    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}