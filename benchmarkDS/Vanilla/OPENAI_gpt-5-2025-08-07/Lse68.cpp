#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <random>
#include <sqlite3.h>

static std::string base64_encode(const unsigned char* data, size_t len) {
    static const char* tbl = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    out.reserve(((len + 2) / 3) * 4);
    for (size_t i = 0; i < len; i += 3) {
        size_t rem = len - i;
        unsigned int b0 = data[i];
        unsigned int b1 = rem > 1 ? data[i + 1] : 0;
        unsigned int b2 = rem > 2 ? data[i + 2] : 0;
        unsigned int val = (b0 << 16) | (b1 << 8) | b2;

        out.push_back(tbl[(val >> 18) & 0x3F]);
        out.push_back(tbl[(val >> 12) & 0x3F]);
        out.push_back(rem > 1 ? tbl[(val >> 6) & 0x3F] : '=');
        out.push_back(rem > 2 ? tbl[val & 0x3F] : '=');
    }
    return out;
}

static bool read_file(const char* path, std::vector<unsigned char>& out) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return false;
    in.seekg(0, std::ios::end);
    std::streampos size = in.tellg();
    if (size < 0) return false;
    out.resize(static_cast<size_t>(size));
    in.seekg(0, std::ios::beg);
    in.read(reinterpret_cast<char*>(out.data()), size);
    return in.good() || in.eof();
}

long long upload_image(const char* db_path, const char* image_path, const char* image_name) {
    std::vector<unsigned char> data;
    if (!read_file(image_path, data)) {
        return -1;
    }
    std::string b64 = base64_encode(data.data(), data.size());

    sqlite3* db = nullptr;
    if (sqlite3_open(db_path, &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return -1;
    }

    const char* create_sql =
        "CREATE TABLE IF NOT EXISTS images ("
        " id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " name TEXT NOT NULL,"
        " data_base64 TEXT NOT NULL"
        ");";
    char* err = nullptr;
    if (sqlite3_exec(db, create_sql, nullptr, nullptr, &err) != SQLITE_OK) {
        if (err) sqlite3_free(err);
        sqlite3_close(db);
        return -1;
    }

    const char* insert_sql = "INSERT INTO images(name, data_base64) VALUES (?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, insert_sql, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        return -1;
    }

    if (sqlite3_bind_text(stmt, 1, image_name, -1, SQLITE_TRANSIENT) != SQLITE_OK ||
        sqlite3_bind_text(stmt, 2, b64.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK) {
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return -1;
    }

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return -1;
    }
    sqlite3_finalize(stmt);

    long long rowid = static_cast<long long>(sqlite3_last_insert_rowid(db));
    sqlite3_close(db);
    return rowid;
}

static void write_dummy(const std::string& path, size_t size, unsigned int seed) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dist(0, 255);
    std::vector<unsigned char> buf(size);
    for (auto& b : buf) b = static_cast<unsigned char>(dist(rng));
    std::ofstream out(path, std::ios::binary);
    out.write(reinterpret_cast<const char*>(buf.data()), buf.size());
}

int main() {
    const char* db_path = "cpp_images.db";
    for (int i = 1; i <= 5; ++i) {
        std::string img_path = "cpp_img_" + std::to_string(i) + ".bin";
        write_dummy(img_path, 22 + i, 300 + i);
        long long id = upload_image(db_path, img_path.c_str(), ("cpp_image_" + std::to_string(i)).c_str());
        std::cout << "C++ inserted row id: " << id << std::endl;
    }
    return 0;
}