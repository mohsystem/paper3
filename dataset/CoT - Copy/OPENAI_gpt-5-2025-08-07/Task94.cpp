// Chain-of-Through steps via comments for readability and security focus.

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdio>

struct Pair {
    std::string key;
    std::string value;
};

static constexpr size_t MAX_RECORDS = 1'000'000;

static inline std::string trim(const std::string& s) {
    size_t i = 0, j = s.size();
    while (i < j && (s[i] == ' ' || s[i] == '\t' || s[i] == '\r' || s[i] == '\n')) ++i;
    while (j > i && (s[j - 1] == ' ' || s[j - 1] == '\t' || s[j - 1] == '\r' || s[j - 1] == '\n')) --j;
    return s.substr(i, j - i);
}

std::vector<Pair> read_and_sort_records(const std::string& file_path) {
    std::vector<Pair> records;
    std::ifstream in(file_path, std::ios::in);
    if (!in) {
        throw std::runtime_error("Cannot open file: " + file_path);
    }
    std::string line;
    while (std::getline(in, line)) {
        std::string trimmed = trim(line);
        if (trimmed.empty()) continue;
        if (!trimmed.empty() && trimmed[0] == '#') continue;
        auto pos = trimmed.find('=');
        if (pos == std::string::npos || pos == 0) continue; // no '=' or empty key
        std::string key = trim(trimmed.substr(0, pos));
        if (key.empty()) continue;
        std::string value = trim(trimmed.substr(pos + 1));
        records.push_back({key, value});
        if (records.size() > MAX_RECORDS) {
            throw std::runtime_error("Too many records; possible resource exhaustion.");
        }
    }
    std::sort(records.begin(), records.end(), [](const Pair& a, const Pair& b) {
        if (a.key < b.key) return true;
        if (a.key > b.key) return false;
        return a.value < b.value;
    });
    return records;
}

static std::string records_to_string(const std::vector<Pair>& recs) {
    std::string s = "[";
    for (size_t i = 0; i < recs.size(); ++i) {
        s += recs[i].key + "=" + recs[i].value;
        if (i + 1 < recs.size()) s += ", ";
    }
    s += "]";
    return s;
}

static std::string write_temp(const std::string& content, int idx) {
    std::string path = "task94_cpp_test_" + std::to_string(idx) + ".txt";
    std::ofstream out(path, std::ios::out | std::ios::trunc);
    out << content;
    out.close();
    return path;
}

int main() {
    std::vector<std::string> tests = {
        "b=2\na=1\nc=3\n",
        "  key2 =  z \n# comment\n\nkey1= a \n  # another comment\n",
        "k=2\nk=1\nk=3\n",
        "noequal\n=onlyvalue\nonlykey=\n mid = val = extra \n",
        "äpple=1\nbanana=2\nÁlpha=0\n",
    };
    std::vector<std::string> paths;
    try {
        for (int i = 0; i < (int)tests.size(); ++i) {
            paths.push_back(write_temp(tests[i], i + 1));
        }
        for (size_t i = 0; i < paths.size(); ++i) {
            auto recs = read_and_sort_records(paths[i]);
            std::cout << "Test " << (i + 1) << " result: " << records_to_string(recs) << "\n";
        }
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
    }
    for (const auto& p : paths) {
        std::remove(p.c_str());
    }
    return 0;
}