#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <regex>
#include <filesystem>
#include <system_error>

namespace fs = std::filesystem;

static const std::size_t MAX_FILE_SIZE = 5u * 1024u * 1024u; // 5 MiB
static const std::size_t MAX_PATTERN_LENGTH = 10000u;
static const std::size_t MAX_MATCHES = 10000u;

static bool has_symlink_in_path(const fs::path& p) {
    try {
        fs::path base = p.is_absolute() ? fs::path(p.root_path()) : fs::current_path();
        fs::path rel = p.is_absolute() ? fs::relative(p, p.root_path()) : p;
        fs::path accum = base;
        for (const auto& part : rel) {
            accum /= part;
            std::error_code ec;
            if (fs::is_symlink(accum, ec)) {
                return true;
            }
        }
        return false;
    } catch (...) {
        return true;
    }
}

std::vector<std::string> search_file(const std::string& pattern, const std::string& filePath) {
    if (pattern.empty() || pattern.size() > MAX_PATTERN_LENGTH) {
        throw std::invalid_argument("Invalid pattern length");
    }

    fs::path base = fs::current_path();
    fs::path target = fs::path(filePath);
    fs::path absTarget = fs::absolute(target).lexically_normal();

    // Ensure within base directory
    fs::path absBase = base.lexically_normal();
    fs::path rel = absTarget.lexically_relative(absBase);
    if (rel.empty() || rel.string().rfind("..", 0) == 0) {
        // rel is empty when equal or error; if equal, allow
        if (absTarget != absBase) {
            throw std::invalid_argument("Path escapes base directory");
        }
    }

    // Reject symlinks in path
    if (has_symlink_in_path(absTarget)) {
        throw std::invalid_argument("Symbolic links are not allowed");
    }

    std::error_code ec;
    if (!fs::is_regular_file(absTarget, ec)) {
        throw std::invalid_argument("Not a regular file");
    }

    std::uintmax_t fsize = fs::file_size(absTarget, ec);
    if (ec || fsize > MAX_FILE_SIZE) {
        throw std::invalid_argument("File too large or inaccessible");
    }

    // Read file
    std::ifstream in(absTarget, std::ios::in | std::ios::binary);
    if (!in) {
        throw std::invalid_argument("Failed to open file");
    }
    std::string content;
    content.resize(static_cast<std::size_t>(fsize));
    if (fsize > 0) {
        in.read(&content[0], static_cast<std::streamsize>(fsize));
        if (!in) {
            throw std::invalid_argument("Failed to read file");
        }
    }

    // Compile regex
    std::regex re;
    try {
        re = std::regex(pattern);
    } catch (const std::regex_error& e) {
        throw std::invalid_argument(std::string("Invalid regex: ") + e.what());
    }

    // Find matches
    std::vector<std::string> results;
    results.reserve(16);
    try {
        std::sregex_iterator it(content.begin(), content.end(), re);
        std::sregex_iterator end;
        std::size_t count = 0;
        for (; it != end && count < MAX_MATCHES; ++it) {
            results.push_back((*it).str());
            count++;
            if (it->length() == 0) {
                // Avoid zero-length infinite loop: advance manually
                auto pos = static_cast<std::size_t>(it->position());
                if (pos < content.size()) {
                    auto next = content.begin() + static_cast<std::ptrdiff_t>(pos + 1);
                    it = std::sregex_iterator(next, content.end(), re);
                }
            }
        }
    } catch (...) {
        // If catastrophic backtracking or other issues occur
        throw std::invalid_argument("Regex search failed");
    }
    return results;
}

static void create_file(const fs::path& name, const std::string& content) {
    fs::path base = fs::current_path();
    fs::path p = fs::absolute(name).lexically_normal();
    fs::path rel = p.lexically_relative(base);
    if (rel.empty() || rel.string().rfind("..", 0) == 0) {
        if (p != base) {
            throw std::runtime_error("Refusing to create outside base directory");
        }
    }
    std::ofstream out(p, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!out) throw std::runtime_error("Failed to create file");
    out.write(content.data(), static_cast<std::streamsize>(content.size()));
    if (!out) throw std::runtime_error("Failed to write file");
}

static void print_results(const std::vector<std::string>& res) {
    std::cout << "Matches: " << res.size() << "\n";
    for (std::size_t i = 0; i < res.size(); ++i) {
        std::string s = res[i];
        std::string safe;
        safe.reserve(s.size());
        for (char c : s) {
            if (c == '\n') safe += "\\n";
            else if (c == '\r') safe += "\\r";
            else safe += c;
        }
        if (safe.size() > 200) {
            safe = safe.substr(0, 200) + "...";
        }
        std::cout << "[" << i << "] " << safe << "\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc == 3) {
        try {
            std::vector<std::string> res = search_file(argv[1], argv[2]);
            print_results(res);
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << "\n";
            return 1;
        }
        return 0;
    }

    try {
        create_file("cpp_test1.txt", "Hello world!\nThis is a test file.");
        create_file("cpp_test2.txt", "Numbers: 123 456 789\nEnd.");
        create_file("cpp_test3.txt", "First line.\nSecond line.\nThird line.");
        create_file("cpp_test4.txt", "Contacts: alice@example.com, bob@test.com");
        create_file("cpp_test5.txt", "No matching tokens here.");

        auto r1 = search_file("world", "cpp_test1.txt");
        std::cout << "Test1: pattern='world' file='cpp_test1.txt'\n";
        print_results(r1);

        auto r2 = search_file("\\d+", "cpp_test2.txt");
        std::cout << "Test2: pattern='\\\\d+' file='cpp_test2.txt'\n";
        print_results(r2);

        auto r3 = search_file("Second line\\.", "cpp_test3.txt");
        std::cout << "Test3: pattern='Second line\\.' file='cpp_test3.txt'\n";
        print_results(r3);

        auto r4 = search_file("[A-Za-z]+@[A-Za-z]+\\.com", "cpp_test4.txt");
        std::cout << "Test4: pattern='[A-Za-z]+@[A-Za-z]+\\\\.com' file='cpp_test4.txt'\n";
        print_results(r4);

        auto r5 = search_file("XYZ", "cpp_test5.txt");
        std::cout << "Test5: pattern='XYZ' file='cpp_test5.txt'\n";
        print_results(r5);
    } catch (const std::exception& e) {
        std::cout << "Test error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}