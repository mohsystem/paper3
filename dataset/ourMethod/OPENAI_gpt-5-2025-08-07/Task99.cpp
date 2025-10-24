#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <filesystem>

namespace fs = std::filesystem;

static bool is_subpath(const fs::path& base, const fs::path& target) {
    auto b = base.lexically_normal();
    auto t = target.lexically_normal();
    auto bit = b.begin(), bend = b.end();
    auto tit = t.begin(), tend = t.end();
    for (; bit != bend && tit != tend; ++bit, ++tit) {
        if (*bit != *tit) return false;
    }
    return bit == bend; // base fully matched as a prefix of target
}

std::vector<std::string> findMatchingLines(const std::string& baseDir, const std::string& userPath) {
    if (userPath.empty() || userPath.size() > 4096) {
        throw std::invalid_argument("Invalid path length.");
    }

    fs::path base = fs::absolute(fs::path(baseDir)).lexically_normal();
    if (!fs::exists(base) || !fs::is_directory(base)) {
        throw std::invalid_argument("Base directory does not exist or is not a directory.");
    }

    fs::path up(userPath);
    if (up.is_absolute()) {
        throw std::invalid_argument("Absolute paths are not allowed.");
    }

    fs::path resolved = (base / up).lexically_normal();
    // Use canonical for comparison to handle ".." and symlinks for directory traversal checks
    fs::path base_canon = fs::canonical(base);
    fs::path resolved_canon;
    try {
        resolved_canon = fs::canonical(resolved);
    } catch (const fs::filesystem_error&) {
        throw std::invalid_argument("File does not exist.");
    }

    if (!is_subpath(base_canon, resolved_canon)) {
        throw std::invalid_argument("Resolved path escapes the base directory.");
    }

    // Refuse symlink for the target and ensure regular file
    if (fs::is_symlink(fs::symlink_status(resolved))) {
        throw std::invalid_argument("Refusing to process symbolic links.");
    }
    if (!fs::is_regular_file(resolved)) {
        throw std::invalid_argument("Path is not a regular file.");
    }

    std::ifstream in(resolved, std::ios::in);
    if (!in) {
        throw std::runtime_error("Failed to open file.");
    }

    std::regex pattern(R"(^\d.*\.$)");
    std::vector<std::string> matches;
    std::string line;
    while (std::getline(in, line)) {
        if (std::regex_match(line, pattern)) {
            matches.push_back(line);
        }
    }
    return matches;
}

static void writeFile(const fs::path& base, const fs::path& name, const std::vector<std::string>& lines) {
    fs::create_directories((base / name).parent_path());
    std::ofstream out(base / name, std::ios::out | std::ios::trunc);
    for (size_t i = 0; i < lines.size(); ++i) {
        out << lines[i];
        if (i + 1 < lines.size()) out << "\n";
    }
}

static void printResult(const std::string& title, const std::vector<std::string>& res) {
    std::cout << "=== " << title << " ===\n";
    if (res.empty()) {
        std::cout << "(no matches)\n";
    } else {
        for (const auto& s : res) {
            std::cout << s << "\n";
        }
    }
}

int main() {
    try {
        fs::path base = fs::temp_directory_path() / "task99_cpp_base";
        fs::create_directories(base);

        writeFile(base, "t1.txt", {"123 start and end.", "Nope", "9.", "0x23.", "abc."});
        writeFile(base, "t2.txt", {"A1.", "1 but no dot", "44.", "7!"});
        writeFile(base, "t3.txt", {"...", "1a.", "", "2."});
        fs::create_directories(base / "subdir");
        writeFile(base, "subdir/inner.txt", {"777.", "x7."});

        // 5 test cases
        try {
            printResult("Test1 t1.txt", findMatchingLines(base.string(), "t1.txt"));
        } catch (const std::exception& e) {
            std::cout << "Test1 error: " << e.what() << "\n";
        }

        try {
            printResult("Test2 t2.txt", findMatchingLines(base.string(), "t2.txt"));
        } catch (const std::exception& e) {
            std::cout << "Test2 error: " << e.what() << "\n";
        }

        try {
            printResult("Test3 t3.txt", findMatchingLines(base.string(), "t3.txt"));
        } catch (const std::exception& e) {
            std::cout << "Test3 error: " << e.what() << "\n";
        }

        try {
            printResult("Test4 subdir (directory should be rejected)", findMatchingLines(base.string(), "subdir"));
        } catch (const std::exception& e) {
            std::cout << "Test4 error: " << e.what() << "\n";
        }

        try {
            printResult("Test5 path traversal (should be rejected)", findMatchingLines(base.string(), "../t1.txt"));
        } catch (const std::exception& e) {
            std::cout << "Test5 error: " << e.what() << "\n";
        }

        try {
            printResult("Extra nested file", findMatchingLines(base.string(), "subdir/inner.txt"));
        } catch (const std::exception& e) {
            std::cout << "Extra error: " << e.what() << "\n";
        }

    } catch (const std::exception& e) {
        std::cout << "Fatal error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}