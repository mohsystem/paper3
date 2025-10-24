// Chain-of-Through process in code:
// 1) Problem understanding: File upload utility validating input and saving securely.
// 2) Security requirements: size limit, allowed extensions, magic sniffing, sanitize names, traversal prevention, unique names, safe write+rename, restrictive perms.
// 3) Secure coding generation: Structured functions for each validation and I/O.
// 4) Code review: Exception-safe, no unsafe system calls, checks for paths.
// 5) Secure code output: Final code ensures mitigations and provides 5 test cases.

#include <bits/stdc++.h>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <random>

namespace fs = std::filesystem;

static const size_t MAX_SIZE = 5 * 1024 * 1024;
static const std::unordered_set<std::string> ALLOWED_EXT = {"txt","png","jpg","jpeg","pdf"};

static std::string sanitizeFilename(const std::string& name) {
    // strip directories
    std::string base = name;
    size_t pos1 = base.find_last_of("/\\");
    if (pos1 != std::string::npos) base = base.substr(pos1 + 1);
    if (base.empty()) base = "file";

    std::string out;
    out.reserve(base.size());
    for (unsigned char c : base) {
        if (std::isalnum(c) || c=='.' || c=='_' || c=='-') out.push_back(c);
        else out.push_back('_');
    }
    if (out == "." || out == ".." || out.empty()) out = "file";
    if (out.size() > 100) out.resize(100);
    return out;
}

static std::string toLower(std::string s) {
    for (auto &c : s) c = std::tolower(static_cast<unsigned char>(c));
    return s;
}

static std::string getExt(const std::string& name) {
    auto pos = name.find_last_of('.');
    if (pos == std::string::npos || pos == name.size()-1) return "";
    return toLower(name.substr(pos+1));
}

static std::string stripExt(const std::string& name) {
    auto pos = name.find_last_of('.');
    if (pos == std::string::npos) return name;
    return name.substr(0, pos);
}

static bool startsWith(const std::vector<uint8_t>& data, const std::vector<uint8_t>& pref) {
    if (data.size() < pref.size()) return false;
    for (size_t i=0;i<pref.size();++i) if (data[i]!=pref[i]) return false;
    return true;
}

static bool isLikelyText(const std::vector<uint8_t>& data) {
    if (data.empty()) return true;
    size_t printable = 0;
    for (auto b : data) {
        if (b==9 || b==10 || b==13 || (b>=32 && b<=126) || b>=128) printable++;
    }
    return (double)printable / (double)data.size() > 0.95;
}

static bool contentMatchesExt(const std::vector<uint8_t>& data, const std::string& ext) {
    if (ext=="png") {
        static const std::vector<uint8_t> sig = {0x89,'P','N','G',0x0D,0x0A,0x1A,0x0A};
        return startsWith(data, sig);
    }
    if (ext=="jpg" || ext=="jpeg") {
        static const std::vector<uint8_t> sig = {0xFF,0xD8,0xFF};
        return startsWith(data, sig);
    }
    if (ext=="pdf") {
        static const std::vector<uint8_t> sig = {'%','P','D','F','-'};
        return startsWith(data, sig);
    }
    if (ext=="txt") {
        return isLikelyText(data);
    }
    return false;
}

static std::string randomHex(size_t bytes) {
    std::random_device rd;
    std::vector<unsigned char> buf(bytes);
    for (size_t i=0;i<bytes;i++) buf[i] = static_cast<unsigned char>(rd());
    static const char* hex = "0123456789abcdef";
    std::string out; out.reserve(bytes*2);
    for (auto b: buf) { out.push_back(hex[b>>4]); out.push_back(hex[b&0xF]); }
    return out;
}

static void setRestrictivePermissions(const fs::path& p) {
    try {
        using perm = fs::perms;
        if (fs::is_directory(p)) {
            fs::permissions(p, perm::owner_read|perm::owner_write|perm::owner_exec|perm::group_read|perm::group_exec,
                            fs::perm_options::replace);
        } else {
            fs::permissions(p, perm::owner_read|perm::owner_write|perm::group_read,
                            fs::perm_options::replace);
        }
    } catch (...) {
        // Best effort
    }
}

// Upload function: returns absolute saved path or empty string on failure
std::string uploadFile(const std::string& originalFilename, const std::vector<uint8_t>& data, const std::string& destDir) {
    try {
        if (originalFilename.empty() || data.empty() || data.size() > MAX_SIZE) return "";

        std::string safe = sanitizeFilename(originalFilename);
        std::string ext = getExt(safe);
        if (ext.empty() || !ALLOWED_EXT.count(ext)) return "";

        if (!contentMatchesExt(data, ext)) return "";

        fs::path dir = fs::absolute(destDir);
        fs::create_directories(dir);
        setRestrictivePermissions(dir);

        std::string base = stripExt(safe);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::system_clock::now().time_since_epoch()).count();
        std::string finalName = base + "_" + std::to_string(ms) + "_" + randomHex(8) + "." + ext;
        fs::path finalPath = fs::weakly_canonical(dir / finalName);

        // Ensure final path within dir
        if (finalPath.string().rfind(fs::weakly_canonical(dir).string(), 0) != 0) return "";

        // Write to temp then rename
        fs::path tmpPath = dir / ("upload_" + randomHex(8) + ".tmp");
        {
            std::ofstream ofs(tmpPath, std::ios::binary | std::ios::trunc);
            if (!ofs) return "";
            ofs.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
            if (!ofs) { ofs.close(); fs::remove(tmpPath); return ""; }
            ofs.flush();
        }
        setRestrictivePermissions(tmpPath);

        fs::rename(tmpPath, finalPath);
        setRestrictivePermissions(finalPath);

        return finalPath.string();
    } catch (...) {
        return "";
    }
}

int main() {
    std::string dir = "uploads_cpp";

    // 1) Valid text
    std::vector<uint8_t> t1{'H','e','l','l','o','\n'};
    std::cout << "Case1: " << uploadFile("readme.txt", t1, dir) << "\n";

    // 2) Valid PNG
    std::vector<uint8_t> png = {0x89,'P','N','G',0x0D,0x0A,0x1A,0x0A,0,0};
    std::cout << "Case2: " << uploadFile("logo.png", png, dir) << "\n";

    // 3) Traversal attempt
    std::cout << "Case3: " << uploadFile("../bad/evil?.txt", std::vector<uint8_t>{'o','k'}, dir) << "\n";

    // 4) Disallowed ext
    std::cout << "Case4: " << uploadFile("malware.exe", std::vector<uint8_t>{'M','Z'}, dir) << "\n";

    // 5) Too large
    std::vector<uint8_t> big(MAX_SIZE + 1, 0);
    std::cout << "Case5: " << uploadFile("big.txt", big, dir) << "\n";

    return 0;
}