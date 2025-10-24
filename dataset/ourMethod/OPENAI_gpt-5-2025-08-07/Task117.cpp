#include <bits/stdc++.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/file.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

struct TarHeader {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char pad[12];
};

static bool is_all_zero(const unsigned char* blk, size_t n) {
    for (size_t i = 0; i < n; ++i) if (blk[i] != 0) return false;
    return true;
}

static uint64_t parse_octal(const char* s, size_t n, bool& ok) {
    ok = true;
    uint64_t v = 0;
    size_t i = 0;
    // skip leading spaces or nulls
    while (i < n && (s[i] == ' ' || s[i] == '\0')) i++;
    for (; i < n; ++i) {
        char c = s[i];
        if (c == ' ' || c == '\0') break;
        if (c < '0' || c > '7') { ok = false; break; }
        v = (v << 3) + (c - '0');
    }
    return v;
}

static unsigned int compute_checksum(const TarHeader& h) {
    unsigned int sum = 0;
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(&h);
    TarHeader temp = h;
    for (int i = 0; i < 8; ++i) temp.chksum[i] = ' ';
    const unsigned char* tbytes = reinterpret_cast<const unsigned char*>(&temp);
    for (size_t i = 0; i < sizeof(TarHeader); ++i) sum += tbytes[i];
    return sum;
}

static bool ensure_base_dir(const std::string& base) {
    struct stat st;
    if (stat(base.c_str(), &st) == 0) {
        if (!S_ISDIR(st.st_mode)) return false;
        // Reject symlink to dir
        struct stat lst;
        if (lstat(base.c_str(), &lst) == 0 && S_ISLNK(lst.st_mode)) return false;
        return true;
    }
    // create
    if (mkdir(base.c_str(), 0700) != 0) return false;
    return true;
}

static bool is_symlink(const std::string& path) {
    struct stat st;
    if (lstat(path.c_str(), &st) != 0) return false;
    return S_ISLNK(st.st_mode);
}

static bool normalize_join(const std::string& baseReal, const std::string& rel, std::string& out) {
    if (rel.empty()) return false;
    if (rel[0] == '/') return false;
    // disallow backslashes as separators and any NUL
    for (char c : rel) {
        if (c == '\0') return false;
    }
    std::vector<std::string> comps;
    std::string cur;
    for (size_t i = 0; i <= rel.size(); ++i) {
        if (i == rel.size() || rel[i] == '/') {
            if (!cur.empty()) {
                if (cur == ".") {
                    // skip
                } else if (cur == "..") {
                    if (comps.empty()) return false;
                    comps.pop_back();
                } else {
                    comps.push_back(cur);
                }
            }
            cur.clear();
        } else {
            cur.push_back(rel[i]);
        }
    }
    std::string joined = baseReal;
    for (auto& c : comps) {
        if (c.find('/') != std::string::npos) return false;
        joined += "/";
        joined += c;
    }
    // Ensure prefix match
    if (joined.size() < baseReal.size()) return false;
    if (joined.compare(0, baseReal.size(), baseReal) != 0) return false;
    // Ensure next char is '/' or end
    if (joined.size() > baseReal.size() && joined[baseReal.size()] != '/') return false;
    out = joined;
    return true;
}

static bool ensure_parent_dirs(const std::string& baseReal, const std::string& fullPath) {
    // Walk from baseReal to parent of fullPath creating dirs as needed, rejecting symlinks.
    if (fullPath.size() <= baseReal.size()) return false;
    size_t i = baseReal.size() + 1;
    while (true) {
        size_t pos = fullPath.find('/', i);
        if (pos == std::string::npos) break;
        std::string dir = fullPath.substr(0, pos);
        struct stat st;
        if (lstat(dir.c_str(), &st) == 0) {
            if (S_ISLNK(st.st_mode)) return false;
            if (!S_ISDIR(st.st_mode)) return false;
        } else {
            if (errno != ENOENT) return false;
            if (mkdir(dir.c_str(), 0700) != 0) return false;
        }
        i = pos + 1;
    }
    // Check parent directory not symlink
    std::string parent = fullPath.substr(0, fullPath.find_last_of('/'));
    struct stat pst;
    if (lstat(parent.c_str(), &pst) != 0) return false;
    if (!S_ISDIR(pst.st_mode)) return false;
    if (S_ISLNK(pst.st_mode)) return false;
    return true;
}

