// Chain-of-Through process in code generation:
// 1) Problem understanding: Delete a specified file; provide a function taking a path and returning success/failure; support CLI arguments.
// 2) Security requirements: Validate input, ensure path is a regular file, deny directories and symlinks, handle errors without exceptions leaking.
// 3) Secure coding generation: Use std::filesystem with symlink_status, is_regular_file, is_symlink, and remove with error_code.
// 4) Code review: Validate empty path, existence, type checks, non-throwing operations, and clear return values.
// 5) Secure code output: Function plus main with 5 test cases. If args given, process those instead.

#include <iostream>
#include <filesystem>
#include <system_error>
#include <fstream>
#include <string>
#include <chrono>
#include <random>

namespace fs = std::filesystem;

bool delete_file_secure(const std::string& path) {
    if (path.empty()) return false;
    std::error_code ec;

    fs::path p(path);
    fs::file_status st = fs::symlink_status(p, ec);
    if (ec) return false;

    if (!fs::exists(st)) return false;
    if (fs::is_symlink(st)) return false;
    if (!fs::is_regular_file(st)) return false;

    // Remove without throwing
    bool removed = fs::remove(p, ec);
    if (ec) return false;
    return removed;
}

static fs::path make_temp_file() {
    fs::path dir = fs::temp_directory_path();
    // Generate reasonably unique filename
    auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    static std::mt19937_64 rng{std::random_device{}()};
    uint64_t rnd = rng();
    fs::path file = dir / ("Task69_" + std::to_string(now) + "_" + std::to_string(rnd) + ".tmp");
    // Create the file
    std::ofstream ofs(file.string(), std::ios::out | std::ios::binary);
    ofs << "data";
    ofs.close();
    return file;
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            bool res = delete_file_secure(argv[i]);
            std::cout << argv[i] << " -> " << (res ? "deleted" : "failed") << "\n";
        }
        return 0;
    }

    // Five test cases
    // Test 1: Create temp file and delete (expect true)
    fs::path t1p = make_temp_file();
    bool t1 = delete_file_secure(t1p.string());
    std::cout << "Test1 delete temp file: " << (t1 ? "PASS" : "FAIL") << "\n";

    // Test 2: Delete same file again (expect false)
    bool t2 = delete_file_secure(t1p.string());
    std::cout << "Test2 delete non-existent file: " << (!t2 ? "PASS" : "FAIL") << "\n";

    // Test 3: Attempt delete a directory (expect false)
    fs::path dir = fs::temp_directory_path() / ("Task69_dir_" + std::to_string(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
    std::error_code ec;
    fs::create_directory(dir, ec);
    bool t3 = delete_file_secure(dir.string());
    std::cout << "Test3 delete directory: " << (!t3 ? "PASS" : "FAIL") << "\n";

    // Test 4: Empty path (expect false)
    bool t4 = delete_file_secure(std::string());
    std::cout << "Test4 empty path: " << (!t4 ? "PASS" : "FAIL") << "\n";

    // Test 5: Another temp file delete (expect true)
    fs::path t5p = make_temp_file();
    bool t5 = delete_file_secure(t5p.string());
    std::cout << "Test5 second temp file: " << (t5 ? "PASS" : "FAIL") << "\n";

    // Cleanup directory
    fs::remove_all(dir, ec);
    return 0;
}