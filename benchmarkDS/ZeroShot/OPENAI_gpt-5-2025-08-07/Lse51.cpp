#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

// Securely write a text file. Returns absolute path string.
std::string writeFileSecure(const std::string& path, const std::string& content) {
    fs::path p(path);
    if (p.has_parent_path()) {
        fs::create_directories(p.parent_path());
        // Attempt to set perms to 0700 for directory if supported
        std::error_code ec;
        fs::permissions(p.parent_path(),
                        fs::perms::owner_all,
                        fs::perm_options::replace,
                        ec);
    }
    // Open with truncation
    std::ofstream ofs(p, std::ios::binary | std::ios::out | std::ios::trunc);
    if (!ofs) throw std::runtime_error("Failed to open for write: " + p.string());
    ofs << content;
    ofs.close();
    // Set file permissions to owner read/write if supported
    std::error_code ec;
    fs::permissions(p, fs::perms::owner_read | fs::perms::owner_write, fs::perm_options::replace, ec);
    return fs::absolute(p).string();
}

// Securely read a text file. Returns content.
std::string readFileSecure(const std::string& path) {
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) throw std::runtime_error("Failed to open for read: " + path);
    std::string data((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    return data;
}

// Resolve a child entry safely under destDir, returning empty string if unsafe.
std::string resolveSafeChild(const std::string& destDir, const std::string& entryName) {
    if (entryName.empty()) return "";
    // Disallow absolute or Windows drive letters
    if (!entryName.empty() && (entryName[0] == '/' || entryName[0] == '\\')) return "";
    if (entryName.size() >= 2 && std::isalpha(static_cast<unsigned char>(entryName[0])) && entryName[1] == ':') return "";

    std::string normEntry = entryName;
    for (auto& c : normEntry) {
        if (c == '\\') c = '/';
    }
    fs::path base(destDir);
    fs::path cand = (base / normEntry).lexically_normal();
    // If base doesn't exist, use absolute normalization for comparison
    fs::path baseAbs = fs::absolute(base).lexically_normal();
    fs::path candAbs = fs::absolute(cand).lexically_normal();
    // Ensure candAbs is within baseAbs
    auto baseIt = baseAbs.begin();
    auto candIt = candAbs.begin();
    for (; baseIt != baseAbs.end() && candIt != candAbs.end(); ++baseIt, ++candIt) {
        if (*baseIt != *candIt) return "";
    }
    if (baseIt != baseAbs.end()) return ""; // base longer than candidate
    // Additionally, reject any path with ".." segments in the entryName
    if (normEntry.find("..") != std::string::npos) return "";
    return candAbs.string();
}

int main() {
    try {
        fs::path base = "/tmp/cpp_lse51";
        fs::create_directories(base);

        // Test 1: write and read
        std::string f1 = (base / "t1.txt").string();
        writeFileSecure(f1, "hello world");
        std::cout << "T1: " << readFileSecure(f1) << "\n";

        // Test 2: overwrite securely
        writeFileSecure(f1, "second line");
        std::cout << "T2: " << readFileSecure(f1) << "\n";

        // Test 3: safe resolve
        std::string dest = "/tmp/unpack";
        fs::create_directories(dest);
        std::string r1 = resolveSafeChild(dest, "dir/sub/file.txt");
        std::cout << "T3 safe: " << (r1.empty() ? std::string("none") : r1) << "\n";

        // Test 4: traversal resolve
        std::string r2 = resolveSafeChild(dest, "../evil.txt");
        std::cout << "T4 bad resolved? " << (!r2.empty() ? "true" : "false") << "\n";

        // Test 5: absolute path resolve
        std::string r3 = resolveSafeChild(dest, "/etc/passwd");
        std::cout << "T5 abs resolved? " << (!r3.empty() ? "true" : "false") << "\n";
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}