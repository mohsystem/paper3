#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdio>

#ifdef _WIN32
#include <io.h>
#define POPEN _popen
#define PCLOSE _pclose
#else
#include <unistd.h>
#define POPEN popen
#define PCLOSE pclose
#endif

static std::string quote_posix(const std::string& path) {
    // Returns a safely single-quoted POSIX shell string
    std::string out;
    out.reserve(path.size() * 4 + 3);
    out.push_back('\'');
    for (char c : path) {
        if (c == '\'') {
            out.push_back('\'');
            out.push_back('\\');
            out.push_back('\'');
            out.push_back('\'');
        } else {
            out.push_back(c);
        }
    }
    out.push_back('\'');
    return out;
}

static std::string quote_windows(const std::string& path) {
    // Wrap in double-quotes and double any embedded quotes
    std::string out;
    out.reserve(path.size() * 2 + 3);
    out.push_back('"');
    for (char c : path) {
        if (c == '"') {
            out.push_back('"');
            out.push_back('"');
        } else {
            out.push_back(c);
        }
    }
    out.push_back('"');
    return out;
}

std::string displayFile(const std::string& filename) {
    std::string cmd;
#ifdef _WIN32
    cmd = "cmd /c type " + quote_windows(filename);
#else
    cmd = "cat " + quote_posix(filename);
#endif

    std::string output;
    FILE* fp = POPEN(cmd.c_str(), "rb");
    if (!fp) return output;

    char buffer[4096];
    size_t n;
    while ((n = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        output.append(buffer, n);
    }
    PCLOSE(fp);
    return output;
}

int main() {
    std::vector<std::pair<std::string, std::string>> tests = {
        {"cpp_file1.txt", "Hello from C++\nSecond line\n"},
        {"cpp_empty.txt", ""},
        {"cpp multi line.txt", "One\nTwo\nThree\n"},
        {"cpp_unicode.txt", "こんにちは世界\nПривет мир\nHola mundo\n"},
        {"cpp_numbers.txt", "100\n200\n300\n400\n500\n"}
    };

    for (const auto& kv : tests) {
        std::ofstream ofs(kv.first, std::ios::binary);
        ofs << kv.second;
    }

    for (const auto& kv : tests) {
        std::cout << "--- " << kv.first << " ---\n";
        std::cout << displayFile(kv.first);
    }

    return 0;
}