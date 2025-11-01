#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <filesystem>

std::string readFileFromSafeDir(const std::string& safeDir, const std::string& requested) {
    if (safeDir.empty() || requested.empty()) {
        throw std::invalid_argument("Empty parameter");
    }
    if (requested.find('\0') != std::string::npos) {
        throw std::invalid_argument("Invalid character in requested path");
    }

    namespace fs = std::filesystem;
    fs::path base = fs::canonical(fs::path(safeDir));
    fs::path req(requested);

    if (req.is_absolute()) {
        throw std::runtime_error("Absolute paths are not allowed");
    }

    fs::path joined = (base / req).lexically_normal();

    if (!fs::exists(joined)) {
        throw std::runtime_error("File does not exist");
    }

    // Resolve canonical path to remove symlinks and check containment
    fs::path canonTarget = fs::canonical(joined);

    // Ensure target remains within base
    auto baseStr = base.string();
    auto targetStr = canonTarget.string();
#ifdef _WIN32
    // Case-insensitive compare for drive-letter paths
    auto lower = [](std::string s){ for (auto& c : s) c = (char)tolower((unsigned char)c); return s; };
    if (lower(targetStr).rfind(lower(baseStr), 0) != 0 ||
        !(targetStr.size() == baseStr.size() || targetStr[baseStr.size()] == '\\' || targetStr[baseStr.size()] == '/')) {
        throw std::runtime_error("Path traversal attempt detected");
    }
#else
    if (targetStr.rfind(baseStr, 0) != 0 ||
        !(targetStr.size() == baseStr.size() || targetStr[baseStr.size()] == '/')) {
        throw std::runtime_error("Path traversal attempt detected");
    }
#endif

    // Disallow symlinks explicitly
    if (fs::is_symlink(joined) || fs::is_symlink(canonTarget)) {
        throw std::runtime_error("Symlinks are not allowed");
    }

    if (!fs::is_regular_file(canonTarget)) {
        throw std::runtime_error("Target is not a regular file");
    }

    std::ifstream in(canonTarget, std::ios::binary);
    if (!in) {
        throw std::runtime_error("Failed to open file");
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

static void createFile(const std::filesystem::path& p, const std::string& content) {
    namespace fs = std::filesystem;
    if (p.has_parent_path()) {
        fs::create_directories(p.parent_path());
    }
    std::ofstream out(p, std::ios::binary | std::ios::trunc);
    out << content;
}

int main() {
    try {
        namespace fs = std::filesystem;
        fs::path base = fs::absolute("safe_dir_cpp");
        fs::create_directories(base);
        createFile(base / "file1.txt", "Hello from C++ file1");
        createFile(base / "sub/inner.txt", "Inner C++ file");
        createFile(base.parent_path() / "outside_cpp.txt", "Outside C++ file");

        std::string safeDir = base.string();

        // Test 1: normal file
        try {
            auto s1 = readFileFromSafeDir(safeDir, "file1.txt");
            std::cout << "Test1 OK: " << s1 << "\n";
        } catch (const std::exception& e) {
            std::cout << "Test1 ERROR: " << e.what() << "\n";
        }

        // Test 2: subdirectory file
        try {
            auto s2 = readFileFromSafeDir(safeDir, "sub/inner.txt");
            std::cout << "Test2 OK: " << s2 << "\n";
        } catch (const std::exception& e) {
            std::cout << "Test2 ERROR: " << e.what() << "\n";
        }

        // Test 3: traversal attempt
        try {
            auto s3 = readFileFromSafeDir(safeDir, "../outside_cpp.txt");
            std::cout << "Test3 OK (unexpected): " << s3 << "\n";
        } catch (const std::exception& e) {
            std::cout << "Test3 Expected ERROR: " << e.what() << "\n";
        }

        // Test 4: absolute path attempt
        try {
            auto s4 = readFileFromSafeDir(safeDir, (base / "file1.txt").string());
            std::cout << "Test4 OK (unexpected): " << s4 << "\n";
        } catch (const std::exception& e) {
            std::cout << "Test4 Expected ERROR: " << e.what() << "\n";
        }

        // Test 5: missing file
        try {
            auto s5 = readFileFromSafeDir(safeDir, "missing.txt");
            std::cout << "Test5 OK (unexpected): " << s5 << "\n";
        } catch (const std::exception& e) {
            std::cout << "Test5 Expected ERROR: " << e.what() << "\n";
        }
    } catch (const std::exception& e) {
        std::cout << "Setup ERROR: " << e.what() << "\n";
    }
    return 0;
}