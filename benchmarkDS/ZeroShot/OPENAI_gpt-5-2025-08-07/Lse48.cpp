#include <bits/stdc++.h>
#include <filesystem>
using namespace std;
namespace fs = std::filesystem;

static const vector<string> allowedExt = {"png","jpg","jpeg","gif","bmp","webp"};
static const size_t MAX_FILE_SIZE = 5ull * 1024ull * 1024ull;

static bool isAllowedName(const string& name) {
    if (name.empty()) return false;
    for (char c : name) {
        if (!(isalnum((unsigned char)c) || c=='.' || c=='_' || c=='-')) return false;
    }
    auto pos = name.find_last_of('.');
    if (pos == string::npos) return false;
    string ext = name.substr(pos + 1);
    string lower;
    lower.reserve(ext.size());
    for (char c : ext) lower.push_back((char)tolower((unsigned char)c));
    for (const auto& e : allowedExt) {
        if (lower == e) return true;
    }
    return false;
}

static vector<unsigned char> img(const string& fileParam) {
    vector<unsigned char> empty;
    try {
        if (!isAllowedName(fileParam)) return empty;

        fs::path base = fs::absolute(fs::path("static") / "images");
        fs::create_directories(base);

        fs::path candidate = (base / fileParam);

        if (!fs::exists(candidate) || !fs::is_regular_file(candidate)) return empty;

        fs::path baseCanon = fs::canonical(base);
        fs::path fileCanon = fs::canonical(candidate);

        auto bcStr = baseCanon.generic_string();
        auto fcStr = fileCanon.generic_string();
        if (fcStr.size() < bcStr.size() || fcStr.compare(0, bcStr.size(), bcStr) != 0 ||
            (fcStr.size() > bcStr.size() && fcStr[bcStr.size()] != '/')) {
            return empty;
        }

        uintmax_t fsize = fs::file_size(fileCanon);
        if (fsize > MAX_FILE_SIZE) return empty;

        vector<unsigned char> data;
        data.resize((size_t)fsize);
        ifstream in(fileCanon, ios::binary);
        if (!in) return empty;
        in.read(reinterpret_cast<char*>(data.data()), (streamsize)data.size());
        if (!in && !in.eof()) return empty;
        return data;
    } catch (...) {
        return empty;
    }
}

static void ensureTestFiles() {
    fs::path base = fs::absolute(fs::path("static") / "images");
    fs::create_directories(base);
    fs::path f1 = base / "test1.png";
    if (!fs::exists(f1)) {
        ofstream o(f1, ios::binary);
        string s = "PNGDATA";
        o.write(s.data(), (streamsize)s.size());
    }
    fs::path f2 = base / "test2.txt";
    if (!fs::exists(f2)) {
        ofstream o(f2, ios::binary);
        string s = "TEXTDATA";
        o.write(s.data(), (streamsize)s.size());
    }
}

int main() {
    ensureTestFiles();
    vector<string> tests = {
        "test1.png",     // valid
        "../secret.txt", // traversal
        "test2.txt",     // invalid ext
        "missing.jpg",   // missing
        "bad/evil.png"   // path separator (invalid name)
    };
    for (auto& t : tests) {
        auto out = img(t);
        cout << t << " -> " << out.size() << "\n";
    }
    return 0;
}