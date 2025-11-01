#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/stat.h>
#ifdef _WIN32
#include <io.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

// MySQL C API
#include <mysql/mysql.h>

namespace fs = std::filesystem;

static const size_t MAX_SIZE = 10 * 1024 * 1024; // 10MB
static const std::string UPLOADS_DIR = "uploads";

struct DBConfig {
    std::string host;
    std::string user;
    std::string password;
    std::string database;
    unsigned int port{3306};
};

static std::string sanitizeFilename(const std::string& original) {
    std::string name = original;
    for (auto& ch : name) {
        if (ch == '\\') ch = '/';
    }
    auto pos = name.find_last_of('/');
    if (pos != std::string::npos) name = name.substr(pos + 1);
    std::string out;
    out.reserve(name.size());
    for (unsigned char ch : name) {
        if (std::isalnum(ch) || ch == '.' || ch == '_' || ch == '-') {
            out.push_back(static_cast<char>(ch));
        } else {
            out.push_back('_');
        }
    }
    std::string low = out;
    for (auto& c : low) c = std::tolower(c);
    if (low.size() < 4 || low.substr(low.size() - 4) != ".pdf") {
        out += ".pdf";
    }
    if (out.size() > 100) {
        out = out.substr(out.size() - 100);
    }
    return out;
}

static bool isValidPdf(const std::vector<uint8_t>& data) {
    if (data.empty() || data.size() > MAX_SIZE) return false;
    if (data.size() < 5) return false;
    if (!(data[0] == '%' && data[1] == 'P' && data[2] == 'D' && data[3] == 'F' && data[4] == '-')) return false;
    std::string s(data.begin(), data.end());
    if (s.find("%%EOF") == std::string::npos) return false;
    return true;
}

static fs::path ensureUploadsDir() {
    fs::path dir = fs::absolute(UPLOADS_DIR);
    if (!fs::exists(dir)) {
        fs::create_directories(dir);
#ifndef _WIN32
        ::chmod(dir.string().c_str(), 0700);
#endif
    }
    return fs::canonical(dir);
}

static void saveToDB(const DBConfig* cfg, const std::string& relPath) {
    if (!cfg) return;
    MYSQL* conn = mysql_init(nullptr);
    if (!conn) return;
    if (!mysql_real_connect(conn, cfg->host.c_str(), cfg->user.c_str(), cfg->password.c_str(),
                            cfg->database.c_str(), cfg->port, nullptr, 0)) {
        mysql_close(conn);
        return;
    }
    const char* createSql =
        "CREATE TABLE IF NOT EXISTS documents ("
        "id INT AUTO_INCREMENT PRIMARY KEY,"
        "path VARCHAR(1024) NOT NULL,"
        "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
        ")";
    if (mysql_query(conn, createSql) != 0) {
        mysql_close(conn);
        return;
    }
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    if (!stmt) {
        mysql_close(conn);
        return;
    }
    const char* insertSql = "INSERT INTO documents (path) VALUES (?)";
    if (mysql_stmt_prepare(stmt, insertSql, static_cast<unsigned long>(std::strlen(insertSql))) != 0) {
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return;
    }
    MYSQL_BIND bind{};
    std::memset(&bind, 0, sizeof(bind));
    unsigned long path_len = static_cast<unsigned long>(relPath.size());
    bind.buffer_type = MYSQL_TYPE_STRING;
    bind.buffer = (void*)relPath.c_str();
    bind.buffer_length = path_len;
    bind.length = &path_len;
    if (mysql_stmt_bind_param(stmt, &bind) != 0) {
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return;
    }
    if (mysql_stmt_execute(stmt) != 0) {
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return;
    }
    mysql_stmt_close(stmt);
    mysql_close(conn);
}

