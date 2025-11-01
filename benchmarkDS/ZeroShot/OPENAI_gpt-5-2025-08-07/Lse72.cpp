#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include <cctype>

namespace fs = std::filesystem;

static bool is_allowed_char(char c) {
    return (std::isalnum(static_cast<unsigned char>(c)) || c == '.' || c == '_' || c == '-');
}

static std::string sanitize_filename(const std::string& filename) {
    fs::path p(filename);
    std::string base = p.filename().string();
    if (base.empty()) base = "upload.bin";

    std::string cleaned;
    cleaned.reserve(base.size());
    for (char c : base) {
        cleaned.push_back(is_allowed_char(c) ? c : '_');
    }
    if (cleaned.empty()) cleaned = "upload.bin";
    if (!cleaned.empty() && cleaned[0] == '.') cleaned[0] = '_';

    // split name/ext
    std::string name = cleaned;
    std::string ext;
    auto pos = cleaned.find_last_of('.');
    if (pos != std::string::npos && pos != 0 && pos < cleaned.size() - 1) {
        name = cleaned.substr(0, pos);
        ext  = cleaned.substr(pos);
    } else {
        name = cleaned;
        ext.clear();
    }

    // Windows reserved names
    std::string upper = name;
    std::transform(upper.begin(), upper.end(), upper.begin(), [](unsigned char c){ return std::toupper(c); });
    static const std::vector<std::string> reserved = {
        "CON","PRN","AUX","NUL",
        "COM1","COM2","COM3","COM4","COM5","COM6","COM7","COM8","COM9",
        "LPT1","LPT2","LPT3","LPT4","LPT5","LPT6","LPT7","LPT8","LPT9"
    };
    if (std::find(reserved.begin(), reserved.end(), upper) != reserved.end()) {
        name = "_" + name;
    }

    if (ext.empty()) ext = ".bin";

    // limit to 255
    size_t maxTotal = 255;
    if (name.size() + ext.size() > maxTotal) {
        if (maxTotal > ext.size()) {
            name.resize(maxTotal - ext.size());
        } else {
            name = "u"; // fallback
        }
    }
    if (name.empty()) name = "upload";
    return name + ext;
}

std::string upload_file(const std::vector<uint8_t>& data, const std::string& filename) {
    try {
        std::string safe = sanitize_filename(filename);
        fs::path base = fs::current_path();
        fs::path uploads = base / "uploads";
        fs::create_directories(uploads);

        std::string name = safe;
        std::string stem = name;
        std::string ext;
        auto pos = name.find_last_of('.');
        if (pos != std::string::npos && pos != 0) {
            stem = name.substr(0, pos);
            ext = name.substr(pos);
        } else {
            stem = name;
            ext.clear();
        }

        int counter = 0;
        fs::path candidate;
        do {
            std::string attempt = (counter == 0) ? (stem + ext) : (stem + "-" + std::to_string(counter) + ext);
            candidate = uploads / attempt;
            if (!fs::exists(candidate)) break;
            counter++;
        } while (true);

        std::ofstream ofs(candidate, std::ios::binary | std::ios::out);
        if (!ofs) return std::string();
        ofs.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
        ofs.close();
        return fs::absolute(candidate).string();
    } catch (...) {
        return std::string();
    }
}

int main() {
    std::vector<std::pair<std::vector<uint8_t>, std::string>> tests = {
        {std::vector<uint8_t>{'H','e','l','l','o'}, "greeting.txt"},
        {std::vector<uint8_t>{'A','n','o'}, "../evil.txt"},
        {std::vector<uint8_t>{'M','a','l'}, "/etc/passwd"},
        {std::vector<uint8_t>{'A','b','s'}, "COM1"},
        {std::vector<uint8_t>{'S','p','e','c'}, "my file (final)!.pdf"}
    };
    for (auto& t : tests) {
        std::string path = upload_file(t.first, t.second);
        std::cout << path << std::endl;
    }
    return 0;
}