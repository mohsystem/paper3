#include <algorithm>
#include <cctype>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <utility>
#include <vector>
#include <filesystem>

static inline std::string ltrim(const std::string &s) {
    size_t i = 0;
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
    return s.substr(i);
}
static inline std::string rtrim(const std::string &s) {
    if (s.empty()) return s;
    size_t i = s.size();
    while (i > 0 && std::isspace(static_cast<unsigned char>(s[i - 1]))) --i;
    return s.substr(0, i);
}
static inline std::string trim(const std::string &s) {
    return rtrim(ltrim(s));
}

static inline bool isCommentOrEmpty(const std::string &s) {
    std::string t = trim(s);
    return t.empty() || (!t.empty() && (t[0] == '#' || t[0] == ';'));
}

std::vector<std::string> readAndSortKeyValueFile(const std::string &filePath) {
    if (filePath.empty()) {
        throw std::invalid_argument("filePath must not be empty");
    }
    std::ifstream in(filePath);
    if (!in) {
        throw std::runtime_error("Unable to open file");
    }
    const size_t MAX_LINE_LEN = 1048576;
    std::vector<std::pair<std::string, std::string>> pairs;
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back(); // handle CRLF
        if (line.size() > MAX_LINE_LEN) {
            continue; // skip overly long lines
        }
        if (isCommentOrEmpty(line)) continue;
        std::string t = trim(line);
        auto pos = t.find('=');
        if (pos == std::string::npos || pos == 0) continue;
        std::string key = trim(t.substr(0, pos));
        std::string value = trim(t.substr(pos + 1));
        if (key.empty()) continue;
        pairs.emplace_back(key, value);
    }
    std::sort(pairs.begin(), pairs.end(), [](const auto &a, const auto &b) {
        if (a.first != b.first) return a.first < b.first;
        return a.second < b.second;
    });
    std::vector<std::string> out;
    out.reserve(pairs.size());
    for (const auto &kv : pairs) {
        out.push_back(kv.first + "=" + kv.second);
    }
    return out;
}

static std::string createTempFileWithContent(const std::string &content) {
    namespace fs = std::filesystem;
    fs::path dir = fs::temp_directory_path();
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<unsigned long long> dist;
    std::string fname;
    fs::path p;
    for (int i = 0; i < 10; ++i) {
        fname = "task94_test_" + std::to_string(dist(gen)) + ".txt";
        p = dir / fname;
        std::error_code ec;
        if (!fs::exists(p, ec)) {
            std::ofstream out(p, std::ios::binary | std::ios::trunc);
            if (!out) continue;
            out.write(content.data(), static_cast<std::streamsize>(content.size()));
            out.close();
            return p.string();
        }
    }
    throw std::runtime_error("Failed to create temp file");
}

static void printResult(const std::string &title, const std::vector<std::string> &result) {
    std::cout << title << "\n";
    for (const auto &s : result) {
        std::cout << s << "\n";
    }
    std::cout << "----\n";
}

int main() {
    std::string c1 =
        "# Comment line\n"
        "a=1\n"
        "b= 2\n"
        " c =3 \n"
        "d=4\n"
        "invalidline\n"
        "=novalue\n"
        "e=\n"
        ";comment\n"
        "f = value with spaces  \n";
    std::string c2 =
        "z = last\n"
        "a = first\n"
        "m = middle\n"
        "a = duplicate\n"
        "x=42\n"
        "y=100\n";
    std::string c3 =
        " key = value=with=equals\n"
        " spaced key = spaced value \n"
        "# comment\n"
        "emptykey= \n"
        " = bad\n"
        "onlykey=\n";
    std::string c4 =
        "café=au lait\n"
        "naïve=façade\n"
        "日本=語\n"
        "ключ=значение\n";
    std::string c5 =
        "delta=4\r\n"
        "alpha=1\r\n"
        "charlie=3\r\n"
        "bravo=2\r\n"
        "alpha=0\r\n";

    std::string p1 = createTempFileWithContent(c1);
    std::string p2 = createTempFileWithContent(c2);
    std::string p3 = createTempFileWithContent(c3);
    std::string p4 = createTempFileWithContent(c4);
    std::string p5 = createTempFileWithContent(c5);

    try {
        printResult("Test 1", readAndSortKeyValueFile(p1));
        printResult("Test 2", readAndSortKeyValueFile(p2));
        printResult("Test 3", readAndSortKeyValueFile(p3));
        printResult("Test 4", readAndSortKeyValueFile(p4));
        printResult("Test 5", readAndSortKeyValueFile(p5));
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n";
    }

    std::error_code ec;
    std::filesystem::remove(p1, ec);
    std::filesystem::remove(p2, ec);
    std::filesystem::remove(p3, ec);
    std::filesystem::remove(p4, ec);
    std::filesystem::remove(p5, ec);
    return 0;
}