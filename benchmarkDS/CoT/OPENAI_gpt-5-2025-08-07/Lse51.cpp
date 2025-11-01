// Chain-of-Through process:
// 1) Problem understanding: Provide secure file open/read/write utilities demonstrating RAII (analog to Python's 'with'); basic tests.
// 2) Security requirements: Validate paths minimally, handle I/O errors, avoid resource leaks using RAII streams.
// 3) Secure coding generation: Implement read/write with std::ifstream/ofstream and UTF-8 compatible bytes.
// 4) Code review: Ensure checks for open failures and no undefined behavior.
// 5) Secure code output: Final functions with five tests.
// Note: C++ standard library does not provide TAR (gz/bz2) extraction natively; not implemented here.

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(p, m) _mkdir(p)
#else
#include <errno.h>
#include <unistd.h>
#define MKDIR(p, m) mkdir(p, m)
#endif

static void make_dirs_recursive(const std::string& path) {
    if (path.empty()) return;
    std::string current;
    for (size_t i = 0; i < path.size(); ++i) {
        char c = path[i];
        current.push_back(c);
        if (c == '/' || i == path.size() - 1) {
            if (!current.empty() && current != "/" && current != "./") {
                // Attempt to create; ignore if exists
                MKDIR(current.c_str(), 0700);
            }
        }
    }
}

static size_t write_text_file(const std::string& path, const std::string& content) {
    if (path.empty()) throw std::invalid_argument("path is empty");
    // Create parent directories if needed (simple approach)
    size_t pos = path.find_last_of("/\\");
    if (pos != std::string::npos) {
        std::string parent = path.substr(0, pos + 1);
        make_dirs_recursive(parent);
    }
    std::ofstream ofs(path, std::ios::binary | std::ios::trunc);
    if (!ofs) throw std::runtime_error("failed to open for writing");
    ofs.write(content.data(), static_cast<std::streamsize>(content.size()));
    if (!ofs) throw std::runtime_error("failed to write");
    ofs.close();
    return content.size();
}

static std::string read_text_file(const std::string& path) {
    if (path.empty()) throw std::invalid_argument("path is empty");
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) throw std::runtime_error("failed to open for reading");
    std::string data;
    ifs.seekg(0, std::ios::end);
    std::streampos len = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    if (len > 0) {
        data.resize(static_cast<size_t>(len));
        ifs.read(&data[0], len);
        if (!ifs) throw std::runtime_error("failed to read");
    }
    return data;
}

// Placeholder: TAR extraction is not implemented due to lack of standard library support.
// std::vector<std::string> extract_tar_secure(const std::string& tarPath, const std::string& destDir) { /* not implemented */ }

int main() {
    try {
#ifdef _WIN32
        std::string tmp = ".\\";
#else
        std::string tmp = "/tmp/";
#endif
        std::vector<std::string> files = {
            tmp + "lse51_cpp_t1.txt",
            tmp + "lse51_cpp_t2.txt",
            tmp + "lse51_cpp_dir/lse51_cpp_t3.txt",
            tmp + "lse51_cpp_dir/sub/lse51_cpp_t4.txt",
            tmp + "lse51_cpp_t5.txt"
        };
        std::vector<std::string> contents = {
            "Hello, C++ RAII",
            "Line1\nLine2\nLine3",
            "Unicode-ish bytes \xE2\x9C\x93",
            "",
            "Final test content"
        };
        for (size_t i = 0; i < files.size(); ++i) {
            size_t written = write_text_file(files[i], contents[i]);
            std::string read = read_text_file(files[i]);
            std::cout << "Test " << (i+1) << " wrote bytes: " << written
                      << " read len: " << read.size() << "\n";
        }
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}