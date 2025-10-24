#include <bits/stdc++.h>
#include <filesystem>
using namespace std;

// Minimal secure TAR extractor (no external libs). This code extracts .tar files only.

namespace fs = std::filesystem;

static bool isAllZero(const array<unsigned char,512>& buf) {
    for (auto b : buf) if (b != 0) return false;
    return true;
}

static uint64_t readOctal(const unsigned char* hdr, int off, int len) {
    uint64_t val = 0;
    int end = off + len;
    int i = off;
    while (i < end && (hdr[i] == 0 || hdr[i] == ' ')) i++;
    for (; i < end; i++) {
        unsigned char c = hdr[i];
        if (c == 0 || c == ' ') break;
        if (c < '0' || c > '7') break;
        val = (val << 3) + (c - '0');
    }
    return val;
}

static string readString(const unsigned char* hdr, int off, int len) {
    int end = off;
    int max = off + len;
    while (end < max && hdr[end] != 0) end++;
    return string(reinterpret_cast<const char*>(hdr + off), end - off);
}

static uint64_t pad512(uint64_t size) {
    uint64_t rem = size % 512;
    return rem == 0 ? 0 : (512 - rem);
}

static fs::path resolveSecure(const fs::path& baseDir, const string& entryName) {
    fs::path target = fs::weakly_canonical(baseDir / fs::path(entryName)).lexically_normal();
    fs::path base = fs::weakly_canonical(baseDir).lexically_normal();
    // Ensure target is inside base
    auto b = base.string();
    auto t = target.string();
    if (t.size() < b.size() || t.compare(0, b.size(), b) != 0 || (t.size() > b.size() && t[b.size()] != fs::path::preferred_separator)) {
        throw runtime_error("Blocked path traversal: " + entryName);
    }
    return target;
}

static void skipFully(istream& in, uint64_t n) {
    const size_t BUF = 8192;
    vector<char> buf(BUF);
    uint64_t remaining = n;
    while (remaining > 0) {
        size_t chunk = static_cast<size_t>(min<uint64_t>(BUF, remaining));
        in.read(buf.data(), chunk);
        streamsize r = in.gcount();
        if (r <= 0) throw runtime_error("Unexpected EOF while skipping");
        remaining -= static_cast<uint64_t>(r);
    }
}

// Extract .tar securely with size limit (returns count of files extracted)
int extractTarSecure(const string& tarPath, const string& destDir, uint64_t maxTotalBytes) {
    fs::path tarP(tarPath);
    fs::path dest(destDir);
    fs::create_directories(dest);

    ifstream in(tarP, ios::binary);
    if (!in) throw runtime_error("Cannot open tar file");

    int filesExtracted = 0;
    uint64_t total = 0;

    array<unsigned char,512> hdr{};
    while (true) {
        in.read(reinterpret_cast<char*>(hdr.data()), 512);
        if (in.gcount() == 0) break;
        if (in.gcount() < 512) throw runtime_error("Unexpected EOF in tar header");
        if (isAllZero(hdr)) {
            // possible second zero block
            in.read(reinterpret_cast<char*>(hdr.data()), 512);
            break;
        }
        string name = readString(hdr.data(), 0, 100);
        string prefix = readString(hdr.data(), 345, 155);
        char typeflag = static_cast<char>(hdr[156]);
        uint64_t size = readOctal(hdr.data(), 124, 12);

        string fullName = prefix.size() ? (prefix + "/" + name) : name;
        if (!fullName.empty() && (fullName[0] == '/' || fullName[0] == '\\')) {
            skipFully(in, size + pad512(size));
            continue;
        }

        fs::path target;
        try {
            target = resolveSecure(dest, fullName);
        } catch (...) {
            skipFully(in, size + pad512(size));
            continue;
        }

        if (typeflag == '5') {
            fs::create_directories(target);
            skipFully(in, pad512(size));
        } else if (typeflag == '0' || typeflag == 0) {
            fs::create_directories(target.parent_path());
            ofstream out(target, ios::binary | ios::trunc);
            if (!out) throw runtime_error("Cannot create file: " + target.string());
            uint64_t remaining = size;
            vector<char> buf(8192);
            while (remaining > 0) {
                size_t chunk = static_cast<size_t>(min<uint64_t>(buf.size(), remaining));
                in.read(buf.data(), chunk);
                streamsize r = in.gcount();
                if (r <= 0) throw runtime_error("Unexpected EOF in tar content");
                if (total + static_cast<uint64_t>(r) > maxTotalBytes) {
                    uint64_t allowed = (maxTotalBytes > total) ? (maxTotalBytes - total) : 0;
                    if (allowed > 0) out.write(buf.data(), static_cast<size_t>(allowed));
                    throw runtime_error("Extraction aborted: size limit exceeded");
                }
                out.write(buf.data(), r);
                total += static_cast<uint64_t>(r);
                remaining -= static_cast<uint64_t>(r);
            }
            out.close();
            skipFully(in, pad512(size));
            filesExtracted++;
        } else {
            // skip symlink/hardlink/others for safety
            skipFully(in, size + pad512(size));
        }
    }

    return filesExtracted;
}

