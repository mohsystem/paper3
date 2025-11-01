#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include <random>
#include <algorithm>
#include <cctype>
#include <chrono>
#ifdef __unix__
#include <sys/stat.h>
#endif

namespace fs = std::filesystem;

static const size_t MAX_SIZE = 1024 * 1024; // 1 MB

bool is_pdf_header(const std::vector<uint8_t>& data) {
    if (data.size() < 5) return false;
    return data[0]=='%' && data[1]=='P' && data[2]=='D' && data[3]=='F' && data[4]=='-';
}

std::string sanitize_filename(const std::string& in) {
    std::string base = in;
    // Keep only basename
    auto pos1 = base.find_last_of("/\\");
    if (pos1 != std::string::npos) base = base.substr(pos1 + 1);
    // Trim trailing dots
    while (!base.empty() && base.back() == '.') base.pop_back();
    std::string out;
    out.reserve(base.size());
    for (unsigned char c : base) {
        if (std::isalnum(c) || c=='-' || c=='_' || c=='.') out.push_back((char)c);
        else out.push_back('_');
    }
    std::string lower = out;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c){ return (char)std::tolower(c); });
    if (lower.size() >= 4 && lower.rfind(".pdf") == lower.size() - 4) {
        out = out.substr(0, out.size() - 4);
    }
    if (out.empty()) out = "file";
    if (out.size() > 100) out.resize(100);
    return out;
}

std::string random_hex(size_t bytes = 8) {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint32_t> dist(0, 255);
    std::string s;
    s.reserve(bytes * 2);
    const char* hex = "0123456789abcdef";
    for (size_t i = 0; i < bytes; ++i) {
        unsigned v = dist(gen);
        s.push_back(hex[(v >> 4) & 0xF]);
        s.push_back(hex[v & 0xF]);
    }
    return s;
}

int save_pdf(const std::vector<uint8_t>& data, const std::string& original_filename, std::string& out_path) {
    try {
        if (data.empty()) return -1;
        if (data.size() > MAX_SIZE) return -2;
        if (!is_pdf_header(data)) return -3;

        fs::path uploads("uploads");
        std::error_code ec;
        fs::create_directories(uploads, ec);
        (void)ec;
#ifdef __unix__
        chmod(uploads.string().c_str(), 0700);
#endif
        std::string base = sanitize_filename(original_filename);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::system_clock::now().time_since_epoch()).count();
        std::string finalName = base + "_" + std::to_string(ms) + "_" + random_hex(8) + ".pdf";
        if (finalName.size() > 255) finalName.resize(255);
        fs::path temp = uploads / ("upload_" + random_hex(6) + ".tmp");
        {
            std::ofstream ofs(temp, std::ios::binary | std::ios::trunc);
            if (!ofs) return -4;
            ofs.write(reinterpret_cast<const char*>(data.data()), (std::streamsize)data.size());
            if (!ofs) return -5;
            ofs.flush();
        }
#ifdef __unix__
        chmod(temp.string().c_str(), 0600);
#endif
        fs::path finalPath = uploads / finalName;
        fs::rename(temp, finalPath, ec);
        if (ec) return -6;
#ifdef __unix__
        chmod(finalPath.string().c_str(), 0600);
#endif
        out_path = fs::absolute(finalPath).string();
        return 0;
    } catch (...) {
        return -9;
    }
}

int main() {
    // Test 1: Valid PDF
    std::vector<uint8_t> valid_pdf = {'%', 'P', 'D', 'F', '-', '1', '.', '7', '\n', '%', 'T', 'e', 's', 't'};
    std::string out;
    std::cout << "Test1: " << save_pdf(valid_pdf, "test.pdf", out) << " " << out << "\n";

    // Test 2: Wrong size (empty)
    out.clear();
    std::vector<uint8_t> empty;
    std::cout << "Test2: " << save_pdf(empty, "test.pdf", out) << "\n";

    // Test 3: Invalid header
    out.clear();
    std::vector<uint8_t> bad = {'H', 'E', 'L', 'L', 'O'};
    std::cout << "Test3: " << save_pdf(bad, "doc.pdf", out) << "\n";

    // Test 4: Path traversal in filename
    out.clear();
    std::cout << "Test4: " << save_pdf(valid_pdf, "../evil.pdf", out) << " " << out << "\n";

    // Test 5: Too large
    out.clear();
    std::vector<uint8_t> big(MAX_SIZE + 10, 0);
    big[0] = '%'; big[1] = 'P'; big[2] = 'D'; big[3] = 'F'; big[4] = '-';
    std::cout << "Test5: " << save_pdf(big, "big.pdf", out) << "\n";

    return 0;
}