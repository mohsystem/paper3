// C++ implementation using MySQL C API (libmysqlclient)
// Chain-of-Through process (brief):
// 1) Goal: store image file into MySQL BLOB.
// 2) Security: validate identifiers, use prepared statements, size limit, robust error handling.
// 3) Secure coding: mysql_stmt with bound parameters, safe table creation with identifier validation.
// 4) Review: avoid SQLi, handle resources and errors.
// 5) Output: final secure function + 5 tests.

#include <mysql.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <regex>
#include <fstream>
#include <iostream>
#include <cstdlib>

static const size_t MAX_FILE_SIZE_BYTES_CPP = 10 * 1024 * 1024; // 10MB

static bool safeIdentifier(const std::string& s) {
    if (s.empty() || s.size() > 64) return false;
    static const std::regex re("^[A-Za-z0-9_]+$");
    return std::regex_match(s, re);
}

static std::string fileNameFromPath(const std::string& p) {
    size_t pos = p.find_last_of("/\\");
    if (pos == std::string::npos) return p;
    return p.substr(pos + 1);
}

static std::string mimeFromExt(const std::string& path) {
    std::string lower = path;
    for (auto& c: lower) c = (char)tolower((unsigned char)c);
    if (lower.size() >= 4 && lower.substr(lower.size()-4) == ".png") return "image/png";
    if ((lower.size() >= 4 && lower.substr(lower.size()-4) == ".jpg") ||
        (lower.size() >= 5 && lower.substr(lower.size()-5) == ".jpeg")) return "image/jpeg";
    if (lower.size() >= 4 && lower.substr(lower.size()-4) == ".gif") return "image/gif";
    if (lower.size() >= 4 && lower.substr(lower.size()-4) == ".bmp") return "image/bmp";
    if (lower.size() >= 5 && lower.substr(lower.size()-5) == ".webp") return "image/webp";
    return "application/octet-stream";
}

static bool readFileBytes(const std::string& path, std::vector<unsigned char>& out) {
    std::ifstream f(path.c_str(), std::ios::binary);
    if (!f) return false;
    f.seekg(0, std::ios::end);
    std::streamoff sz = f.tellg();
    if (sz <= 0 || sz > (std::streamoff)MAX_FILE_SIZE_BYTES_CPP) return false;
    f.seekg(0, std::ios::beg);
    out.resize((size_t)sz);
    if (!f.read(reinterpret_cast<char*>(out.data()), sz)) return false;
    return true;
}