static bool write_file_secure(const std::string& targetPath, int mode, std::istream& in, uint64_t size) {
    std::string dir = targetPath.substr(0, targetPath.find_last_of('/'));
    std::string tmpl = dir + "/.tmp_extract_XXXXXX";
    std::vector<char> buf(tmpl.begin(), tmpl.end());
    buf.push_back('\0');
    int tfd = mkstemp(buf.data());
    if (tfd < 0) return false;
    // Restrictive permissions first
    fchmod(tfd, 0600);
    const size_t BUFSZ = 8192;
    std::vector<char> ibuf(BUFSZ);
    uint64_t remaining = size;
    while (remaining > 0) {
        size_t toRead = remaining > BUFSZ ? BUFSZ : (size_t)remaining;
        in.read(ibuf.data(), toRead);
        std::streamsize got = in.gcount();
        if (got != (std::streamsize)toRead) { close(tfd); unlink(buf.data()); return false; }
        ssize_t off = 0;
        while (off < got) {
            ssize_t w = write(tfd, ibuf.data() + off, (size_t)(got - off));
            if (w < 0) { close(tfd); unlink(buf.data()); return false; }
            off += w;
        }
        remaining -= toRead;
    }
    // Flush and fsync
    if (fsync(tfd) != 0) { close(tfd); unlink(buf.data()); return false; }
    if (close(tfd) != 0) { unlink(buf.data()); return false; }
    // Ensure target not symlink
    struct stat lst;
    if (lstat(targetPath.c_str(), &lst) == 0 && S_ISLNK(lst.st_mode)) {
        unlink(buf.data());
        return false;
    }
    // Atomic rename
    if (rename(buf.data(), targetPath.c_str()) != 0) {
        unlink(buf.data());
        return false;
    }
    // Set final permissions (mask to 0644)
    chmod(targetPath.c_str(), (mode & 0644) | 0600); // ensure owner read/write at least
    // fsync directory to persist rename
    int dfd = open(dir.c_str(), O_RDONLY | O_DIRECTORY);
    if (dfd >= 0) { fsync(dfd); close(dfd); }
    return true;
}

