// Chain-of-Through process in code generation:
// 1) Problem understanding: Implement upload_image to read image bytes, Base64-encode, and insert into a database.
// 2) Security requirements: Validate image name, limit file size, safe I/O, thread-safe DB.
// 3) Secure coding generation: Use robust checks, RAII, minimal data exposure.
// 4) Code review: Verify bounds, padding, synchronization, and error handling.
// 5) Secure code output: Finalized secure implementation.

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <mutex>
#include <cstdint>
#include <cstdio>

class SafeDatabase {
private:
    std::vector<std::pair<std::string, std::string>> store;
    std::mutex mtx;
public:
    bool insert(const std::string& name, const std::string& b64) {
        std::lock_guard<std::mutex> lock(mtx);
        store.emplace_back(name, b64);
        return true;
    }
    std::vector<std::pair<std::string, std::string>> get_all() {
        std::lock_guard<std::mutex> lock(mtx);
        return store;
    }
    size_t count() {
        std::lock_guard<std::mutex> lock(mtx);
        return store.size();
    }
};

static bool is_valid_name(const std::string& name) {
    if (name.size() < 1 || name.size() > 100) return false;
    for (char c : name) {
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '.' || c == '_' || c == '-')) {
            return false;
        }
    }
    return true;
}

static bool read_file_capped(const std::string& path, std::vector<unsigned char>& out, std::uint64_t maxBytes) {
    std::ifstream in(path, std::ios::binary | std::ios::ate);
    if (!in) return false;
    std::ifstream::pos_type size = in.tellg();
    if (size < 0) return false;
    std::uint64_t usize = static_cast<std::uint64_t>(size);
    if (usize > maxBytes) return false;
    out.resize(static_cast<size_t>(usize));
    in.seekg(0, std::ios::beg);
    if (!in.read(reinterpret_cast<char*>(out.data()), static_cast<std::streamsize>(usize))) {
        return false;
    }
    return true;
}

static std::string base64_encode(const std::vector<unsigned char>& data) {
    static const char tbl[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t in_len = data.size();
    if (in_len == 0) return std::string();
    size_t out_len = 4 * ((in_len + 2) / 3);
    std::string out;
    out.reserve(out_len);
    size_t i = 0;
    while (i + 3 <= in_len) {
        uint32_t n = (data[i] << 16) | (data[i+1] << 8) | data[i+2];
        out.push_back(tbl[(n >> 18) & 63]);
        out.push_back(tbl[(n >> 12) & 63]);
        out.push_back(tbl[(n >> 6) & 63]);
        out.push_back(tbl[n & 63]);
        i += 3;
    }
    if (i < in_len) {
        uint32_t n = 0;
        int rem = static_cast<int>(in_len - i);
        n |= data[i] << 16;
        if (rem == 2) n |= data[i+1] << 8;
        out.push_back(tbl[(n >> 18) & 63]);
        out.push_back(tbl[(n >> 12) & 63]);
        if (rem == 2) {
            out.push_back(tbl[(n >> 6) & 63]);
            out.push_back('=');
        } else {
            out.push_back('=');
            out.push_back('=');
        }
    }
    return out;
}

// upload_image: reads an image from filePath, converts to Base64, inserts into db with imageName. Returns true on success.
bool upload_image(SafeDatabase& db, const std::string& imageName, const std::string& filePath) {
    if (!is_valid_name(imageName)) return false;
    const std::uint64_t MAX_BYTES = 5ull * 1024ull * 1024ull; // 5 MiB
    std::vector<unsigned char> data;
    if (!read_file_capped(filePath, data, MAX_BYTES)) return false;
    std::string b64 = base64_encode(data);
    return db.insert(imageName, b64);
}

static bool write_bytes_to_file(const std::string& path, const std::vector<unsigned char>& data) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out) return false;
    out.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
    return out.good();
}

int main() {
    SafeDatabase db;
    // Create 5 test binary files
    std::vector<std::string> paths;
    std::vector<std::string> names = {"img1.png", "photo-02.jpg", "scan_03.tif", "icon-4.bmp", "graph5.svg"};
    for (int i = 0; i < 5; ++i) {
        size_t sz = 128 + i * 64;
        std::vector<unsigned char> content(sz);
        for (size_t j = 0; j < sz; ++j) {
            content[j] = static_cast<unsigned char>((i * 37 + j) & 0xFF);
        }
        std::string path = "cpp_test_" + std::to_string(i+1) + ".bin";
        if (!write_bytes_to_file(path, content)) {
            std::cout << "Failed to write test file " << path << "\n";
            return 1;
        }
        paths.push_back(path);
    }

    // 5 test cases
    for (int i = 0; i < 5; ++i) {
        bool ok = upload_image(db, names[i], paths[i]);
        std::cout << "Test " << (i+1) << " inserted=" << (ok ? "true" : "false") << "\n";
    }

    std::cout << "Total records: " << db.count() << "\n";
    for (auto& rec : db.get_all()) {
        std::string preview = rec.second.substr(0, std::min<size_t>(20, rec.second.size()));
        std::cout << "Name=" << rec.first << ", b64_preview=" << preview << "...\n";
    }

    return 0;
}