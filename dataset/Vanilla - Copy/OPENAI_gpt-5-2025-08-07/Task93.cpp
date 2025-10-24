#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cctype>

static std::string trim(const std::string& s) {
    size_t i = 0, j = s.size();
    while (i < j && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
    while (j > i && std::isspace(static_cast<unsigned char>(s[j - 1]))) --j;
    return s.substr(i, j - i);
}

std::vector<std::string> sortKeyValueFile(const std::string& filePath) {
    std::ifstream in(filePath);
    std::vector<std::pair<std::string, std::string>> kv;
    if (!in) return {};
    std::string line;
    while (std::getline(in, line)) {
        std::string t = trim(line);
        if (t.empty() || (!t.empty() && t[0] == '#')) continue;
        size_t posEq = t.find('=');
        size_t posCol = t.find(':');
        size_t idx;
        if (posEq == std::string::npos && posCol == std::string::npos) continue;
        else if (posEq == std::string::npos) idx = posCol;
        else if (posCol == std::string::npos) idx = posEq;
        else idx = std::min(posEq, posCol);
        if (idx == 0) continue;
        std::string key = trim(t.substr(0, idx));
        std::string value = trim(t.substr(idx + 1));
        if (key.empty()) continue;
        kv.emplace_back(key, value);
    }
    std::sort(kv.begin(), kv.end(), [](const auto& a, const auto& b) {
        if (a.first != b.first) return a.first < b.first;
        return a.second < b.second;
    });
    std::vector<std::string> out;
    out.reserve(kv.size());
    for (auto& p : kv) out.push_back(p.first + "=" + p.second);
    return out;
}

static void writeFile(const std::string& path, const std::string& content) {
    std::ofstream out(path, std::ios::binary);
    out << content;
}

int main() {
    std::string f1 = "task93_cpp_test1.txt";
    std::string f2 = "task93_cpp_test2.txt";
    std::string f3 = "task93_cpp_test3.txt";
    std::string f4 = "task93_cpp_test4.txt";
    std::string f5 = "task93_cpp_test5.txt";

    writeFile(f1, "b=2\na=1\nc=3\n");
    writeFile(f2, "orange:10\napple:20\nbanana:15\n");
    writeFile(f3, " # comment\n z : last \n\n a : first \n middle=between \n");
    writeFile(f4, "dup=2\ndup=1\nsame:0\nsame:5\n");
    writeFile(f5, "invalidline\n k=v\n x : y \n #comment \n :missingkey? \n =missingkey2 \n");

    auto r1 = sortKeyValueFile(f1);
    auto r2 = sortKeyValueFile(f2);
    auto r3 = sortKeyValueFile(f3);
    auto r4 = sortKeyValueFile(f4);
    auto r5 = sortKeyValueFile(f5);

    std::cout << "Test1: ";
    for (size_t i = 0; i < r1.size(); ++i) { if (i) std::cout << ", "; std::cout << r1[i]; }
    std::cout << "\n";
    std::cout << "Test2: ";
    for (size_t i = 0; i < r2.size(); ++i) { if (i) std::cout << ", "; std::cout << r2[i]; }
    std::cout << "\n";
    std::cout << "Test3: ";
    for (size_t i = 0; i < r3.size(); ++i) { if (i) std::cout << ", "; std::cout << r3[i]; }
    std::cout << "\n";
    std::cout << "Test4: ";
    for (size_t i = 0; i < r4.size(); ++i) { if (i) std::cout << ", "; std::cout << r4[i]; }
    std::cout << "\n";
    std::cout << "Test5: ";
    for (size_t i = 0; i < r5.size(); ++i) { if (i) std::cout << ", "; std::cout << r5[i]; }
    std::cout << "\n";

    return 0;
}