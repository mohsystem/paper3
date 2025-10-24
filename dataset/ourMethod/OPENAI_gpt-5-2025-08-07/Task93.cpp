#include <algorithm>
#include <cctype>
#include <cerrno>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

static std::string trim(const std::string &s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start])) != 0) start++;
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1])) != 0) end--;
    return s.substr(start, end - start);
}

static bool starts_with_path(const fs::path& base, const fs::path& target) {
    auto baseIt = base.begin();
    auto targetIt = target.begin();
    for (; baseIt != base.end() && targetIt != target.end(); ++baseIt, ++targetIt) {
        if (*baseIt != *targetIt) {
            return false;
        }
    }
    return baseIt == base.end();
}

static bool has_symlink_in_path(const fs::path& base, const fs::path& target) {
    fs::path cur;
    for (auto it = target.begin(); it != target.end(); ++it) {
        if (cur.empty()) {
            cur = *it;
        } else {
            cur /= *it;
        }
        // Only check within/under base prefix sequence
        fs::path absCur = cur.is_absolute() ? cur : fs::absolute(cur);
        std::error_code ec;
        auto st = fs::symlink_status(absCur, ec);
        if (ec) continue;
        if (fs::is_symlink(st)) {
            return true;
        }
        // Stop early if we've reached the full target path
    }
    return false;
}

static std::vector<std::pair<std::string, std::string>> read_and_sort_key_value_file(
    const std::string& base_dir,
    const std::string& relative_path,
    std::string& error_out
) {
    error_out.clear();
    std::vector<std::pair<std::string, std::string>> result;

    try {
        if (relative_path.empty() || relative_path.size() > 4096) {
            throw std::runtime_error("Invalid relative path length.");
        }
        if (!base_dir.size()) {
            throw std::runtime_error("Base directory must not be empty.");
        }
        fs::path rel(relative_path);
        if (rel.is_absolute()) {
            throw std::runtime_error("Absolute paths are not allowed.");
        }

        fs::path baseCanonical = fs::canonical(fs::path(base_dir));
        fs::path tentative = (baseCanonical / rel).lexically_normal();

        if (!starts_with_path(baseCanonical, tentative)) {
            throw std::runtime_error("Resolved path escapes base directory.");
        }

        // Refuse symlinks in the final file and in any component of the path
        std::error_code ec;
        auto st = fs::symlink_status(tentative, ec);
        if (ec) {
            throw std::runtime_error("File not found or inaccessible.");
        }
        if (fs::is_symlink(st)) {
            throw std::runtime_error("Refusing to process symlink.");
        }
        // Check for symlinks in parents; build absolute to evaluate correctly
        fs::path absTentative = tentative.is_absolute() ? tentative : fs::absolute(tentative);
        fs::path cur;
        for (auto it = absTentative.begin(); it != absTentative.end(); ++it) {
            if (cur.empty()) cur = *it;
            else cur /= *it;
            auto pst = fs::symlink_status(cur, ec);
            if (!ec && fs::is_symlink(pst)) {
                throw std::runtime_error("Refusing to process path containing symlink component.");
            }
        }

        if (!fs::is_regular_file(tentative)) {
            throw std::runtime_error("Not a regular file.");
        }

        std::ifstream in(tentative, std::ios::in | std::ios::binary);
        if (!in) {
            throw std::runtime_error("Failed to open file for reading.");
        }

        std::string line;
        while (std::getline(in, line)) {
            if (line.size() > 8192) {
                continue;
            }
            std::string trimmed;
            // remove '\r' if present (Windows newlines)
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            trimmed = trim(line);
            if (trimmed.empty() || (!trimmed.empty() && (trimmed[0] == '#' || trimmed[0] == ';'))) {
                continue;
            }
            auto pos = trimmed.find('=');
            if (pos == std::string::npos || pos == 0 || pos == trimmed.size() - 1) {
                continue;
            }
            std::string key = trim(trimmed.substr(0, pos));
            std::string val = trim(trimmed.substr(pos + 1));
            if (key.empty() || val.empty() || key.size() > 1024 || val.size() > 4096) {
                continue;
            }
            result.emplace_back(key, val);
        }

        std::sort(result.begin(), result.end(), [](const auto& a, const auto& b) {
            if (a.first < b.first) return true;
            if (a.first > b.first) return false;
            return a.second < b.second;
        });
    } catch (const std::exception& ex) {
        error_out = ex.what();
        result.clear();
    }
    return result;
}

