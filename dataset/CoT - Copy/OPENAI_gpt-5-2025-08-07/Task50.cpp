#include <bits/stdc++.h>
#include <filesystem>
#include <fstream>

using namespace std;

// Chain-of-Through process:
// 1) Understand: Securely store uploaded files and return confirmation.
// 2) Security: Validate names/ext, limit size, prevent traversal, unique names, restricted directory.
// 3) Implement: Use std::filesystem for paths, sanitize filename, and generate unique names.
// 4) Review: Handle all error paths, avoid overwrites, and restrict permissions where possible.
// 5) Output: Final secure function and tests.

static const size_t MAX_SIZE = 10 * 1024 * 1024; // 10 MB
static const string UPLOAD_DIR = "uploads";
static const unordered_set<string> ALLOWED_EXT = {
    "txt","pdf","png","jpg","jpeg","gif","bin","dat"
};

static string toLowerStr(const string& s) {
    string r = s;
    transform(r.begin(), r.end(), r.begin(), [](unsigned char c){ return (char)tolower(c); });
    return r;
}

static string sanitizeFileName(const string& name) {
    filesystem::path p(name);
    string base = p.filename().string();
    // Remove any slashes/backslashes just in case
    base.erase(remove(base.begin(), base.end(), '/'), base.end());
    base.erase(remove(base.begin(), base.end(), '\\'), base.end());
    if (base.find("..") != string::npos) return "";
    string cleaned;
    cleaned.reserve(base.size());
    for (char c : base) {
        if (isalnum((unsigned char)c) || c=='.' || c=='_' || c=='-') cleaned.push_back(c);
        else cleaned.push_back('_');
    }
    if (cleaned.empty() || cleaned.size() > 100) return "";
    return cleaned;
}

static string getExtension(const string& name) {
    auto pos = name.find_last_of('.');
    if (pos == string::npos || pos == name.size() - 1) return "";
    return toLowerStr(name.substr(pos + 1));
}

static string randomHex(size_t len) {
    static random_device rd;
    static mt19937_64 gen(rd());
    string out;
    out.reserve(len);
    for (size_t i = 0; i < len/16 + 1; ++i) {
        uint64_t v = gen();
        for (int j = 0; j < 16; ++j) {
            int nibble = (int)(v & 0xF);
            out.push_back("0123456789abcdef"[nibble]);
            v >>= 4;
            if (out.size() == len) break;
        }
        if (out.size() == len) break;
    }
    return out;
}

string uploadFile(const string& originalFileName, const vector<unsigned char>& content) {
    if (originalFileName.empty()) return "ERROR: Invalid file name.";
    if (content.size() > MAX_SIZE) {
        return string("ERROR: File too large. Max size is ") + to_string(MAX_SIZE) + " bytes.";
    }

    // Create uploads directory
    try {
        if (!filesystem::exists(UPLOAD_DIR)) {
            filesystem::create_directories(UPLOAD_DIR);
            // Try to restrict perms (POSIX only). If it fails, ignore.
            #ifdef __unix__
            filesystem::permissions(UPLOAD_DIR,
                filesystem::perms::owner_all,
                filesystem::perm_options::replace);
            #endif
        }
    } catch (...) {
        return "ERROR: Could not create upload directory.";
    }

    string safe = sanitizeFileName(originalFileName);
    if (safe.empty()) return "ERROR: Invalid file name.";

    string ext = getExtension(safe);
    if (ext.empty() || ALLOWED_EXT.find(ext) == ALLOWED_EXT.end()) {
        return "ERROR: Disallowed or missing file extension.";
    }

    string base = safe.substr(0, safe.size() - (ext.size() + 1));
    if (base.empty()) base = "file";
    if (base.size() > 64) base = base.substr(0, 64);

    for (int attempt = 0; attempt < 10; ++attempt) {
        string suffix = randomHex(16);
        string finalName = base + "-" + suffix + "." + ext;
        filesystem::path target = filesystem::path(UPLOAD_DIR) / finalName;
        if (filesystem::exists(target)) continue;

        // Use ofstream; note: not strictly O_EXCL; collision mitigated by unique suffix and existence check.
        ofstream out(target, ios::binary | ios::out | ios::trunc);
        if (!out.is_open()) {
            return "ERROR: Failed to store file.";
        }
        out.write(reinterpret_cast<const char*>(content.data()), static_cast<streamsize>(content.size()));
        if (!out.good()) {
            out.close();
            try { filesystem::remove(target); } catch (...) {}
            return "ERROR: Failed to write file.";
        }
        out.close();

        #ifdef __unix__
        try {
            filesystem::permissions(target,
                filesystem::perms::owner_read | filesystem::perms::owner_write,
                filesystem::perm_options::replace);
        } catch (...) {}
        #endif

        return string("OK: Stored as ") + UPLOAD_DIR + "/" + finalName + " (size " + to_string(content.size()) + " bytes).";
    }
    return "ERROR: Could not store file after multiple attempts.";
}

int main() {
    // 5 test cases
    cout << uploadFile("hello.txt", vector<unsigned char>{'H','e','l','l','o'}) << "\n";
    cout << uploadFile("../evil.txt", vector<unsigned char>{'b','a','d'}) << "\n";
    cout << uploadFile("report.pdf", vector<unsigned char>{0x25,0x50,0x44,0x46}) << "\n"; // %PDF
    vector<unsigned char> big(MAX_SIZE + 1, 0);
    cout << uploadFile("image.png", big) << "\n";
    cout << uploadFile("bad?.txt", vector<unsigned char>{'o','k'}) << "\n";
    return 0;
}