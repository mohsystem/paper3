#include <bits/stdc++.h>
#include <filesystem>
using namespace std;

static bool readExact(ifstream& in, char* buf, size_t n) {
    in.read(buf, n);
    return static_cast<size_t>(in.gcount()) == n;
}

static bool isZeroBlock(const char* buf, size_t n) {
    for (size_t i = 0; i < n; ++i) if (buf[i] != 0) return false;
    return true;
}

static uint64_t parseOctal(const char* p, size_t n) {
    uint64_t v = 0;
    size_t i = 0;
    // skip leading spaces and nulls
    while (i < n && (p[i] == ' ' || p[i] == '\0')) i++;
    for (; i < n; ++i) {
        char c = p[i];
        if (c == '\0' || c == ' ') break;
        if (c < '0' || c > '7') break;
        v = (v << 3) + static_cast<uint64_t>(c - '0');
    }
    return v;
}

static uint64_t roundUp512(uint64_t x) {
    uint64_t rem = x % 512ULL;
    return rem == 0 ? x : x + (512ULL - rem);
}

static bool pathIsSafe(const filesystem::path& destBase, const filesystem::path& target) {
    error_code ec;
    auto destAbs = filesystem::absolute(destBase, ec);
    if (ec) return false;
    destAbs = destAbs.lexically_normal();
    auto targetAbs = filesystem::absolute(target, ec);
    if (ec) return false;
    targetAbs = targetAbs.lexically_normal();
    auto destStr = destAbs.native();
    auto targetStr = targetAbs.native();

#ifdef _WIN32
    // Case-insensitive comparison on Windows
    auto toLower = [](const wstring& s){ wstring r=s; for (auto& ch:r) ch=std::towlower(ch); return r; };
    wstring d = toLower(destStr);
    wstring t = toLower(targetStr);
    if (t.size() < d.size()) return false;
    if (t.compare(0, d.size(), d) != 0) return false;
    if (t.size() == d.size()) return true;
    wchar_t sep = L'\\';
    if (d.back() == L'\\' || d.back() == L'/') return true;
    return t[d.size()] == L'\\' || t[d.size()] == L'/';
#else
    if (targetStr.size() < destStr.size()) return false;
    if (targetStr.compare(0, destStr.size(), destStr) != 0) return false;
    if (targetStr.size() == destStr.size()) return true;
    char sep = '/';
    if (!destStr.empty() && (destStr.back() == '/' )) return true;
    return targetStr[destStr.size()] == '/';
#endif
}

static string readString(const char* buf, size_t off, size_t len) {
    size_t end = off;
    size_t max = off + len;
    while (end < max && buf[end] != '\0') end++;
    return string(buf + off, buf + end);
}

// Extract uncompressed TAR archives
vector<string> extract_tar(const string& archivePath, const string& destDir) {
    vector<string> extracted;
    filesystem::path destBase(destDir);
    error_code ec;
    filesystem::create_directories(destBase, ec);

    ifstream in(archivePath, ios::binary);
    if (!in) {
        throw runtime_error("Cannot open TAR file: " + archivePath);
    }
    array<char,512> header{};
    while (true) {
        if (!readExact(in, header.data(), 512)) {
            break; // EOF or truncated
        }
        if (isZeroBlock(header.data(), 512)) {
            // Consume possible second zero block
            readExact(in, header.data(), 512);
            break;
        }
        string name = readString(header.data(), 0, 100);
        string prefix = readString(header.data(), 345, 155);
        char typeflag = header[156];
        uint64_t size = parseOctal(header.data() + 124, 12);

        if (!prefix.empty()) {
            name = prefix + "/" + name;
        }
        // Sanitize
        for (auto& ch : name) if (ch == '\\') ch = '/';
        if (name.empty() || name[0] == '/') {
            // skip
        } else {
            // reject traversal
            bool bad = false;
            {
                // check for .. segments
                stringstream ss(name);
                string seg;
                while (getline(ss, seg, '/')) {
                    if (seg == "..") { bad = true; break; }
                }
            }
            filesystem::path outPath = destBase / name;
            if (!bad && pathIsSafe(destBase, outPath)) {
                if (typeflag == '5' || (!name.empty() && name.back() == '/')) {
                    filesystem::create_directories(outPath, ec);
                } else if (typeflag == '0' || typeflag == '\0') {
                    filesystem::create_directories(outPath.parent_path(), ec);
                    ofstream out(outPath, ios::binary | ios::trunc);
                    if (!out) {
                        throw runtime_error("Cannot create file: " + outPath.string());
                    }
                    uint64_t remaining = size;
                    vector<char> buf(8192);
                    while (remaining > 0) {
                        size_t toRead = static_cast<size_t>(min<uint64_t>(buf.size(), remaining));
                        in.read(buf.data(), toRead);
                        streamsize got = in.gcount();
                        if (got <= 0) throw runtime_error("Unexpected EOF in file data");
                        out.write(buf.data(), got);
                        if (!out) throw runtime_error("Write failed");
                        remaining -= static_cast<uint64_t>(got);
                    }
                    out.close();
                    extracted.push_back(filesystem::absolute(outPath, ec).string());
                    // Skip padding
                    uint64_t pad = roundUp512(size) - size;
                    if (pad > 0) in.ignore(pad);
                    continue;
                } else {
                    // skip other types
                }
            }
        }
        // Skip content for non-regular or skipped entries
        uint64_t skip = roundUp512(size);
        if (skip > 0) in.ignore(skip);
    }
    return extracted;
}

int main() {
    // 5 test cases (demonstration). Adjust paths as needed.
    vector<pair<string,string>> tests = {
        {"nonexistent.tar", "out1"},
        {"./sample.tar", "out2"},
        {"./unsafe.tar", "out3"},
        {"./archive.tar", "out4"},
        {"./another.tar", "out5"}
    };
    for (size_t i = 0; i < tests.size(); ++i) {
        try {
            auto res = extract_tar(tests[i].first, tests[i].second);
            cout << "Test" << (i+1) << " extracted " << res.size() << " files\n";
        } catch (const exception& e) {
            cout << "Test" << (i+1) << " failed: " << e.what() << "\n";
        }
    }
    return 0;
}