#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <random>
#include <chrono>
#include <unordered_set>
#include <filesystem>

namespace fs = std::filesystem;

static const size_t MAX_SIZE = 5u * 1024u * 1024u;

static std::string sanitizeFilename(const std::string& name) {
    std::string out;
    out.reserve(name.size());
    for (unsigned char c : name) {
        if (std::isalnum(c) || c == '.' || c == '_' || c == '-') {
            out.push_back(c);
        } else {
            out.push_back('_');
        }
    }
    while (!out.empty() && out.front() == '.') {
        out.erase(out.begin());
    }
    if (out.empty()) out = "file";
    if (out.size() > 255) out.resize(255);
    return out;
}

static std::string allowedExtOrDefault(const std::string& filename, const std::unordered_set<std::string>& allowed, const std::string& defExt) {
    auto pos = filename.find_last_of('.');
    if (pos != std::string::npos && pos + 1 < filename.size()) {
        std::string ext = filename.substr(pos + 1);
        for (auto& c : ext) c = (char)std::tolower((unsigned char)c);
        if (allowed.count(ext)) return ext;
    }
    return defExt;
}

static std::string randomHex(size_t bytes) {
    std::random_device rd;
    std::vector<unsigned char> buf(bytes);
    for (size_t i = 0; i < bytes; ++i) buf[i] = (unsigned char)rd();
    static const char* hex = "0123456789abcdef";
    std::string out;
    out.reserve(bytes * 2);
    for (unsigned char b : buf) {
        out.push_back(hex[(b >> 4) & 0xF]);
        out.push_back(hex[b & 0xF]);
    }
    return out;
}

static void setOwnerOnlyPermissions(const fs::path& p, bool isDir) {
    try {
        fs::permissions(p,
                        fs::perms::owner_read | fs::perms::owner_write | (isDir ? fs::perms::owner_exec : fs::perms::none),
                        fs::perm_options::replace);
    } catch (...) {
    }
}

std::string uploadFile(const std::string& storageDir, const std::string& originalFilename, const std::vector<uint8_t>& content) {
    if (originalFilename.empty()) throw std::invalid_argument("originalFilename is empty.");
    if (content.empty()) throw std::invalid_argument("File content is empty.");
    if (content.size() > MAX_SIZE) throw std::invalid_argument("File too large. Limit is " + std::to_string(MAX_SIZE) + " bytes.");

    fs::path dir(storageDir);
    fs::create_directories(dir);
    setOwnerOnlyPermissions(dir, true);

    static const std::unordered_set<std::string> allowed = {"txt","pdf","png","jpg","jpeg","gif","bin"};

    std::string safe = sanitizeFilename(originalFilename);
    std::string ext = allowedExtOrDefault(safe, allowed, "bin");
    auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch().count();
    std::string uniqueName = "f-" + std::to_string(now) + "-" + randomHex(16) + "." + ext;

    fs::path finalPath = dir / uniqueName;
    fs::path tmpPath = dir / ("upload-" + randomHex(8) + ".tmp");

    // Write to temp then move
    {
        std::ofstream ofs(tmpPath, std::ios::binary | std::ios::out | std::ios::trunc);
        if (!ofs) throw std::runtime_error("Failed to open temp file for writing.");
        ofs.write(reinterpret_cast<const char*>(content.data()), static_cast<std::streamsize>(content.size()));
        if (!ofs) {
            ofs.close();
            std::error_code ec;
            fs::remove(tmpPath, ec);
            throw std::runtime_error("Failed to write file content.");
        }
        ofs.close();
    }
    setOwnerOnlyPermissions(tmpPath, false);

    std::error_code ec;
    fs::rename(tmpPath, finalPath, ec);
    if (ec) {
        // fallback: replace_existing
        fs::remove(finalPath, ec);
        ec.clear();
        fs::rename(tmpPath, finalPath, ec);
        if (ec) {
            fs::remove(tmpPath);
            throw std::runtime_error("Failed to move temp file to final destination.");
        }
    }
    setOwnerOnlyPermissions(finalPath, false);

    return "Upload successful. Stored as " + finalPath.filename().string() + " (" + std::to_string(content.size()) + " bytes).";
}

int main() {
    std::string storage = "uploads_cpp";
    std::vector<std::pair<std::string, std::vector<uint8_t>>> tests;

    tests.push_back({"hello.txt", std::vector<uint8_t>{'H','e','l','l','o',' ','W','o','r','l','d'}});
    tests.push_back({"../../etc/passwd", std::vector<uint8_t>{'N','o','t',' ','p','a','s','s','w','d'}});
    tests.push_back({"image.PNG", std::vector<uint8_t>{1,2,3,4,5}});
    tests.push_back({"script.exe", std::vector<uint8_t>{'b','i','n','a','r','y'}});
    std::vector<uint8_t> big(MAX_SIZE + 1024, 0xAB);
    tests.push_back({"too_big.pdf", big});

    for (size_t i = 0; i < tests.size(); ++i) {
        try {
            auto msg = uploadFile(storage, tests[i].first, tests[i].second);
            std::cout << (i+1) << ": " << msg << "\n";
        } catch (const std::exception& e) {
            std::cout << (i+1) << ": Upload failed: " << e.what() << "\n";
        }
    }
    return 0;
}