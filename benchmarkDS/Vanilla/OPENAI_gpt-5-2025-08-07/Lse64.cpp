#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <random>
#include <iomanip>
#include <sstream>
#include <mysql/mysql.h>

class Lse64 {
public:
    static constexpr const char* DB_HOST = "127.0.0.1";
    static constexpr unsigned int DB_PORT = 3306;
    static constexpr const char* DB_USER = "root";
    static constexpr const char* DB_PASS = "";
    static constexpr const char* DB_NAME = "test";
    static constexpr const char* UPLOAD_DIR = "uploads";

    static void ensureUploadsDir() {
        std::filesystem::create_directories(UPLOAD_DIR);
    }

    static MYSQL* getConnection() {
        MYSQL* conn = mysql_init(nullptr);
        if (!conn) throw std::runtime_error("mysql_init failed");
        if (!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, DB_PORT, nullptr, 0)) {
            std::string err = mysql_error(conn);
            mysql_close(conn);
            throw std::runtime_error("mysql_real_connect failed: " + err);
        }
        return conn;
    }

    static void ensureTable(MYSQL* conn) {
        const char* ddl =
            "CREATE TABLE IF NOT EXISTS pdf_uploads ("
            " id BIGINT AUTO_INCREMENT PRIMARY KEY,"
            " filename VARCHAR(255) NOT NULL,"
            " filepath VARCHAR(1024) NOT NULL,"
            " uploaded_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
            ")";
        if (mysql_query(conn, ddl) != 0) {
            throw std::runtime_error(std::string("DDL error: ") + mysql_error(conn));
        }
    }

    static long upload_pdf(const std::vector<unsigned char>& data, const std::string& originalFilename) {
        ensureUploadsDir();
        std::string safe = sanitizeFilename(originalFilename);
        if (safe.size() < 4 || !endsWithPdf(safe)) safe += ".pdf";
        std::string unique = uniqueStamp();
        std::string finalName = unique + "-" + safe;
        std::filesystem::path path = std::filesystem::absolute(std::filesystem::path(UPLOAD_DIR) / finalName);

        // write file
        std::ofstream ofs(path, std::ios::binary);
        ofs.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
        ofs.close();

        MYSQL* conn = getConnection();
        try {
            ensureTable(conn);

            // escape strings
            std::string escName = escapeString(conn, originalFilename);
            std::string escPath = escapeString(conn, path.string());

            std::string sql = "INSERT INTO pdf_uploads (filename, filepath) VALUES ('" + escName + "', '" + escPath + "')";
            if (mysql_query(conn, sql.c_str()) != 0) {
                std::string err = mysql_error(conn);
                mysql_close(conn);
                throw std::runtime_error("Insert error: " + err);
            }
            long id = static_cast<long>(mysql_insert_id(conn));
            mysql_close(conn);
            return id;
        } catch (...) {
            mysql_close(conn);
            throw;
        }
    }

    static std::vector<unsigned char> download_pdf(long id) {
        MYSQL* conn = getConnection();
        try {
            std::ostringstream oss;
            oss << "SELECT filepath FROM pdf_uploads WHERE id=" << id;
            if (mysql_query(conn, oss.str().c_str()) != 0) {
                std::string err = mysql_error(conn);
                mysql_close(conn);
                throw std::runtime_error("Select error: " + err);
            }
            MYSQL_RES* res = mysql_store_result(conn);
            if (!res) {
                std::string err = mysql_error(conn);
                mysql_close(conn);
                throw std::runtime_error("Store result error: " + err);
            }
            MYSQL_ROW row = mysql_fetch_row(res);
            if (!row || !row[0]) {
                mysql_free_result(res);
                mysql_close(conn);
                throw std::runtime_error("Record not found");
            }
            std::string filepath = row[0];
            mysql_free_result(res);
            mysql_close(conn);

            std::ifstream ifs(filepath, std::ios::binary);
            if (!ifs) throw std::runtime_error("File not found: " + filepath);
            std::vector<unsigned char> buf((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
            return buf;
        } catch (...) {
            mysql_close(conn);
            throw;
        }
    }

private:
    static std::string sanitizeFilename(const std::string& name) {
        std::string out;
        for (char c : name) {
            if (std::isalnum(static_cast<unsigned char>(c)) || c == '.' || c == '_' || c == '-') out.push_back(c);
            else out.push_back('_');
        }
        if (out.empty()) out = "upload.pdf";
        return out;
    }

    static bool endsWithPdf(const std::string& s) {
        if (s.size() < 4) return false;
        std::string tail = s.substr(s.size() - 4);
        for (auto& ch : tail) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
        return tail == ".pdf";
    }

    static std::string uniqueStamp() {
        auto now = std::chrono::system_clock::now().time_since_epoch();
        auto micros = std::chrono::duration_cast<std::chrono::microseconds>(now).count();
        std::mt19937_64 rng(static_cast<uint64_t>(micros));
        std::uniform_int_distribution<uint64_t> dist;
        std::ostringstream oss;
        oss << micros << "-" << std::hex << dist(rng);
        return oss.str();
    }

    static std::string escapeString(MYSQL* conn, const std::string& s) {
        std::string out;
        out.resize(s.size() * 2 + 1);
        unsigned long len = mysql_real_escape_string(conn, &out[0], s.c_str(), static_cast<unsigned long>(s.size()));
        out.resize(len);
        return out;
    }
};

static std::vector<unsigned char> makeMinimalPdf(const std::string& message) {
    std::ostringstream oss;
    oss << "%PDF-1.4\n";
    // base64 message as comment
    static const char* b64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    auto toBase64 = [&](const std::string& m) {
        std::string out;
        int val = 0, valb = -6;
        for (uint8_t c : m) {
            val = (val << 8) + c;
            valb += 8;
            while (valb >= 0) {
                out.push_back(b64[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }
        if (valb > -6) out.push_back(b64[((val << 8) >> (valb + 8)) & 0x3F]);
        while (out.size() % 4) out.push_back('=');
        return out;
    };
    oss << "% " << toBase64(message) << "\n";
    oss << "1 0 obj\n<< /Type /Catalog /Pages 2 0 R >>\nendobj\n";
    oss << "2 0 obj\n<< /Type /Pages /Count 0 >>\nendobj\n";
    oss << "trailer\n<< /Root 1 0 R >>\n%%EOF\n";
    std::string s = oss.str();
    return std::vector<unsigned char>(s.begin(), s.end());
}

int main() {
    try {
        for (int i = 1; i <= 5; ++i) {
            std::vector<unsigned char> pdf = makeMinimalPdf("Test PDF " + std::to_string(i));
            long id = Lse64::upload_pdf(pdf, "file" + std::to_string(i) + ".pdf");
            std::vector<unsigned char> data = Lse64::download_pdf(id);
            std::cout << "Inserted id=" << id << ", downloaded bytes=" << data.size() << std::endl;
        }
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}