static bool extract_tar(const std::string& tarPath, const std::string& outBase) {
    if (!ensure_base_dir(outBase)) return false;
    char realbuf[PATH_MAX];
    if (!realpath(outBase.c_str(), realbuf)) return false;
    std::string baseReal(realbuf);
    std::ifstream in(tarPath, std::ios::binary);
    if (!in) return false;

    const size_t BLOCK = 512;
    std::vector<char> block(BLOCK);
    int zeroBlocks = 0;
    while (true) {
        in.read(block.data(), BLOCK);
        if (in.gcount() == 0) break;
        if (in.gcount() != (std::streamsize)BLOCK) return false;
        if (is_all_zero(reinterpret_cast<unsigned char*>(block.data()), BLOCK)) {
            zeroBlocks++;
            if (zeroBlocks == 2) break;
            else continue;
        } else {
            zeroBlocks = 0;
        }
        TarHeader h{};
        std::memcpy(&h, block.data(), sizeof(TarHeader));
        // Validate header checksum
        bool okSize = false, okChk = true;
        uint64_t fsize = parse_octal(h.size, sizeof(h.size), okSize);
        unsigned int expect = compute_checksum(h);
        bool okChksumField = true;
        uint64_t chkRead = parse_octal(h.chksum, sizeof(h.chksum), okChksumField);
        if (!okSize || !okChksumField || (unsigned int)chkRead != expect) {
            return false;
        }
        // Build path
        std::string name(h.name, h.name + strnlen(h.name, sizeof(h.name)));
        std::string prefix(h.prefix, h.prefix + strnlen(h.prefix, sizeof(h.prefix)));
        std::string fullName = prefix.empty() ? name : (prefix + "/" + name);
        // For directories, tar may have trailing slash
        char type = h.typeflag ? h.typeflag : '0';
        if (type == '5') {
            if (!fullName.empty() && fullName.back() == '/') fullName.pop_back();
        }
        std::string targetPath;
        if (!normalize_join(baseReal, fullName, targetPath)) {
            return false;
        }
        // Enforce size reasonable: limit to 100MB for safety in demo
        if (fsize > 100ULL * 1024ULL * 1024ULL) return false;

        // Modes
        bool okMode = false;
        int mode = (int)parse_octal(h.mode, sizeof(h.mode), okMode);
        if (!okMode) mode = 0644;

        if (type == '0' || type == '\0') {
            if (!ensure_parent_dirs(baseReal, targetPath)) return false;
            if (!write_file_secure(targetPath, mode, in, fsize)) return false;
            // Skip padding
            uint64_t pad = (BLOCK - (fsize % BLOCK)) % BLOCK;
            if (pad) { in.ignore(pad); if (!in) return false; }
        } else if (type == '5') {
            // Directory
            if (!ensure_parent_dirs(baseReal, targetPath + "/dummy")) return false;
            struct stat st;
            if (lstat(targetPath.c_str(), &st) != 0) {
                if (mkdir(targetPath.c_str(), 0700) != 0) return false;
            } else {
                if (S_ISLNK(st.st_mode)) return false;
                if (!S_ISDIR(st.st_mode)) return false;
            }
            // No data for dir; still need to skip padding for size if any (should be 0)
            uint64_t pad = (BLOCK - (fsize % BLOCK)) % BLOCK;
            if (pad) { in.ignore(pad); if (!in) return false; }
        } else if (type == '2') {
            // Symlink entry: reject for safety
            return false;
        } else {
            // Unknown type: skip data but treat as error
            return false;
        }
    }
    return true;
}

// Helper to write tar headers for tests
static void write_octal(char* dest, size_t n, uint64_t v) {
    // write v in octal, right-aligned, null-terminated with space
    std::snprintf(dest, n, "%0*llo", (int)(n - 1), (unsigned long long)v);
    // ensure trailing space or null
    dest[n - 1] = '\0';
}

static void fill_header(TarHeader& h, const std::string& name, char typeflag, uint64_t size, int mode, const std::string& prefix = "") {
    std::memset(&h, 0, sizeof(h));
    std::strncpy(h.name, name.c_str(), sizeof(h.name));
    std::snprintf(h.mode, sizeof(h.mode), "%07o", mode & 0777);
    std::snprintf(h.uid, sizeof(h.uid), "%07o", 0);
    std::snprintf(h.gid, sizeof(h.gid), "%07o", 0);
    std::snprintf(h.size, sizeof(h.size), "%011llo", (unsigned long long)size);
    std::snprintf(h.mtime, sizeof(h.mtime), "%011o", (unsigned)time(nullptr));
    std::memset(h.chksum, ' ', sizeof(h.chksum));
    h.typeflag = typeflag;
    std::strncpy(h.magic, "ustar", 5);
    h.magic[5] = '\0';
    std::memcpy(h.version, "00", 2);
    std::strncpy(h.uname, "user", sizeof(h.uname));
    std::strncpy(h.gname, "group", sizeof(h.gname));
    if (!prefix.empty()) std::strncpy(h.prefix, prefix.c_str(), sizeof(h.prefix));
    unsigned int sum = compute_checksum(h);
    std::snprintf(h.chksum, sizeof(h.chksum), "%06o", sum);
    h.chksum[6] = '\0';
    h.chksum[7] = ' ';
}

