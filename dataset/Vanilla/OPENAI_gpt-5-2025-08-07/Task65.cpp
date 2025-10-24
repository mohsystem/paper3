#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <cstdio>
#include <cctype>

static bool isValidFilename(const std::string& name) {
    const std::string invalid = "<>\"|?*";
    for (unsigned char c : name) {
        if (c < 32) return false;
        if (invalid.find(static_cast<char>(c)) != std::string::npos) return false;
    }
    // Basic component length check
    size_t start = 0;
    while (start <= name.size()) {
        size_t pos = name.find_first_of("/\\", start);
        size_t len = (pos == std::string::npos) ? (name.size() - start) : (pos - start);
        if (len > 255) return false;
        if (pos == std::string::npos) break;
        start = pos + 1;
    }
    return true;
}

std::string readFile(const std::string& filename) {
    if (filename.empty()) {
        return "ERROR: Filename is empty.";
    }
    if (!isValidFilename(filename)) {
        return "ERROR: Filename contains invalid characters.";
    }

    struct stat st;
    if (stat(filename.c_str(), &st) != 0) {
        return "ERROR: File does not exist.";
    }
    if (S_ISDIR(st.st_mode)) {
        return "ERROR: Path is a directory, not a file.";
    }

    std::ifstream file(filename, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        return "ERROR: File is not readable.";
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

static std::string createTempFileWithContent(const std::string& content) {
    char nameBuf[L_tmpnam];
    if (std::tmpnam(nameBuf) == nullptr) {
        return "";
    }
    std::string path = nameBuf;
    std::ofstream out(path, std::ios::out | std::ios::binary);
    if (!out.is_open()) return "";
    out.write(content.data(), static_cast<std::streamsize>(content.size()));
    out.close();
    return path;
}

int main(int argc, char** argv) {
    if (argc > 1) {
        std::cout << "CLI arg result:\n";
        std::cout << readFile(argv[1]) << "\n";
    }

    std::vector<std::string> tests;

    // 1) Valid temp file
    tests.push_back(createTempFileWithContent("Hello from Task65 test case 1.\nLine 2."));

    // 2) Valid temp file with larger content
    {
        std::ostringstream oss;
        for (int i = 0; i < 100; ++i) {
            oss << "Line " << (i + 1) << ": Sample content\n";
        }
        tests.push_back(createTempFileWithContent(oss.str()));
    }

    // 3) Non-existent file
    tests.push_back("this_file_should_not_exist_1234567890.txt");

    // 4) Empty filename
    tests.push_back("");

    // 5) Invalid filename
    tests.push_back("bad|name.txt");

    for (size_t i = 0; i < tests.size(); ++i) {
        std::cout << "Test " << (i + 1) << " (" << tests[i] << "):\n";
        std::cout << readFile(tests[i]) << "\n";
    }

    return 0;
}