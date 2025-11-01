#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <filesystem>
#include <random>

namespace fs = std::filesystem;

static const size_t MAX_SIZE = 10 * 1024 * 1024; // 10 MB
static const char* BASE_DIR = "out_cpp";

static bool isSafeFileName(const std::string& name) {
    if (name.empty() || name.size() > 255) return false;
    if (name == "." || name == "..") return false;
    for (char c : name) {
        bool ok = (c >= 'A' && c <= 'Z') ||
                  (c >= 'a' && c <= 'z') ||
                  (c >= '0' && c <= '9') ||
                  c == '.' || c == '_' || c == '-';
        if (!ok) return false;
    }
    return true;
}

static std::string base64_encode(const std::vector<uint8_t>& data) {
    static const char* tbl =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t in_len = data.size();
    if (in_len == 0) return std::string();
    size_t out_len = ((in_len + 2) / 3) * 4;
    std::string out;
    out.resize(out_len);
    size_t i = 0, j = 0;
    while (i + 2 < in_len) {
        uint32_t n = (data[i] << 16) | (data[i + 1] << 8) | (data[i + 2]);
        out[j++] = tbl[(n >> 18) & 63];
        out[j++] = tbl[(n >> 12) & 63];
        out[j++] = tbl[(n >> 6) & 63];
        out[j++] = tbl[n & 63];
        i += 3;
    }
    if (i < in_len) {
        uint32_t n = data[i] << 16;
        if ((i + 1) < in_len) n |= (data[i + 1] << 8);
        out[j++] = tbl[(n >> 18) & 63];
        out[j++] = tbl[(n >> 12) & 63];
        if ((i + 1) < in_len) {
            out[j++] = tbl[(n >> 6) & 63];
            out[j++] = '=';
        } else {
            out[j++] = '=';
            out[j++] = '=';
        }
    }
    return out;
}

// Returns number of bytes written to the encoded file
size_t upload_image(const std::vector<uint8_t>& imageData, const std::string& outputFileName) {
    if (imageData.size() > MAX_SIZE) {
        throw std::runtime_error("Image too large");
    }
    if (!isSafeFileName(outputFileName)) {
        throw std::runtime_error("Invalid filename");
    }

    fs::path base = fs::weakly_canonical(fs::path(BASE_DIR));
    fs::create_directories(base);
    fs::path out = fs::weakly_canonical(base / outputFileName);
    if (out.string().rfind(base.string(), 0) != 0) {
        throw std::runtime_error("Path traversal detected");
    }
    if (fs::exists(out)) {
        throw std::runtime_error("File already exists");
    }

    std::string encoded = base64_encode(imageData);
    std::ofstream ofs(out, std::ios::binary);
    if (!ofs) {
        throw std::runtime_error("Failed to open output file");
    }
    ofs.write(encoded.data(), static_cast<std::streamsize>(encoded.size()));
    if (!ofs) {
        throw std::runtime_error("Failed to write output file");
    }
    ofs.close();
    return encoded.size();
}

static std::vector<uint8_t> random_bytes(size_t n) {
    std::vector<uint8_t> v(n);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 255);
    for (size_t i = 0; i < n; ++i) v[i] = static_cast<uint8_t>(dist(gen));
    return v;
}

int main() {
    try {
        std::cout << "Test1: " << upload_image(random_bytes(0), "img0.b64") << " bytes written.\n";
    } catch (const std::exception& e) { std::cout << "Test1 failed: " << e.what() << "\n"; }

    try {
        std::cout << "Test2: " << upload_image(random_bytes(10), "img1.b64") << " bytes written.\n";
    } catch (const std::exception& e) { std::cout << "Test2 failed: " << e.what() << "\n"; }

    try {
        std::cout << "Test3: " << upload_image(random_bytes(1024), "img2.b64") << " bytes written.\n";
    } catch (const std::exception& e) { std::cout << "Test3 failed: " << e.what() << "\n"; }

    try {
        std::cout << "Test4: " << upload_image(random_bytes(2048), "img3.b64") << " bytes written.\n";
    } catch (const std::exception& e) { std::cout << "Test4 failed: " << e.what() << "\n"; }

    try {
        std::cout << "Test5: " << upload_image(random_bytes(9999), "img4.b64") << " bytes written.\n";
    } catch (const std::exception& e) { std::cout << "Test5 failed: " << e.what() << "\n"; }

    return 0;
}