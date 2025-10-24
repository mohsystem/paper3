#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <system_error>

namespace fs = std::filesystem;

std::string readFileAndPrint(const std::string& path) {
    const std::size_t MAX_BYTES = 10u * 1024u * 1024u; // 10 MB
    if (path.empty()) {
        std::cerr << "Error: file path is empty.\n";
        return std::string();
    }

    std::error_code ec;
    fs::path p(path);
    fs::file_status st = fs::symlink_status(p, ec);
    if (ec) {
        std::cerr << "Error: unable to access file metadata.\n";
        return std::string();
    }
    if (!fs::is_regular_file(st)) {
        std::cerr << "Error: not a regular file or is a symlink.\n";
        return std::string();
    }

    std::ifstream in(path, std::ios::binary);
    if (!in) {
        std::cerr << "Error: cannot open file for reading.\n";
        return std::string();
    }

    std::string out;
    out.reserve(8192);
    std::vector<char> buf(8192);
    std::size_t total = 0;

    while (in) {
        in.read(buf.data(), static_cast<std::streamsize>(buf.size()));
        std::streamsize got = in.gcount();
        if (got > 0) {
            total += static_cast<std::size_t>(got);
            if (total > MAX_BYTES) {
                std::cerr << "Error: file exceeds maximum allowed size of 10 MB.\n";
                return std::string();
            }
            out.append(buf.data(), static_cast<std::size_t>(got));
        }
    }

    std::cout << out;
    std::cout.flush();
    return out;
}

int main() {
    // Create 4 temporary files for testing and 1 non-existent path
    std::vector<fs::path> paths;
    try {
        fs::path tdir = fs::temp_directory_path();
        fs::path t1 = tdir / fs::unique_path("task36-%%%%-%%%%.txt");
        fs::path t2 = tdir / fs::unique_path("task36-%%%%-%%%%.txt");
        fs::path t3 = tdir / fs::unique_path("task36-%%%%-%%%%.txt");
        fs::path t4 = tdir / fs::unique_path("task36-%%%%-%%%%.txt");

        {
            std::ofstream o(t1, std::ios::binary);
            o << "Hello, Task36!\n";
        }
        {
            std::ofstream o(t2, std::ios::binary);
            // empty file
        }
        {
            std::ofstream o(t3, std::ios::binary);
            o << "こんにちは世界\nEmoji: 😀\n";
        }
        {
            std::ofstream o(t4, std::ios::binary);
            o << "Line1\nLine2\nLine3\n";
        }

        fs::path nonExistent = tdir / fs::path("does_not_exist_" + std::to_string(::time(nullptr)) + ".txt");

        paths = { t1, t2, t3, t4, nonExistent };
    } catch (...) {
        std::cerr << "Error: failed to set up test files.\n";
        return 1;
    }

    // Run 5 test cases
    for (const auto& p : paths) {
        readFileAndPrint(p.string());
    }

    return 0;
}