// Helpers to create simple TAR files for testing
static void writeOctal(unsigned char* hdr, int off, int len, uint64_t value) {
    string s;
    if (value == 0) s = "0";
    else {
        uint64_t v = value;
        string tmp;
        while (v > 0) {
            tmp.push_back(char('0' + (v & 7)));
            v >>= 3;
        }
        reverse(tmp.begin(), tmp.end());
        s = tmp;
    }
    int idx = len - 1;
    hdr[off + idx] = 0; idx--;
    if (idx >= 0) { hdr[off + idx] = ' '; idx--; }
    for (int i = int(s.size()) - 1; i >= 0 && idx >= 0; --i, --idx) {
        hdr[off + idx] = (unsigned char)s[i];
    }
    while (idx >= 0) {
        hdr[off + idx] = '0';
        --idx;
    }
}

static void writeTarHeader(ostream& out, const string& name, uint64_t size, char typeflag) {
    array<unsigned char,512> hdr{};
    // name
    for (size_t i = 0; i < name.size() && i < 100; ++i) hdr[i] = (unsigned char)name[i];
    // mode
    writeOctal(hdr.data(), 100, 8, 0644);
    // uid/gid
    writeOctal(hdr.data(), 108, 8, 0);
    writeOctal(hdr.data(), 116, 8, 0);
    // size
    writeOctal(hdr.data(), 124, 12, size);
    // mtime
    writeOctal(hdr.data(), 136, 12, (uint64_t)time(nullptr));
    // checksum placeholder (spaces)
    for (int i = 148; i < 156; ++i) hdr[i] = ' ';
    // typeflag
    hdr[156] = (unsigned char)typeflag;
    // magic "ustar"
    string magic = "ustar";
    for (size_t i = 0; i < magic.size(); ++i) hdr[257 + i] = (unsigned char)magic[i];
    hdr[262] = '0';
    hdr[263] = '0';
    // compute checksum
    uint64_t sum = 0;
    for (auto b : hdr) sum += b;
    writeOctal(hdr.data(), 148, 8, sum);
    out.write(reinterpret_cast<const char*>(hdr.data()), 512);
}

static void padTo512(ostream& out, uint64_t size) {
    uint64_t p = pad512(size);
    vector<char> zero(p, 0);
    if (p) out.write(zero.data(), p);
}

static void createSampleTar(const fs::path& path) {
    ofstream out(path, ios::binary | ios::trunc);
    string d = "dir/";
    writeTarHeader(out, d, 0, '5'); // directory

    string aName = "dir/a.txt";
    string aData = "Hello from tar a.txt";
    writeTarHeader(out, aName, aData.size(), '0');
    out.write(aData.data(), aData.size());
    padTo512(out, aData.size());

    string bName = "b.txt";
    string bData = "Hello from tar b.txt";
    writeTarHeader(out, bName, bData.size(), '0');
    out.write(bData.data(), bData.size());
    padTo512(out, bData.size());

    array<char,512> zero{};
    out.write(zero.data(), 512);
    out.write(zero.data(), 512);
}

