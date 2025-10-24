#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>

bool deleteFile(const std::string& path) {
    try {
        return std::filesystem::remove(path);
    } catch (...) {
        return false;
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            bool res = deleteFile(argv[i]);
            std::cout << argv[i] << " -> " << (res ? "true" : "false") << std::endl;
        }
        return 0;
    }

    // 5 test cases
    std::vector<std::string> paths;
    auto tmpdir = std::filesystem::temp_directory_path();

    // 1-3: create temp files to delete
    auto f1 = tmpdir / std::filesystem::unique_path("Task69_test_%%%%%%%%.tmp");
    auto f2 = tmpdir / std::filesystem::unique_path("Task69_test_%%%%%%%%.tmp");
    auto f3 = tmpdir / std::filesystem::unique_path("Task69_test_%%%%%%%%.tmp");

    { std::ofstream(f1.string()) << "data1"; }
    { std::ofstream(f2.string()) << "data2"; }
    { std::ofstream(f3.string()) << "data3"; }

    paths.push_back(f1.string()); // 1
    paths.push_back(f2.string()); // 2
    paths.push_back(f3.string()); // 3

    // 4: non-existent file
    auto nonExistent = tmpdir / std::filesystem::path("Task69_nonexistent_" + std::to_string(std::chrono::high_resolution_clock::now().time_since_epoch().count()) + ".tmp");
    paths.push_back(nonExistent.string());

    // 5: try deleting f1 again after it's been deleted
    paths.push_back(f1.string());

    for (const auto& p : paths) {
        bool res = deleteFile(p);
        std::cout << p << " -> " << (res ? "true" : "false") << std::endl;
    }

    return 0;
}