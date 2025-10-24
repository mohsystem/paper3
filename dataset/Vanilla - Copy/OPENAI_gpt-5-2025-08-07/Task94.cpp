#include <bits/stdc++.h>
using namespace std;

static inline string trim(const string& s) {
    size_t b = s.find_first_not_of(" \t\r\n");
    if (b == string::npos) return "";
    size_t e = s.find_last_not_of(" \t\r\n");
    return s.substr(b, e - b + 1);
}

vector<pair<string,string>> readAndSortKeyValueFile(const string& path) {
    vector<pair<string,string>> pairs;
    ifstream in(path);
    string line;
    while (std::getline(in, line)) {
        size_t pos = line.find('=');
        if (pos == string::npos) continue;
        string k = trim(line.substr(0, pos));
        string v = trim(line.substr(pos + 1));
        if (k.empty() && v.empty()) continue;
        pairs.emplace_back(k, v);
    }
    sort(pairs.begin(), pairs.end(), [](const auto& a, const auto& b) {
        if (a.first != b.first) return a.first < b.first;
        return a.second < b.second;
    });
    return pairs;
}

void writeFile(const string& path, const string& content) {
    ofstream out(path);
    out << content;
}

void runTest(const string& title, const string& path) {
    cout << "== " << title << " ==" << "\n";
    auto res = readAndSortKeyValueFile(path);
    for (auto& kv : res) {
        cout << kv.first << "=" << kv.second << "\n";
    }
    cout << "\n";
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string f1 = "test1_kv_cpp.txt";
    string f2 = "test2_kv_cpp.txt";
    string f3 = "test3_kv_cpp.txt";
    string f4 = "test4_kv_cpp.txt";
    string f5 = "test5_kv_cpp.txt";

    writeFile(f1, "b=2\na=1\nc=3\n");
    writeFile(f2, "k=3\nk=1\nk=2\n");
    writeFile(f3, "  x  =  10  \ninvalid line\ny=5\n=onlyvalue\nonlykey=\n");
    writeFile(f4, "10=ten\n2=two\n1=one\n");
    writeFile(f5, "a= \n a=0\n b= \n z=last\n   \nnoequalsline\n");

    runTest("Test 1", f1);
    runTest("Test 2", f2);
    runTest("Test 3", f3);
    runTest("Test 4", f4);
    runTest("Test 5", f5);

    return 0;
}