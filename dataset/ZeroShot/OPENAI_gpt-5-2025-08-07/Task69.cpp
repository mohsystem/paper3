#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <random>

namespace fs = std::filesystem;

bool deleteFileSecure(const std::string& path) {
    if (path.empty()) return false;
    std::error_code ec;
    fs::path p = fs::absolute(fs::path(path), ec);
    if (ec) return false;

    // Avoid deleting root paths
    if (p == p.root_path()) return false;

    if (!fs::exists(p, ec)) return false;
    if (ec) return false;

    if (fs::is_directory(p, ec)) return false; // refuse directories
    if (ec) return false;

    return fs::remove(p, ec) && !ec;
}

static fs::path make_temp_file(const std::string& prefix) {
    fs::path dir = fs::temp_directory_path();
    auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<unsigned long long> dist;
    fs::path p = dir / (prefix + std::to_string(now) + "_" + std::to_string(dist(gen)) + ".tmp");
    std::ofstream ofs(p.string(), std::ios::binary);
    ofs << "x";
    ofs.close();
    return p;
}

static fs::path make_temp_dir(const std::string& prefix) {
    fs::path dir = fs::temp_directory_path();
    auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    fs::path p = dir / (prefix + std::to_string(now));
    std::error_code ec;
    fs::create_directory(p, ec);
    return p;
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            bool ok = deleteFileSecure(argv[i]);
            std::cout << argv[i] << " -> " << (ok ? "true" : "false") << std::endl;
        }
    } else {
        fs::path f1 = make_temp_file("task69_test1_");
        fs::path f2 = make_temp_file("task69_test2_");
        fs::path f3 = make_temp_file("task69_test3_");
        fs::path d = make_temp_dir("task69_dir_");
        fs::path nonexist = d / "nonexistent_test.tmp";

        std::cout << "Test1 delete file: " << (deleteFileSecure(f1.string()) ? "true" : "false") << std::endl;
        std::cout << "Test2 delete file: " << (deleteFileSecure(f2.string()) ? "true" : "false") << std::endl;
        std::cout << "Test3 delete directory (should be false): " << (deleteFileSecure(d.string()) ? "true" : "false") << std::endl;
        std::cout << "Test4 delete non-existent (should be false): " << (deleteFileSecure(nonexist.string()) ? "true" : "false") << std::endl;
        std::cout << "Test5 delete file: " << (deleteFileSecure(f3.string()) ? "true" : "false") << std::endl;

        std::error_code ec;
        fs::remove(d, ec);
    }
    return 0;
}