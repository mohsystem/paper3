#include <iostream>
#include <vector>
#include <string>
#include <sqlite3.h>
#include <cctype>
#include <limits>

static const size_t MAX_IMAGE_SIZE = 5 * 1024 * 1024; // 5 MB

static bool is_valid_image_name(const std::string& name) {
    if (name.empty() || name.size() > 255) return false;
    for (unsigned char c : name) {
        if (!(std::isalnum(c) || c == '.' || c == '_' || c == '-')) return false;
    }
    return true;
}

static std::string base64_encode(const unsigned char* data, size_t len) {
    static const char tbl[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    if (len == 0) return std::string();

    // Compute output length safely
    size_t out_len = 4 * ((len + 2) / 3);
    std::string out;
    out.reserve(out_len);

    size_t i = 0;
    while (i + 2 < len) {
        unsigned int n = (data[i] << 16) | (data[i+1] << 8) | data[i+2];
        out.push_back(tbl[(n >> 18) & 63]);
        out.push_back(tbl[(n >> 12) & 63]);
        out.push_back(tbl[(n >> 6) & 63]);
        out.push_back(tbl[n & 63]);
        i += 3;
    }
    if (i < len) {
        unsigned int n = data[i] << 16;
        if (i + 1 < len) n |= (data[i+1] << 8);
        out.push_back(tbl[(n >> 18) & 63]);
        out.push_back(tbl[(n >> 12) & 63]);
        if (i + 1 < len) {
            out.push_back(tbl[(n >> 6) & 63]);
        } else {
            out.push_back('=');
        }
        out.push_back('=');
    }
    // Adjust padding for exact multiples of 3
    size_t mod = len % 3;
    if (mod == 0) {
        // last two '=' added above need to be actual data
        out[out.size() - 1] = tbl[(data[len - 1] & 0x3F)];
        out[out.size() - 2] = tbl[ ((data[len - 2] & 0x0F) << 2) | ((data[len - 1] & 0xC0) >> 6) ];
    } else if (mod == 2) {
        // one '=' correct
    } else if (mod == 1) {
        // two '=' correct
    }
    // Fix: The above adjustment may be off; simpler to recompute tail properly:
    // Recompute tail safely
    out.clear();
    i = 0;
    while (i + 2 < len) {
        unsigned int n = (data[i] << 16) | (data[i+1] << 8) | data[i+2];
        out.push_back(tbl[(n >> 18) & 63]);
        out.push_back(tbl[(n >> 12) & 63]);
        out.push_back(tbl[(n >> 6) & 63]);
        out.push_back(tbl[n & 63]);
        i += 3;
    }
    if (i < len) {
        unsigned int n = data[i] << 16;
        out.push_back(tbl[(n >> 18) & 63]);
        if (i + 1 < len) {
            n |= (data[i+1] << 8);
            out.push_back(tbl[(n >> 12) & 63]);
            out.push_back(tbl[(n >> 6) & 63]);
            out.push_back('=');
        } else {
            out.push_back(tbl[(n >> 12) & 63]);
            out.push_back('=');
            out.push_back('=');
        }
    }
    return out;
}

static int init_db(sqlite3* db) {
    const char* sql =
        "CREATE TABLE IF NOT EXISTS images ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "data TEXT NOT NULL,"
        "created_at TEXT DEFAULT CURRENT_TIMESTAMP)";
    char* err = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
        if (err) sqlite3_free(err);
    }
    return rc;
}

long long upload_image(sqlite3* db, const std::string& image_name, const std::vector<unsigned char>& image_bytes) {
    if (!db) return -1;
    if (!is_valid_image_name(image_name)) return -1;
    if (image_bytes.empty() || image_bytes.size() > MAX_IMAGE_SIZE) return -1;

    std::string b64 = base64_encode(image_bytes.data(), image_bytes.size());
    const char* sql = "INSERT INTO images(name, data) VALUES(?, ?)";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK || !stmt) {
        if (stmt) sqlite3_finalize(stmt);
        return -1;
    }

    rc = sqlite3_bind_text(stmt, 1, image_name.c_str(), -1, SQLITE_TRANSIENT);
    if (rc == SQLITE_OK) rc = sqlite3_bind_text(stmt, 2, b64.c_str(), -1, SQLITE_TRANSIENT);

    long long id = -1;
    if (rc == SQLITE_OK) {
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_DONE) {
            id = static_cast<long long>(sqlite3_last_insert_rowid(db));
        }
    }
    sqlite3_finalize(stmt);
    return id;
}

static std::vector<unsigned char> repeat(unsigned char b, size_t count) {
    return std::vector<unsigned char>(count, b);
}

int main() {
    sqlite3* db = nullptr;
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
        std::cerr << "Failed to open DB\n";
        return 1;
    }
    if (init_db(db) != SQLITE_OK) {
        std::cerr << "Failed to init DB\n";
        sqlite3_close(db);
        return 1;
    }

    std::vector<unsigned char> img1 = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    std::vector<unsigned char> img2 = {0xFF, 0xD8, 0xFF, 0xE0};
    std::vector<unsigned char> img3 = {'G','I','F','8','9','a'};
    std::vector<unsigned char> img4 = repeat(0xAB, 1024);
    std::vector<unsigned char> img5 = repeat(0xCD, 2048);

    long long id1 = upload_image(db, "sample1.png", img1);
    long long id2 = upload_image(db, "photo-02.jpg", img2);
    long long id3 = upload_image(db, "icon.gif", img3);
    long long id4 = upload_image(db, "large_block.bin", img4);
    long long id5 = upload_image(db, "data_chunk.dat", img5);

    std::cout << "Inserted IDs: " << id1 << ", " << id2 << ", " << id3 << ", " << id4 << ", " << id5 << "\n";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM images", -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int cnt = sqlite3_column_int(stmt, 0);
            std::cout << "Total rows: " << cnt << "\n";
        }
        sqlite3_finalize(stmt);
    }

    sqlite3_close(db);
    return 0;
}