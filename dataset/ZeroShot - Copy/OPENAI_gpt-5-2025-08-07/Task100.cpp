#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <filesystem>
#include <chrono>
#include <random>

namespace fs = std::filesystem;

static const std::uintmax_t MAX_BYTES = 5u * 1024u * 1024u; // 5 MB
static const std::size_t MAX_PATTERN_LEN = 10000;

std::vector<std::string> searchInFile(const std::string& pattern, const std::string& filePath) {
    if (pattern.size() > MAX_PATTERN_LEN) {
        throw std::runtime_error("Pattern too long.");
    }
    fs::path p(filePath);
    if (fs::is_symlink(p)) {
        throw std::runtime_error("Refusing to follow symbolic links.");
    }
    std::error_code ec;
    if (!fs::is_regular_file(p, ec) || ec) {
        throw std::runtime_error("Not a regular file.");
    }
    auto size = fs::file_size(p, ec);
    if (ec) {
        throw std::runtime_error("Unable to get file size.");
    }
    if (size > MAX_BYTES) {
        throw std::runtime_error("File too large.");
    }
    std::ifstream in(filePath, std::ios::in | std::ios::binary);
    if (!in) {
        throw std::runtime_error("Cannot open file.");
    }
    std::string content;
    content.resize(static_cast<std::size_t>(size));
    in.read(content.data(), static_cast<std::streamsize>(content.size()));
    content.resize(static_cast<std::size_t>(in.gcount()));

    std::vector<std::string> matches;
    try {
        std::regex re(pattern);
        auto begin = std::sregex_iterator(content.begin(), content.end(), re);
        auto end = std::sregex_iterator();
        for (auto it = begin; it != end; ++it) {
            matches.push_back(it->str());
        }
    } catch (const std::regex_error& e) {
        throw std::runtime_error(std::string("Invalid regex pattern: ") + e.what());
    }
    return matches;
}

static fs::path makeTempFileWithContent(const std::string& content) {
    fs::path dir = fs::temp_directory_path();
    // Create a pseudo-random filename
    auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::mt19937_64 rng(static_cast<unsigned long long>(now));
    unsigned long long r = rng();
    fs::path file = dir / ("task100_" + std::to_string(r) + ".txt");
    std::ofstream out(file, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!out) {
        throw std::runtime_error("Failed to create temp file.");
    }
    out.write(content.data(), static_cast<std::streamsize>(content.size()));
    out.close();
    return file;
}

int main(int argc, char** argv) {
    if (argc == 3) {
        try {
            auto matches = searchInFile(argv[1], argv[2]);
            std::cout << "Matches found: " << matches.size() << "\n";
            for (const auto& m : matches) {
                std::cout << m << "\n";
            }
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
            return 1;
        }
        return 0;
    }

    // Run 5 test cases
    std::string content = "The quick brown fox jumps over the lazy dog.\nNumbers: 12345 and 67890.\nTHE end.\n";
    std::vector<std::string> patterns = {
        "fox",
        "[A-Za-z]{4}",
        "[Tt][Hh][Ee]",
        "[0-9]+",
        "notfound"
    };
    fs::path tmp;
    try {
        tmp = makeTempFileWithContent(content);
        std::cout << "Test file: " << tmp.string() << "\n";
        for (std::size_t i = 0; i < patterns.size(); ++i) {
            std::cout << "Test " << (i + 1) << " Pattern: " << patterns[i] << "\n";
            try {
                auto res = searchInFile(patterns[i], tmp.string());
                std::cout << "Matches found: " << res.size() << "\n";
                for (const auto& m : res) {
                    std::cout << m << "\n";
                }
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << "\n";
            }
            std::cout << "---\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Test error: " << e.what() << "\n";
    }
    if (!tmp.empty()) {
        std::error_code ec;
        fs::remove(tmp, ec);
    }
    return 0;
}