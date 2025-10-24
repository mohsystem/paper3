#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

static inline void rtrim(std::string& s) {
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.pop_back();
}
static inline void ltrim(std::string& s) {
    size_t i = 0;
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
    if (i > 0) s.erase(0, i);
}
static inline void trim(std::string& s) {
    rtrim(s);
    ltrim(s);
}

std::vector<std::string> readAndSortKeyValues(const std::string& filePath) {
    std::ifstream in(filePath, std::ios::in);
    std::vector<std::pair<std::string, std::string>> items;
    std::string line;
    while (std::getline(in, line)) {
        std::string t = line;
        trim(t);
        if (t.empty()) continue;
        if (!t.empty() && (t[0] == '#' || t[0] == ';')) continue;
        auto pos = t.find('=');
        if (pos == std::string::npos || pos == 0) continue;
        std::string key = t.substr(0, pos);
        std::string val = t.substr(pos + 1);
        trim(key);
        trim(val);
        if (key.empty()) continue;
        items.emplace_back(std::move(key), std::move(val));
    }
    std::sort(items.begin(), items.end(), [](const auto& a, const auto& b) {
        if (a.first == b.first) return a.second < b.second;
        return a.first < b.first;
    });
    std::vector<std::string> out;
    out.reserve(items.size());
    for (const auto& kv : items) {
        out.emplace_back(kv.first + "=" + kv.second);
    }
    return out;
}

static void writeFile(const std::string& path, const std::string& content) {
    std::ofstream out(path, std::ios::out | std::ios::trunc);
    out << content;
    out.flush();
}

int main() {
    std::string f1 = "task93_test1.txt";
    std::string f2 = "task93_test2.txt";
    std::string f3 = "task93_test3.txt";
    std::string f4 = "task93_test4.txt";
    std::string f5 = "task93_test5.txt";

    writeFile(f1, "b=2\na=1\nc=3\n");
    writeFile(f2, "  x = 10 \n\n=bad\n y= 5  \n  # comment\n");
    writeFile(f3, "a=2\na=1\n");
    writeFile(f4, "# comment\nfoo=bar\ninvalid line\nk=v=extra\n; another comment\n");
    writeFile(f5, "Apple=1\napple=2\nß=sharp\nss=double\n A=9 \n");

    std::vector<std::string> files = {f1, f2, f3, f4, f5};
    for (size_t i = 0; i < files.size(); ++i) {
        std::cout << "Test " << (i + 1) << ":\n";
        auto res = readAndSortKeyValues(files[i]);
        for (const auto& s : res) {
            std::cout << s << "\n";
        }
    }
    return 0;
}