static void createTraversalTar(const fs::path& path) {
    ofstream out(path, ios::binary | ios::trunc);
    string sName = "safe.txt";
    string sData = "safe";
    writeTarHeader(out, sName, sData.size(), '0');
    out.write(sData.data(), sData.size());
    padTo512(out, sData.size());

    string eName = "../evil.txt";
    string eData = "evil";
    writeTarHeader(out, eName, eData.size(), '0');
    out.write(eData.data(), eData.size());
    padTo512(out, eData.size());

    array<char,512> zero{};
    out.write(zero.data(), 512);
    out.write(zero.data(), 512);
}

static void createLargeTar(const fs::path& path, size_t sizeBytes) {
    ofstream out(path, ios::binary | ios::trunc);
    string name = "large.bin";
    writeTarHeader(out, name, sizeBytes, '0');
    vector<char> buf(8192, 1);
    size_t remaining = sizeBytes;
    while (remaining > 0) {
        size_t chunk = min(buf.size(), remaining);
        out.write(buf.data(), chunk);
        remaining -= chunk;
    }
    padTo512(out, sizeBytes);
    array<char,512> zero{};
    out.write(zero.data(), 512);
    out.write(zero.data(), 512);
}

int main() {
    try {
        fs::path base = fs::temp_directory_path() / "task117_cpp_tests";
        fs::create_directories(base);
        cout << "C++ tests working dir: " << base << "\n";

        // Test 1: simple tar
        fs::path tar1 = base / "sample.tar";
        fs::path out1 = base / "out_tar1";
        createSampleTar(tar1);
        fs::create_directories(out1);
        int c1 = extractTarSecure(tar1.string(), out1.string(), 10'000'000);
        cout << "Test1 TAR extracted files: " << c1 << "\n";

        // Test 2: traversal tar
        fs::path tar2 = base / "trav.tar";
        fs::path out2 = base / "out_tar2";
        createTraversalTar(tar2);
        fs::create_directories(out2);
        int c2 = extractTarSecure(tar2.string(), out2.string(), 10'000'000);
        cout << "Test2 TAR extracted files (expect 1): " << c2 << "\n";

        // Test 3: large tar with size limit
        fs::path tarLarge = base / "large.tar";
        fs::path out3 = base / "out_tar3";
        createLargeTar(tarLarge, 2'000'000);
        fs::create_directories(out3);
        try {
            extractTarSecure(tarLarge.string(), out3.string(), 100'000);
            cout << "Test3 TAR size limit: FAILED\n";
        } catch (const exception& e) {
            cout << "Test3 TAR size limit: PASSED (" << e.what() << ")\n";
        }

        // Test 4: nested directories already covered; create another sample
        fs::path tar4 = base / "sample2.tar";
        fs::path out4 = base / "out_tar4";
        createSampleTar(tar4);
        fs::create_directories(out4);
        int c4 = extractTarSecure(tar4.string(), out4.string(), 10'000'000);
        cout << "Test4 TAR extracted files: " << c4 << "\n";

        // Test 5: symlink entry skipped
        // Build a tar with a symlink entry
        fs::path tar5 = base / "symlink.tar";
        {
            ofstream out(tar5, ios::binary | ios::trunc);
            // Add a regular file
            string data = "file";
            writeTarHeader(out, "file.txt", data.size(), '0');
            out.write(data.data(), data.size());
            padTo512(out, data.size());
            // Add a symlink entry to be skipped by extractor
            writeTarHeader(out, "link.txt", 0, '2'); // symlink, no data
            array<char,512> zero{};
            out.write(zero.data(), 512);
            out.write(zero.data(), 512);
        }
        fs::path out5 = base / "out_tar5";
        fs::create_directories(out5);
        int c5 = extractTarSecure(tar5.string(), out5.string(), 10'000'000);
        cout << "Test5 TAR with symlink entry skipped, files extracted: " << c5 << "\n";
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << "\n";
    }
    return 0;
}