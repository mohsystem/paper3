#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>

static const std::uintmax_t MAX_SIZE = 10u * 1024u * 1024u; // 10MB

std::string readFileSecure(const std::string& inputPath, bool& ok) {
    ok = false;
    try {
        namespace fs = std::filesystem;
        fs::path base = fs::canonical(fs::current_path());
        std::string baseStr = base.generic_string();
        if (!baseStr.empty() && baseStr.back() != '/') baseStr.push_back('/');

        fs::path targetCanonical = fs::canonical(fs::path(inputPath));
        std::string targStr = targetCanonical.generic_string();

        // Ensure target is within base directory boundary
        if (targStr.compare(0, baseStr.size(), baseStr) != 0) {
            return {};
        }

        if (!fs::is_regular_file(targetCanonical)) {
            return {};
        }

        std::uintmax_t size = fs::file_size(targetCanonical);
        if (size > MAX_SIZE) {
            return {};
        }

        std::ifstream in(targetCanonical, std::ios::binary);
        if (!in) {
            return {};
        }

        std::string data;
        data.resize(static_cast<size_t>(size));
        in.read(&data[0], static_cast<std::streamsize>(data.size()));
        if (!in && !in.eof()) {
            return {};
        }

        ok = true;
        return data;
    } catch (...) {
        return {};
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        for (int i = 1; i < argc; ++i) {
            std::string path = argv[i];
            bool ok = false;
            std::string content = readFileSecure(path, ok);
            std::cout << "----- " << path << " -----\n";
            if (ok) {
                std::cout << content << "\n";
            } else {
                std::cout << "ERROR: Unable to read file securely.\n";
            }
        }
        return 0;
    }

    // 5 test cases
    try {
        {
            std::ofstream f("cpp_t1.txt", std::ios::binary);
            f << "C++ Test 1: Hello World";
        }

        std::filesystem::create_directories("cpp_subdir");
        {
            std::ofstream f("cpp_subdir/cpp_t2.txt", std::ios::binary);
            f << "C++ Test 2: Inside subdir";
        }

        std::vector<std::string> tests = {
            "cpp_t1.txt",                 // valid
            "cpp_subdir/cpp_t2.txt",      // valid in subdir
            "cpp_no_such.txt",            // non-existent
            "cpp_subdir",                 // directory
            "../cpp_outside.txt"          // escape attempt
        };

        for (const auto& p : tests) {
            bool ok = false;
            std::string content = readFileSecure(p, ok);
            std::cout << "===== Testing: " << p << " =====\n";
            if (ok) {
                std::cout << content << "\n";
            } else {
                std::cout << "ERROR: Unable to read file securely.\n";
            }
        }
    } catch (const std::exception& e) {
        std::cout << "ERROR: Test setup failed: " << e.what() << "\n";
    }
    return 0;
}