#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <utility>
#include <algorithm>
#include <sys/stat.h>

// Step 1-5 implemented succinctly with validation and safe parsing.

static inline std::string trim(const std::string &s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) start++;
    if (start == s.size()) return "";
    size_t end = s.size() - 1;
    while (end > start && std::isspace(static_cast<unsigned char>(s[end]))) end--;
    return s.substr(start, end - start + 1);
}

std::vector<std::pair<std::string, std::string>> read_and_sort_key_values(const std::string &file_path) {
    const long long MAX_FILE_SIZE = 10LL * 1024LL * 1024LL; // 10 MB
    const size_t MAX_KEY_LEN = 4096;
    const size_t MAX_VALUE_LEN = 2000000;

    struct stat st;
    if (stat(file_path.c_str(), &st) != 0) {
        throw std::runtime_error("File not found: " + file_path);
    }
    if ((st.st_mode & S_IFMT) == S_IFDIR) {
        throw std::runtime_error("Path is a directory: " + file_path);
    }
    if (st.st_size > MAX_FILE_SIZE) {
        throw std::runtime_error("File too large");
    }

    std::ifstream in(file_path);
    if (!in) {
        throw std::runtime_error("Unable to open file: " + file_path);
    }

    std::map<std::string, std::string> sorted;
    long long totalValueBytes = 0;
    std::string raw;
    while (std::getline(in, raw)) {
        std::string line = trim(raw);
        if (line.empty() || line.rfind("#", 0) == 0 || line.rfind("//", 0) == 0) continue;

        auto posEq = line.find('=');
        auto posCol = line.find(':');
        size_t pos;
        if (posEq == std::string::npos && posCol == std::string::npos) continue;
        else if (posEq == std::string::npos) pos = posCol;
        else if (posCol == std::string::npos) pos = posEq;
        else pos = std::min(posEq, posCol);

        std::string key = trim(line.substr(0, pos));
        std::string value = trim(line.substr(pos + 1));
        if (key.empty()) continue;
        if (key.size() > MAX_KEY_LEN) throw std::runtime_error("Key too long");
        if (value.size() > MAX_VALUE_LEN) throw std::runtime_error("Value too long");

        totalValueBytes += static_cast<long long>(value.size());
        if (totalValueBytes > MAX_FILE_SIZE) throw std::runtime_error("Accumulated values exceed safe threshold");

        sorted[key] = value; // last wins
    }

    std::vector<std::pair<std::string, std::string>> res;
    res.reserve(sorted.size());
    for (const auto &kv : sorted) res.emplace_back(kv.first, kv.second);
    return res;
}

static void write_file(const std::string &path, const std::string &content) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out) throw std::runtime_error("Cannot write file: " + path);
    out << content;
}

static void run_test(const std::string &title, const std::string &filename, const std::string &content) {
    write_file(filename, content);
    auto res = read_and_sort_key_values(filename);
    std::cout << "=== " << title << " (" << filename << ") ===\n";
    for (const auto &p : res) {
        std::cout << p.first << "=" << p.second << "\n";
    }
}

int main() {
    try {
        run_test("Test 1 - Basic", "cpp_test_kv_1.txt",
                 "banana=yellow\napple=red\ncarrot=orange\n");
        run_test("Test 2 - Unsorted with spaces", "cpp_test_kv_2.txt",
                 "  zeta = 26 \nalpha=1\nbeta = 2\n  gamma= 3\n");
        run_test("Test 3 - Duplicates", "cpp_test_kv_3.txt",
                 "a=1\na=2\nb=3\nA=upper\n");
        run_test("Test 4 - Mixed separators and comments", "cpp_test_kv_4.txt",
                 "# Comment line\n// Another comment\nhost: localhost\nport=8080\n user : admin \npassword = secret\n");
        run_test("Test 5 - Special characters", "cpp_test_kv_5.txt",
                 "path=/usr/local/bin\nkey.with.dots=value.with.dots\n spaced key = spaced value \nemoji=🙂\ncolon:used\n");
    } catch (const std::exception &ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}