static fs::path secure_write_file(const fs::path& base_dir, const fs::path& name, const std::vector<std::string>& lines) {
    fs::path baseCanonical = fs::canonical(base_dir);
    fs::path target = (baseCanonical / name).lexically_normal();
    if (!starts_with_path(baseCanonical, target)) {
        throw std::runtime_error("Write path escapes base directory.");
    }
    if (fs::exists(target)) {
        throw std::runtime_error("File already exists: " + target.string());
    }
    fs::create_directories(target.parent_path());
    std::ofstream out(target, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!out) {
        throw std::runtime_error("Failed to open file for writing: " + target.string());
    }
    for (const auto& line : lines) {
        out << line << "\n";
    }
    out.flush();
    if (!out) {
        throw std::runtime_error("Failed to write file: " + target.string());
    }
    return target;
}

int main() {
    try {
        fs::path base = fs::temp_directory_path() / fs::path("task93_kv_cpp");
        fs::create_directories(base);

        // Test 1: Normal case
        secure_write_file(base, "test1.txt", {"b=2", "a=1", "c=3"});
        std::string err1;
        auto r1 = read_and_sort_key_value_file(base.string(), "test1.txt", err1);
        if (!err1.empty()) std::cout << "Test1 error: " << err1 << "\n";
        else {
            std::cout << "Test1 sorted:\n";
            for (auto& kv : r1) std::cout << "  " << kv.first << "=" << kv.second << "\n";
        }

        // Test 2: Whitespace, comments, malformed lines
        secure_write_file(base, "test2.txt", {"   # comment", " b = x ", "a= y", "invalidline", "=novalue", "keyonly="});
        std::string err2;
        auto r2 = read_and_sort_key_value_file(base.string(), "test2.txt", err2);
        if (!err2.empty()) std::cout << "Test2 error: " << err2 << "\n";
        else {
            std::cout << "Test2 sorted:\n";
            for (auto& kv : r2) std::cout << "  " << kv.first << "=" << kv.second << "\n";
        }

        // Test 3: Duplicate keys and varying values
        secure_write_file(base, "test3.txt", {"dup=2", "dup=1", "alpha=zz", "alpha=aa"});
        std::string err3;
        auto r3 = read_and_sort_key_value_file(base.string(), "test3.txt", err3);
        if (!err3.empty()) std::cout << "Test3 error: " << err3 << "\n";
        else {
            std::cout << "Test3 sorted:\n";
            for (auto& kv : r3) std::cout << "  " << kv.first << "=" << kv.second << "\n";
        }

        // Test 4: Path traversal attempt
        std::string err4;
        auto r4 = read_and_sort_key_value_file(base.string(), "../outside.txt", err4);
        if (!err4.empty()) std::cout << "Test4 caught expected error: " << err4 << "\n";
        else {
            std::cout << "Test4 unexpected success, records: " << r4.size() << "\n";
        }

        // Test 5: Absolute path attempt
        secure_write_file(base, "test5.txt", {"z=last", "m=mid", "a=first"});
        std::string err5;
        fs::path abs = fs::absolute(base / "test5.txt");
        auto r5 = read_and_sort_key_value_file(base.string(), abs.string(), err5);
        if (!err5.empty()) std::cout << "Test5 caught expected error: " << err5 << "\n";
        else {
            std::cout << "Test5 unexpected success, records: " << r5.size() << "\n";
        }

    } catch (const std::exception& ex) {
        std::cerr << "Fatal error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}