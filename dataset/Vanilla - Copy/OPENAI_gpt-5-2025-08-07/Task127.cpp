#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

std::string copyToTemp(const std::string& sourcePath) {
    if (sourcePath.empty()) {
        std::cerr << "[C++] Error: sourcePath is empty.\n";
        return "";
    }
    try {
        fs::path src(sourcePath);
        if (!fs::exists(src)) {
            std::cerr << "[C++] Error: Source file does not exist: " << sourcePath << "\n";
            return "";
        }
        if (fs::is_directory(src)) {
            std::cerr << "[C++] Error: Source path is a directory: " << sourcePath << "\n";
            return "";
        }
        fs::path tempDir = fs::temp_directory_path();
        fs::path dest = tempDir / fs::path(src.filename().string() + "-" + fs::unique_path().string() + ".tmp");
        fs::copy_file(src, dest, fs::copy_options::overwrite_existing);
        return dest.string();
    } catch (const std::exception& e) {
        std::cerr << "[C++] Exception while copying to temp: " << e.what() << "\n";
        return "";
    }
}

std::string createSampleFile(const std::string& content) {
    try {
        fs::path tempDir = fs::temp_directory_path();
        fs::path path = tempDir / fs::path("task127-sample-" + fs::unique_path().string() + ".txt");
        std::ofstream out(path, std::ios::binary);
        out.write(content.data(), static_cast<std::streamsize>(content.size()));
        out.close();
        return path.string();
    } catch (const std::exception& e) {
        std::cerr << "[C++] Failed to create sample file: " << e.what() << "\n";
        return "";
    }
}

bool filesEqual(const std::string& a, const std::string& b) {
    try {
        std::ifstream fa(a, std::ios::binary);
        std::ifstream fb(b, std::ios::binary);
        if (!fa.good() || !fb.good()) return false;
        std::vector<char> ba(8192), bb(8192);
        while (true) {
            fa.read(ba.data(), ba.size());
            fb.read(bb.data(), bb.size());
            std::streamsize ra = fa.gcount();
            std::streamsize rb = fb.gcount();
            if (ra != rb) return false;
            if (ra == 0) return true;
            if (!std::equal(ba.begin(), ba.begin() + ra, bb.begin())) return false;
        }
    } catch (...) {
        std::cerr << "[C++] Failed to compare files.\n";
        return false;
    }
}

int main() {
    // Test 1: Valid small file
    std::string src1 = createSampleFile("Hello, Task127!");
    std::string dst1 = copyToTemp(src1);
    std::cout << "[C++][Test1] src=" << src1 << " dst=" << dst1 << " equal=" << (dst1.size() && filesEqual(src1, dst1) ? "true" : "false") << "\n";

    // Test 2: Non-existent file
    fs::path src2 = fs::temp_directory_path() / "nonexistent_12345_abcdef.txt";
    std::string dst2 = copyToTemp(src2.string());
    std::cout << "[C++][Test2] src=" << src2.string() << " dst=" << dst2 << "\n";

    // Test 3: Directory instead of file
    std::string src3 = fs::temp_directory_path().string();
    std::string dst3 = copyToTemp(src3);
    std::cout << "[C++][Test3] src=" << src3 << " dst=" << dst3 << "\n";

    // Test 4: Empty string path
    std::string dst4 = copyToTemp("");
    std::cout << "[C++][Test4] src='' dst=" << dst4 << "\n";

    // Test 5: Larger content file
    std::string big;
    big.reserve(200000);
    for (int i = 0; i < 10000; ++i) {
        big += "Line " + std::to_string(i) + " - Task127\n";
    }
    std::string src5 = createSampleFile(big);
    std::string dst5 = copyToTemp(src5);
    std::cout << "[C++][Test5] src=" << src5 << " dst=" << dst5 << " equal=" << (dst5.size() && filesEqual(src5, dst5) ? "true" : "false") << "\n";

    return 0;
}