static bool create_sample_tar_basic(const std::string& tarPath) {
    std::ofstream out(tarPath, std::ios::binary);
    if (!out) return false;
    // dir entry
    {
        TarHeader h{};
        fill_header(h, "dir", '5', 0, 0755);
        out.write(reinterpret_cast<char*>(&h), sizeof(h));
    }
    // file entry dir/hello.txt
    std::string content = "Hello, world!\n";
    {
        TarHeader h{};
        fill_header(h, "dir/hello.txt", '0', content.size(), 0644);
        out.write(reinterpret_cast<char*>(&h), sizeof(h));
        out.write(content.data(), content.size());
        size_t pad = (512 - (content.size() % 512)) % 512;
        std::string zeros(pad, '\0');
        if (pad) out.write(zeros.data(), pad);
    }
    // Two zero blocks
    std::string zeros(1024, '\0');
    out.write(zeros.data(), zeros.size());
    return out.good();
}

static bool create_tar_with_entry(const std::string& tarPath, const std::string& entryName, char typeflag, const std::string& data) {
    std::ofstream out(tarPath, std::ios::binary);
    if (!out) return false;
    if (typeflag == '5') {
        TarHeader h{};
        fill_header(h, entryName, '5', 0, 0755);
        out.write(reinterpret_cast<char*>(&h), sizeof(h));
    } else {
        TarHeader h{};
        fill_header(h, entryName, typeflag, data.size(), 0644);
        out.write(reinterpret_cast<char*>(&h), sizeof(h));
        if (typeflag == '0' || typeflag == '\0') {
            out.write(data.data(), data.size());
            size_t pad = (512 - (data.size() % 512)) % 512;
            std::string zeros(pad, '\0');
            if (pad) out.write(zeros.data(), pad);
        }
    }
    std::string zeros(1024, '\0');
    out.write(zeros.data(), zeros.size());
    return out.good();
}

int main() {
    // Prepare output bases
    std::string base1 = "out1";
    std::string base2 = "out2";
    std::string base3 = "out3";
    std::string base4 = "out4";
    std::string base5 = "out5";
    mkdir(base1.c_str(), 0700);
    mkdir(base2.c_str(), 0700);
    mkdir(base3.c_str(), 0700);
    mkdir(base4.c_str(), 0700);
    mkdir(base5.c_str(), 0700);

    // Test 1: Basic extraction
    std::string tar1 = "test1.tar";
    create_sample_tar_basic(tar1);
    bool r1 = extract_tar(tar1, base1);
    std::cout << "Test1 (basic tar) result: " << (r1 ? "OK" : "FAIL") << "\n";

    // Test 2: Path traversal should fail
    std::string tar2 = "test2.tar";
    create_tar_with_entry(tar2, "../evil.txt", '0', "malicious");
    bool r2 = extract_tar(tar2, base2);
    std::cout << "Test2 (path traversal) result: " << (!r2 ? "OK" : "FAIL") << "\n";

    // Test 3: Symlink entry should fail
    std::string tar3 = "test3.tar";
    create_tar_with_entry(tar3, "link", '2', "");
    bool r3 = extract_tar(tar3, base3);
    std::cout << "Test3 (symlink entry) result: " << (!r3 ? "OK" : "FAIL") << "\n";

    // Test 4: Nonexistent tar file
    bool r4 = extract_tar("no_such_file.tar", base4);
    std::cout << "Test4 (nonexistent) result: " << (!r4 ? "OK" : "FAIL") << "\n";

    // Test 5: Absolute path should fail
    std::string tar5 = "test5.tar";
    create_tar_with_entry(tar5, "/abs.txt", '0', "abs");
    bool r5 = extract_tar(tar5, base5);
    std::cout << "Test5 (absolute path) result: " << (!r5 ? "OK" : "FAIL") << "\n";
    return 0;
}