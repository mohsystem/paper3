#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <filesystem>
#include <random>
#include <cctype>
#include <algorithm>

// Chain-of-Through Step 1: Problem understanding
// upload_file saves uploaded data to 'uploads' directory securely and returns the saved path or an error message.

// Chain-of-Through Step 2: Security requirements handled via checks.
static const size_t MAX_SIZE_BYTES = 1'000'000; // 1 MB
static const std::set<std::string> ALLOWED_EXTS = {
    "txt","png","jpg","jpeg","pdf","zip","gif","bmp","csv","json"
};

static std::string toLower(const std::string& s) {
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c){ return std::tolower(c); });
    return out;
}

static std::string basenameOnly(const std::string& path) {
    std::filesystem::path p(path);
    return p.filename().string();
}

static std::string getExt(const std::string& filename) {
    auto base = basenameOnly(filename);
    auto pos = base.find_last_of('.');
    if (pos == std::string::npos || pos == 0 || pos == base.size() - 1) return "";
    return base.substr(pos + 1);
}

static std::string stripExt(const std::string& filename) {
    auto base = basenameOnly(filename);
    auto pos = base.find_last_of('.');
    if (pos == std::string::npos || pos == 0) return base;
    return base.substr(0, pos);
}

static std::string sanitizeName(const std::string& in) {
    std::string out;
    out.reserve(std::min<size_t>(in.size(), 100));
    for (char c : in) {
        if (out.size() >= 100) break;
        if (std::isalnum(static_cast<unsigned char>(c)) || c == '.' || c == '_' || c == '-') {
            out.push_back(c);
        } else {
            out.push_back('_');
        }
    }
    if (out.empty() || (!out.empty() && out[0] == '.')) out = "file";
    return out;
}

static std::string randomHex(size_t bytes) {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dist;
    std::string s;
    s.reserve(bytes * 2);
    for (size_t i = 0; i < bytes; i += 8) {
        uint64_t val = dist(gen);
        for (int j = 0; j < 8 && i + j < bytes; ++j) {
            unsigned char b = (val >> (j * 8)) & 0xFF;
            const char* hex = "0123456789abcdef";
            s.push_back(hex[(b >> 4) & 0xF]);
            s.push_back(hex[b & 0xF]);
        }
    }
    return s;
}

std::string upload_file(const std::string& originalFilename, const std::vector<unsigned char>& data, const std::string& contentType) {
    // Chain-of-Through Step 3: Secure coding generation
    if (data.empty()) return "ERROR: Empty or no data";
    if (data.size() > MAX_SIZE_BYTES) return "ERROR: File too large";
    if (originalFilename.empty()) return "ERROR: Missing filename";

    std::string ext = toLower(getExt(originalFilename));
    if (ext.empty() || ALLOWED_EXTS.find(ext) == ALLOWED_EXTS.end()) {
        return "ERROR: Disallowed file extension";
    }

    std::string safeBase = sanitizeName(stripExt(originalFilename));
    std::string unique = safeBase + "_" + randomHex(8) + "." + ext;

    std::filesystem::path uploads("uploads");
    std::error_code ec;
    std::filesystem::create_directories(uploads, ec);
    if (ec) return "ERROR: Could not create uploads directory";

    std::filesystem::path target = (uploads / unique);
    target = std::filesystem::weakly_canonical(target, ec).empty() ? std::filesystem::absolute(target) : std::filesystem::weakly_canonical(target);
    auto uploadsAbs = std::filesystem::weakly_canonical(uploads, ec);
    if (ec) uploadsAbs = std::filesystem::absolute(uploads);

    // Ensure path is within uploads
    auto targetStr = target.string();
    auto uploadsStr = uploadsAbs.string();
#ifdef _WIN32
    std::transform(targetStr.begin(), targetStr.end(), targetStr.begin(), ::tolower);
    std::transform(uploadsStr.begin(), uploadsStr.end(), uploadsStr.begin(), ::tolower);
#endif
    if (targetStr.find(uploadsStr) != 0) {
        return "ERROR: Invalid path";
    }

    // Attempt exclusive create by checking existence then opening
    if (std::filesystem::exists(target)) {
        return "ERROR: Name collision, try again";
    }

    std::ofstream ofs(target, std::ios::binary | std::ios::out);
    if (!ofs) return "ERROR: Write failed";
    ofs.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
    if (!ofs.good()) return "ERROR: Write failed";
    ofs.close();

    // Chain-of-Through Step 4 & 5: Review and finalize
    return target.string();
}

int main() {
    // 5 test cases
    std::vector<unsigned char> d1 = {'H','e','l','l','o'};
    std::cout << "Test1: " << upload_file("hello.txt", d1, "text/plain") << "\n";

    std::vector<unsigned char> d2 = {'f','a','k','e'};
    std::cout << "Test2: " << upload_file("../../etc/passwd", d2, "text/plain") << "\n"; // no ext -> error

    std::vector<unsigned char> d3 = {0x89, 'P', 'N', 'G'};
    std::cout << "Test3: " << upload_file("img.png", d3, "image/png") << "\n";

    std::vector<unsigned char> d4 = {'M','Z'};
    std::cout << "Test4: " << upload_file("virus.exe", d4, "application/octet-stream") << "\n";

    std::vector<unsigned char> d5(MAX_SIZE_BYTES + 1, 0);
    std::cout << "Test5: " << upload_file("big.pdf", d5, "application/pdf") << "\n";

    return 0;
}