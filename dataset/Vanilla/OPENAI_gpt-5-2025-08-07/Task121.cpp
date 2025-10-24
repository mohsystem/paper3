#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

std::string upload_file(const std::string& source_path, const std::string& dest_dir, const std::string& new_name) {
    fs::path src(source_path);
    if (!fs::exists(src) || !fs::is_regular_file(src)) {
        throw std::runtime_error("Source file does not exist: " + source_path);
    }
    fs::path destDir(dest_dir);
    fs::create_directories(destDir);
    std::string finalName = new_name.size() > 0 ? new_name : src.filename().string();
    fs::path dest = destDir / finalName;
    fs::copy_file(src, dest, fs::copy_options::overwrite_existing);
    return fs::absolute(dest).string();
}

static void write_file(const fs::path& p, const std::string& content) {
    fs::create_directories(p.parent_path());
    std::ofstream ofs(p, std::ios::binary | std::ios::trunc);
    ofs << content;
}

int main() {
    try {
        fs::path fixtures("fixtures_cpp");
        fs::path uploads("uploads_cpp");
        fs::create_directories(fixtures);
        fs::create_directories(uploads);

        fs::path f1 = fixtures / "cfile1.txt";
        fs::path f2 = fixtures / "cfile2.txt";
        fs::path f3 = fixtures / "cfile3.txt";
        fs::path f4 = fixtures / "blob.bin";
        fs::path f5 = fixtures / "readme.md";

        write_file(f1, "Hello from C++ file1");
        write_file(f2, "Hello from C++ file2");
        write_file(f3, "Hello from C++ file3");
        write_file(f4, std::string("\x00\x01\x02BINARY", 8));
        write_file(f5, "# C++ Readme\nDetails...");

        std::cout << upload_file(f1.string(), uploads.string(), "") << std::endl;
        std::cout << upload_file(f2.string(), uploads.string(), "renamed_cfile2.txt") << std::endl;
        std::cout << upload_file(f3.string(), (uploads / "nestedX" / "nestedY").string(), "") << std::endl;
        std::cout << upload_file(f4.string(), uploads.string(), "blob_copy.bin") << std::endl;
        std::cout << upload_file(f5.string(), (uploads / "docs").string(), "readme_copy.md") << std::endl;
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
    return 0;
}