bool storeImageToMySQL(
    const std::string& host,
    unsigned int port,
    const std::string& user,
    const std::string& pass,
    const std::string& db,
    const std::string& table,
    const std::string& recordId,
    const std::string& filePath
) {
    if (!safeIdentifier(table)) return false;
    if (recordId.empty()) return false;

    std::vector<unsigned char> data;
    if (!readFileBytes(filePath, data)) return false;
    std::string filename = fileNameFromPath(filePath);
    std::string mime = mimeFromExt(filePath);

    MYSQL* conn = mysql_init(nullptr);
    if (!conn) return false;

    // Optional timeouts
    unsigned int timeout_sec = 8;
    mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, &timeout_sec);
    mysql_options(conn, MYSQL_OPT_READ_TIMEOUT, &timeout_sec);
    mysql_options(conn, MYSQL_OPT_WRITE_TIMEOUT, &timeout_sec);

    if (!mysql_real_connect(conn, host.c_str(), user.c_str(), pass.c_str(), db.c_str(), port, nullptr, 0)) {
        mysql_close(conn);
        return false;
    }

    bool ok = false;
    if (mysql_autocommit(conn, 0) != 0) {
        mysql_close(conn);
        return false;
    }

    std::string createSQL = "CREATE TABLE IF NOT EXISTS `" + table + "` ("
                            "id VARCHAR(128) PRIMARY KEY,"
                            "filename VARCHAR(255) NOT NULL,"
                            "mime_type VARCHAR(100) NOT NULL,"
                            "size_bytes BIGINT NOT NULL,"
                            "data LONGBLOB NOT NULL)";
    if (mysql_query(conn, createSQL.c_str()) != 0) {
        mysql_rollback(conn);
        mysql_autocommit(conn, 1);
        mysql_close(conn);
        return false;
    }

    std::string insertSQL = "REPLACE INTO `" + table + "` (id, filename, mime_type, size_bytes, data) VALUES (?,?,?,?,?)";
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    if (!stmt) {
        mysql_rollback(conn);
        mysql_autocommit(conn, 1);
        mysql_close(conn);
        return false;
    }
    if (mysql_stmt_prepare(stmt, insertSQL.c_str(), (unsigned long)insertSQL.size()) != 0) {
        mysql_stmt_close(stmt);
        mysql_rollback(conn);
        mysql_autocommit(conn, 1);
        mysql_close(conn);
        return false;
    }

    MYSQL_BIND bind[5];
    memset(bind, 0, sizeof(bind));

    // id
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (void*)recordId.c_str();
    unsigned long id_len = (unsigned long)recordId.size();
    bind[0].length = &id_len;

    // filename
    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = (void*)filename.c_str();
    unsigned long fname_len = (unsigned long)filename.size();
    bind[1].length = &fname_len;

    // mime_type
    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].buffer = (void*)mime.c_str();
    unsigned long mime_len = (unsigned long)mime.size();
    bind[2].length = &mime_len;

    // size_bytes
    long long size_bytes = (long long)data.size();
    bind[3].buffer_type = MYSQL_TYPE_LONGLONG;
    bind[3].buffer = (void*)&size_bytes;

    // data (BLOB)
    bind[4].buffer_type = MYSQL_TYPE_LONG_BLOB;
    bind[4].buffer = (void*)data.data();
    unsigned long blob_len = (unsigned long)data.size();
    bind[4].length = &blob_len;

    if (mysql_stmt_bind_param(stmt, bind) != 0) {
        mysql_stmt_close(stmt);
        mysql_rollback(conn);
        mysql_autocommit(conn, 1);
        mysql_close(conn);
        return false;
    }

    if (mysql_stmt_execute(stmt) == 0) {
        ok = true;
        mysql_commit(conn);
    } else {
        mysql_rollback(conn);
    }

    mysql_stmt_close(stmt);
    mysql_autocommit(conn, 1);
    mysql_close(conn);
    return ok;
}

static std::vector<unsigned char> tiny_png_cpp() {
    unsigned char arr[] = {0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A,0x00,0x00,0x00,0x00,0x49,0x45,0x4E,0x44,0xAE,0x42,0x60,0x82};
    return std::vector<unsigned char>(arr, arr + sizeof(arr));
}

static std::string writeTempImg(const std::string& suffix) {
    std::string path = std::string("imgcpp_") + std::to_string(rand()) + suffix;
    std::ofstream f(path.c_str(), std::ios::binary);
    auto b = tiny_png_cpp();
    f.write(reinterpret_cast<const char*>(b.data()), (std::streamsize)b.size());
    f.close();
    return path;
}

int main() {
    // 5 test cases
    std::string host = std::getenv("DB_HOST") ? std::getenv("DB_HOST") : "127.0.0.1";
    unsigned int port = std::getenv("DB_PORT") ? (unsigned int)std::stoi(std::getenv("DB_PORT")) : 3306;
    std::string user = std::getenv("DB_USER") ? std::getenv("DB_USER") : "root";
    std::string pass = std::getenv("DB_PASS") ? std::getenv("DB_PASS") : "password";
    std::string db = std::getenv("DB_NAME") ? std::getenv("DB_NAME") : "test";
    std::string table = std::getenv("DB_TABLE") ? std::getenv("DB_TABLE") : "images_secure_cpp";

    std::string p1 = writeTempImg(".png");
    std::string p2 = writeTempImg(".jpg");
    std::string p3 = writeTempImg(".gif");
    std::string p4 = writeTempImg(".webp");
    std::string p5 = writeTempImg(".bmp");

    storeImageToMySQL(host, port, user, pass, db, table, "cpp_img1", p1);
    storeImageToMySQL(host, port, user, pass, db, table, "cpp_img2", p2);
    storeImageToMySQL(host, port, user, pass, db, table, "cpp_img3", p3);
    storeImageToMySQL(host, port, user, pass, db, table, "cpp_img4", p4);
    storeImageToMySQL(host, port, user, pass, db, table, "cpp_img5", p5);

    // Cleanup files (best-effort)
    std::remove(p1.c_str());
    std::remove(p2.c_str());
    std::remove(p3.c_str());
    std::remove(p4.c_str());
    std::remove(p5.c_str());

    return 0;
}