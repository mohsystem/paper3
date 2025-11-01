#include <mysql/mysql.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <sys/stat.h>

static const size_t MAX_FILE_SIZE = 20 * 1024 * 1024; // 20 MB
static const char* TABLE_NAME = "images";

static bool file_exists_regular(const std::string& path, size_t& out_size) {
    struct stat st{};
    if (stat(path.c_str(), &st) != 0) return false;
    if (!S_ISREG(st.st_mode)) return false;
    out_size = static_cast<size_t>(st.st_size);
    return true;
}

static std::string detect_image_mime(const std::vector<unsigned char>& head, const std::string& filename) {
    if (head.size() >= 8 &&
        head[0]==0x89 && head[1]==0x50 && head[2]==0x4E && head[3]==0x47 &&
        head[4]==0x0D && head[5]==0x0A && head[6]==0x1A && head[7]==0x0A) {
        return "image/png";
    }
    if (head.size() >= 3 && head[0]==0xFF && head[1]==0xD8 && head[2]==0xFF) {
        return "image/jpeg";
    }
    if (head.size() >= 6 &&
        head[0]=='G' && head[1]=='I' && head[2]=='F' &&
        head[3]=='8' && (head[4]=='7' || head[4]=='9') && head[5]=='a') {
        return "image/gif";
    }
    if (head.size() >= 12 &&
        head[0]=='R' && head[1]=='I' && head[2]=='F' && head[3]=='F' &&
        head[8]=='W' && head[9]=='E' && head[10]=='B' && head[11]=='P') {
        return "image/webp";
    }
    if (head.size() >= 2 && head[0]=='B' && head[1]=='M') {
        return "image/bmp";
    }
    if (head.size() >= 4 &&
        ((head[0]=='I' && head[1]=='I' && head[2]==0x2A && head[3]==0x00) ||
         (head[0]=='M' && head[1]=='M' && head[2]==0x00 && head[3]==0x2A))) {
        return "image/tiff";
    }
    // fallback by extension
    auto pos = filename.find_last_of('.');
    if (pos != std::string::npos) {
        std::string ext = filename.substr(pos);
        for (auto & c: ext) c = (char)tolower(c);
        if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
        if (ext == ".png") return "image/png";
        if (ext == ".gif") return "image/gif";
        if (ext == ".bmp") return "image/bmp";
        if (ext == ".tif" || ext == ".tiff") return "image/tiff";
        if (ext == ".webp") return "image/webp";
    }
    return "";
}

static std::string sha256_hex(const std::vector<unsigned char>& data) {
    unsigned char digest[SHA256_DIGEST_LENGTH];
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, data.data(), data.size());
    SHA256_Final(digest, &ctx);

    static const char* hex = "0123456789abcdef";
    std::string out;
    out.resize(64);
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        out[2*i]   = hex[(digest[i] >> 4) & 0xF];
        out[2*i+1] = hex[digest[i] & 0xF];
    }
    return out;
}

static void ensure_table_exists(MYSQL* conn) {
    std::string ddl =
        "CREATE TABLE IF NOT EXISTS " + std::string(TABLE_NAME) + " ("
        "id BIGINT NOT NULL AUTO_INCREMENT PRIMARY KEY,"
        "name VARCHAR(255) NOT NULL,"
        "mime_type VARCHAR(100) NOT NULL,"
        "data LONGBLOB NOT NULL,"
        "sha256 CHAR(64) NOT NULL,"
        "created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,"
        "UNIQUE KEY uniq_sha256 (sha256)"
        ") CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci";
    if (mysql_query(conn, ddl.c_str()) != 0) {
        throw std::runtime_error(std::string("DDL error: ") + mysql_error(conn));
    }
}