std::string upload_pdf(const std::vector<uint8_t>& fileBytes, const std::string& originalFilename, const DBConfig* cfg) {
    if (!isValidPdf(fileBytes)) {
        throw std::runtime_error("Invalid PDF");
    }
    fs::path root = ensureUploadsDir();
    std::string safe = sanitizeFilename(originalFilename);
    std::string base = safe;
    if (base.size() >= 4 && std::string(base.end() - 4, base.end()) == ".pdf") {
        base = std::string(base.begin(), base.end() - 4);
    }

    fs::path target;
    int attempts = 0;
    do {
        std::string unique;
        {
            // simple UUID-like hex
            static const char* hex = "0123456789abcdef";
            for (int i = 0; i < 32; ++i) unique.push_back(hex[rand() % 16]);
        }
        std::string finalName = base + "_" + unique + ".pdf";
        target = root / finalName;
        attempts++;
        if (attempts > 10) throw std::runtime_error("Failed to allocate unique filename");
    } while (fs::exists(target));

#ifndef _WIN32
    int fd = ::open(target.string().c_str(), O_WRONLY | O_CREAT | O_EXCL, 0600);
    if (fd < 0) throw std::runtime_error("Failed to open file");
    ssize_t written = ::write(fd, fileBytes.data(), fileBytes.size());
    ::close(fd);
    if (written < 0 || static_cast<size_t>(written) != fileBytes.size()) {
        std::remove(target.string().c_str());
        throw std::runtime_error("Failed to write file");
    }
#else
    std::ofstream ofs(target, std::ios::binary | std::ios::out);
    if (!ofs) throw std::runtime_error("Failed to open file");
    ofs.write(reinterpret_cast<const char*>(fileBytes.data()), static_cast<std::streamsize>(fileBytes.size()));
    ofs.close();
#endif
    fs::path real = fs::canonical(target);
    if (real.string().rfind(root.string(), 0) != 0) {
        std::remove(real.string().c_str());
        throw std::runtime_error("Path traversal detected");
    }
    fs::path rel = fs::relative(real, root);
    std::string relStr = rel.generic_string();
    saveToDB(cfg, relStr);
    return relStr;
}

std::vector<uint8_t> download_pdf(const std::string& storedPath) {
    fs::path root = ensureUploadsDir();
    fs::path target = fs::weakly_canonical(root / storedPath);
    fs::path real = fs::canonical(target);
    if (real.string().rfind(root.string(), 0) != 0) {
        throw std::runtime_error("Invalid file path");
    }
    std::ifstream ifs(real, std::ios::binary);
    if (!ifs) throw std::runtime_error("Failed to open file");
    std::vector<uint8_t> buf((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    return buf;
}

static DBConfig* cfgFromEnv() {
    const char* host = std::getenv("DB_HOST");
    const char* user = std::getenv("DB_USER");
    const char* pass = std::getenv("DB_PASS");
    const char* name = std::getenv("DB_NAME");
    const char* portStr = std::getenv("DB_PORT");
    if (!host || !user || !pass || !name) return nullptr;
    unsigned int port = portStr ? static_cast<unsigned int>(std::strtoul(portStr, nullptr, 10)) : 3306;
    DBConfig* cfg = new DBConfig{host, user, pass, name, port};
    return cfg;
}

int main() {
    srand((unsigned)time(nullptr));
    DBConfig* cfg = cfgFromEnv();
    std::vector<std::string> filenames = {"report.pdf","invoice..pdf","../../evil.pdf","résumé.pdf","noext"};
    std::vector<std::vector<uint8_t>> pdfs;
    for (int i = 0; i < 5; ++i) {
        std::string s = "%PDF-1.4\n% test " + std::to_string(i) + "\n1 0 obj\n<<>>\nendobj\n%%EOF\n";
        pdfs.emplace_back(s.begin(), s.end());
    }
    std::vector<std::string> paths(5);
    for (int i = 0; i < 5; ++i) {
        try {
            paths[i] = upload_pdf(pdfs[i], filenames[i], cfg);
            std::cout << "Uploaded: " << paths[i] << "\n";
        } catch (const std::exception& e) {
            std::cout << "Upload failed " << i << ": " << e.what() << "\n";
            paths[i].clear();
        }
    }
    for (int i = 0; i < 5; ++i) {
        try {
            if (paths[i].empty()) {
                std::cout << "Skip download " << i << "\n";
                continue;
            }
            auto got = download_pdf(paths[i]);
            bool ok = (got == pdfs[i]);
            std::cout << "Download " << i << " ok=" << (ok ? "true" : "false") << "\n";
        } catch (const std::exception& e) {
            std::cout << "Download failed " << i << ": " << e.what() << "\n";
        }
    }
    delete cfg;
    return 0;
}