long long store_image_to_db(const char* host, unsigned int port, const char* user, const char* pass, const char* database, const char* image_path) {
    std::string path(image_path);
    size_t size = 0;
    if (!file_exists_regular(path, size)) {
        throw std::invalid_argument("File not found or not a regular file");
    }
    if (size == 0 || size > MAX_FILE_SIZE) {
        throw std::invalid_argument("Invalid file size");
    }

    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) throw std::runtime_error("Failed to open file");
    std::vector<unsigned char> head(64, 0);
    ifs.read(reinterpret_cast<char*>(head.data()), (std::streamsize)head.size());
    std::streamsize got = ifs.gcount();
    head.resize((size_t)got);
    ifs.clear();
    ifs.seekg(0, std::ios::beg);
    std::vector<unsigned char> data;
    data.resize(size);
    ifs.read(reinterpret_cast<char*>(data.data()), (std::streamsize)size);
    if (!ifs) throw std::runtime_error("Failed to read file");
    ifs.close();

    std::string mime = detect_image_mime(head, path);
    if (mime.empty()) {
        throw std::invalid_argument("Unsupported or unrecognized image format");
    }

    std::string sha256 = sha256_hex(data);

    MYSQL* conn = mysql_init(nullptr);
    if (!conn) throw std::runtime_error("mysql_init failed");

    unsigned int timeout = 10;
    mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
    mysql_options(conn, MYSQL_OPT_READ_TIMEOUT, &timeout);
    mysql_options(conn, MYSQL_OPT_WRITE_TIMEOUT, &timeout);

    if (!mysql_real_connect(conn, host, user, pass, database, port, nullptr, 0)) {
        std::string err = mysql_error(conn);
        mysql_close(conn);
        throw std::runtime_error(std::string("Connect failed: ") + err);
    }

    try {
        ensure_table_exists(conn);

        const char* sql = "INSERT INTO images (name, mime_type, data, sha256) VALUES (?, ?, ?, ?)";
        MYSQL_STMT* stmt = mysql_stmt_init(conn);
        if (!stmt) throw std::runtime_error("mysql_stmt_init failed");
        if (mysql_stmt_prepare(stmt, sql, (unsigned long)strlen(sql)) != 0) {
            std::string err = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error(std::string("mysql_stmt_prepare failed: ") + err);
        }

        std::string fname;
        {
            size_t pos = path.find_last_of("/\\");
            fname = (pos == std::string::npos) ? path : path.substr(pos + 1);
        }

        MYSQL_BIND bind[4];
        memset(bind, 0, sizeof(bind));

        // name
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (void*)fname.c_str();
        bind[0].buffer_length = (unsigned long)fname.size();
        unsigned long name_len = (unsigned long)fname.size();
        bind[0].length = &name_len;

        // mime
        bind[1].buffer_type = MYSQL_TYPE_STRING;
        bind[1].buffer = (void*)mime.c_str();
        bind[1].buffer_length = (unsigned long)mime.size();
        unsigned long mime_len = (unsigned long)mime.size();
        bind[1].length = &mime_len;

        // data
        bind[2].buffer_type = MYSQL_TYPE_LONG_BLOB;
        bind[2].buffer = (void*)data.data();
        bind[2].buffer_length = (unsigned long)data.size();
        unsigned long data_len = (unsigned long)data.size();
        bind[2].length = &data_len;

        // sha256
        bind[3].buffer_type = MYSQL_TYPE_STRING;
        bind[3].buffer = (void*)sha256.c_str();
        bind[3].buffer_length = (unsigned long)sha256.size();
        unsigned long sha_len = (unsigned long)sha256.size();
        bind[3].length = &sha_len;

        if (mysql_stmt_bind_param(stmt, bind) != 0) {
            std::string err = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error(std::string("mysql_stmt_bind_param failed: ") + err);
        }

        if (mysql_stmt_execute(stmt) != 0) {
            std::string err = mysql_stmt_error(stmt);
            mysql_stmt_close(stmt);
            throw std::runtime_error(std::string("mysql_stmt_execute failed: ") + err);
        }

        long long inserted_id = (long long)mysql_stmt_insert_id(stmt);
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return inserted_id;
    } catch (...) {
        mysql_close(conn);
        throw;
    }
}

int main() {
    const char* host = std::getenv("DB_HOST") ? std::getenv("DB_HOST") : "127.0.0.1";
    unsigned int port = std::getenv("DB_PORT") ? (unsigned int)std::strtoul(std::getenv("DB_PORT"), nullptr, 10) : 3306;
    const char* db   = std::getenv("DB_NAME") ? std::getenv("DB_NAME") : "test";
    const char* user = std::getenv("DB_USER") ? std::getenv("DB_USER") : "root";
    const char* pass = std::getenv("DB_PASS") ? std::getenv("DB_PASS") : "";

    const char* tests[5] = {
        "sample1.jpg",
        "sample2.png",
        "not_found_image.bin",
        "test.txt",
        "small.gif"
    };

    for (int i = 0; i < 5; ++i) {
        try {
            long long id = store_image_to_db(host, port, user, pass, db, tests[i]);
            std::cout << "Test " << (i+1) << ": Inserted ID = " << id << " for file " << tests[i] << std::endl;
        } catch (const std::exception& ex) {
            std::cerr << "Test " << (i+1) << ": Failed for file " << tests[i] << " -> " << ex.what() << std::endl;
        }
    }

    return 0;
}