#include <mysql/mysql.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>

#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif
#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif

static const size_t MAX_IMAGE_BYTES = 10u * 1024u * 1024u; // 10 MiB

struct FdCloser {
    int fd;
    explicit FdCloser(int f = -1) : fd(f) {}
    ~FdCloser() { if (fd >= 0) ::close(fd); }
    FdCloser(const FdCloser&) = delete;
    FdCloser& operator=(const FdCloser&) = delete;
    FdCloser(FdCloser&& other) noexcept : fd(other.fd) { other.fd = -1; }
};

static bool safe_table_name(const std::string& t) {
    if (t.empty() || t.size() > 64) return false;
    for (char c : t) {
        if (!( (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
               (c >= '0' && c <= '9') || c == '_' )) return false;
    }
    return true;
}

static bool detect_mime(const std::vector<unsigned char>& data, std::string& out_mime) {
    if (data.size() >= 8) {
        const unsigned char png[8] = {0x89,'P','N','G',0x0D,0x0A,0x1A,0x0A};
        if (memcmp(data.data(), png, 8) == 0) { out_mime = "image/png"; return true; }
    }
    if (data.size() >= 3) {
        if (data[0]==0xFF && data[1]==0xD8 && data[2]==0xFF) { out_mime = "image/jpeg"; return true; }
    }
    if (data.size() >= 6) {
        if (data[0]=='G' && data[1]=='I' && data[2]=='F' && data[3]=='8' && data[5]=='a') {
            out_mime = "image/gif"; return true;
        }
    }
    if (data.size() >= 2) {
        if (data[0]=='B' && data[1]=='M') { out_mime = "image/bmp"; return true; }
    }
    if (data.size() >= 12) {
        if (data[0]=='R' && data[1]=='I' && data[2]=='F' && data[3]=='F' &&
            data[8]=='W' && data[9]=='E' && data[10]=='B' && data[11]=='P') {
            out_mime = "image/webp"; return true;
        }
    }
    return false;
}

static bool allowed_extension(const std::string& rel, const std::string& mime) {
    auto lower = [](std::string s){ for (auto& c: s) c = (char)tolower((unsigned char)c); return s; };
    std::string p = lower(rel);
    if (mime == "image/png") return p.size()>=4 && p.rfind(".png")==p.size()-4;
    if (mime == "image/jpeg") {
        return (p.size()>=4 && p.rfind(".jpg")==p.size()-4) ||
               (p.size()>=5 && p.rfind(".jpeg")==p.size()-5);
    }
    if (mime == "image/gif") return p.size()>=4 && p.rfind(".gif")==p.size()-4;
    if (mime == "image/bmp") return p.size()>=4 && p.rfind(".bmp")==p.size()-4;
    if (mime == "image/webp") return p.size()>=5 && p.rfind(".webp")==p.size()-5;
    return false;
}

static bool read_image_secure(const std::string& base_dir, const std::string& rel_path, std::vector<unsigned char>& out, std::string& mime) {
    if (rel_path.empty() || (!rel_path.empty() && rel_path[0] == '/')) {
        std::cerr << "Error: invalid path.\n";
        return false;
    }
    struct stat st{};
    if (lstat(base_dir.c_str(), &st) != 0) {
        std::cerr << "Error: base directory not accessible.\n";
        return false;
    }
    if (!S_ISDIR(st.st_mode) || S_ISLNK(st.st_mode)) {
        std::cerr << "Error: base directory invalid.\n";
        return false;
    }
    int dirfd = open(base_dir.c_str(), O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (dirfd < 0) {
        std::cerr << "Error: cannot open base directory.\n";
        return false;
    }
    FdCloser d(dirfd);

    int flags = O_RDONLY | O_CLOEXEC | O_NOFOLLOW;
    int fd = openat(dirfd, rel_path.c_str(), flags);
    if (fd < 0) {
        std::cerr << "Error: cannot open file.\n";
        return false;
    }
    FdCloser f(fd);

    struct stat fst{};
    if (fstat(fd, &fst) != 0) {
        std::cerr << "Error: cannot stat file.\n";
        return false;
    }
    if (!S_ISREG(fst.st_mode)) {
        std::cerr << "Error: not a regular file.\n";
        return false;
    }
    if (fst.st_size <= 0 || (size_t)fst.st_size > MAX_IMAGE_BYTES) {
        std::cerr << "Error: invalid file size.\n";
        return false;
    }

    out.clear();
    out.reserve((size_t)fst.st_size);
    unsigned char buf[8192];
    size_t total = 0;
    while (true) {
        ssize_t r = read(fd, buf, sizeof(buf));
        if (r < 0) {
            std::cerr << "Error: read failed.\n";
            return false;
        }
        if (r == 0) break;
        if (total + (size_t)r > MAX_IMAGE_BYTES) {
            std::cerr << "Error: file too large.\n";
            return false;
        }
        out.insert(out.end(), buf, buf + r);
        total += (size_t)r;
    }
    if (out.empty()) {
        std::cerr << "Error: empty file.\n";
        return false;
    }
    if (!detect_mime(out, mime)) {
        std::cerr << "Error: unsupported image format.\n";
        return false;
    }
    if (!allowed_extension(rel_path, mime)) {
        std::cerr << "Error: extension mismatch.\n";
        return false;
    }
    return true;
}

bool store_image_to_mysql(const std::string& host, unsigned port, const std::string& database,
                          const std::string& user, const std::string& password,
                          const std::string& table_name,
                          const std::string& base_dir, const std::string& relative_path,
                          const std::string& name_in_db,
                          const char* ssl_ca_path /* nullable */) {
    if (!safe_table_name(table_name) || host.empty() || database.empty() || user.empty() ||
        base_dir.empty() || relative_path.empty() || name_in_db.empty()) {
        std::cerr << "Error: invalid parameters.\n";
        return false;
    }

    std::vector<unsigned char> data;
    std::string mime;
    if (!read_image_secure(base_dir, relative_path, data, mime)) {
        return false;
    }

    MYSQL *conn = mysql_init(nullptr);
    if (!conn) {
        std::cerr << "Error: mysql init failed.\n";
        return false;
    }

    unsigned int timeout = 10;
    mysql_options(conn, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
    // Enforce TLS with hostname verification
    enum mysql_ssl_mode mode = SSL_MODE_VERIFY_IDENTITY;
    mysql_options(conn, MYSQL_OPT_SSL_MODE, &mode);
    if (ssl_ca_path && *ssl_ca_path) {
        mysql_options(conn, MYSQL_OPT_SSL_CA, ssl_ca_path);
    }

    if (!mysql_real_connect(conn, host.c_str(), user.c_str(), password.c_str(),
                            database.c_str(), port, nullptr, 0)) {
        std::cerr << "Error: database connection failed.\n";
        mysql_close(conn);
        return false;
    }

    // Create table if not exists
    {
        std::ostringstream oss;
        oss << "CREATE TABLE IF NOT EXISTS `" << table_name << "` ("
            << "id BIGINT AUTO_INCREMENT PRIMARY KEY,"
            << "name VARCHAR(255) NOT NULL,"
            << "mime VARCHAR(64) NOT NULL,"
            << "data LONGBLOB NOT NULL,"
            << "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
            << ") CHARACTER SET utf8mb4 COLLATE utf8mb4_0900_ai_ci";
        if (mysql_query(conn, oss.str().c_str()) != 0) {
            std::cerr << "Error: table creation failed.\n";
            mysql_close(conn);
            return false;
        }
    }

    // Prepare insert
    std::ostringstream ins;
    ins << "INSERT INTO `" << table_name << "` (name, mime, data) VALUES (?, ?, ?)";
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    if (!stmt) {
        std::cerr << "Error: stmt init failed.\n";
        mysql_close(conn);
        return false;
    }
    if (mysql_stmt_prepare(stmt, ins.str().c_str(), (unsigned long)ins.str().size()) != 0) {
        std::cerr << "Error: stmt prepare failed.\n";
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }

    MYSQL_BIND bind[3];
    memset(bind, 0, sizeof(bind));

    // name
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (void*)name_in_db.c_str();
    bind[0].buffer_length = (unsigned long)name_in_db.size();
    unsigned long name_len = (unsigned long)name_in_db.size();
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
    unsigned long blob_len = (unsigned long)data.size();
    bind[2].length = &blob_len;

    if (mysql_stmt_bind_param(stmt, bind) != 0) {
        std::cerr << "Error: bind failed.\n";
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }

    if (mysql_stmt_execute(stmt) != 0) {
        std::cerr << "Error: execute failed.\n";
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }

    mysql_stmt_close(stmt);
    mysql_close(conn);
    return true;
}

static void write_file(const std::string& path, const std::vector<unsigned char>& data) {
    // create parent dirs
    size_t pos = path.find_last_of('/');
    if (pos != std::string::npos) {
        std::string dir = path.substr(0, pos);
        // best-effort mkdir -p
        std::string cur;
        std::stringstream ss(dir);
        std::string item;
        while (std::getline(ss, item, '/')) {
            if (item.empty()) continue;
            cur += "/" + item;
            mkdir(cur.c_str(), 0700);
        }
    }
    int fd = ::open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0600);
    if (fd >= 0) {
        ssize_t written = 0;
        const unsigned char* p = data.data();
        size_t remaining = data.size();
        while (remaining > 0) {
            ssize_t w = ::write(fd, p + written, remaining);
            if (w <= 0) break;
            written += w;
            remaining -= (size_t)w;
        }
        ::close(fd);
    }
}

static std::vector<unsigned char> minimal_png() {
    const unsigned char arr[] = {
        0x89,'P','N','G',0x0D,0x0A,0x1A,0x0A,
        0x00,0x00,0x00,0x0D,'I','H','D','R',
        0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x01,
        0x08,0x06,0x00,0x00,0x00,0x1F,0x15,0xC4,0x89,
        0x00,0x00,0x00,0x0A,'I','D','A','T',
        0x78,0x9C,0x63,0x00,0x01,0x00,0x00,0x05,0x00,0x01,
        0x0D,0x0A,0x2D,0xB4,0x00,0x00,0x00,0x00,'I','E','N','D',
        0xAE,0x42,0x60,0x82
    };
    return std::vector<unsigned char>(arr, arr + sizeof(arr));
}

static std::vector<unsigned char> minimal_jpeg() {
    const unsigned char arr[] = {
        0xFF,0xD8,0xFF,0xE0,0x00,0x10,'J','F','I','F',0x00,0x01,0x01,0x00,0x00,0x01,0x00,0x01,0x00,0x00,
        0xFF,0xD9
    };
    return std::vector<unsigned char>(arr, arr + sizeof(arr));
}

int main() {
    std::string host = std::getenv("DB_HOST") ? std::getenv("DB_HOST") : "localhost";
    unsigned port = std::getenv("DB_PORT") ? (unsigned)std::stoi(std::getenv("DB_PORT")) : 3306u;
    std::string db = std::getenv("DB_NAME") ? std::getenv("DB_NAME") : "test";
    std::string user = std::getenv("DB_USER") ? std::getenv("DB_USER") : "testuser";
    std::string pass = std::getenv("DB_PASS") ? std::getenv("DB_PASS") : "testpass";
    const char* ssl_ca = std::getenv("MYSQL_SSL_CA"); // optional
    std::string table = "images_secure";

    // Prepare temp dir
    char templ[] = "/tmp/safe_images_cppXXXXXX";
    char* d = mkdtemp(templ);
    if (!d) {
        std::cerr << "Failed to create temp directory.\n";
        return 1;
    }
    std::string base = d;

    // Create test files
    std::string png = base + "/pic1.png";
    std::string jpg = base + "/nested/dir/pic2.jpg";
    std::string bad = base + "/note.txt";
    std::string big = base + "/big.webp";
    write_file(png, minimal_png());
    write_file(jpg, minimal_jpeg());
    write_file(bad, std::vector<unsigned char>{'h','e','l','l','o'});
    {
        std::vector<unsigned char> bigc(10 * 1024 * 1024, 'A');
        bigc.push_back(0);
        write_file(big, bigc);
    }

    std::cout << "Test1 (valid PNG): " << (store_image_to_mysql(host, port, db, user, pass, table, base, "pic1.png", "png_test", ssl_ca) ? "true" : "false") << "\n";
    std::cout << "Test2 (valid JPEG): " << (store_image_to_mysql(host, port, db, user, pass, table, base, "nested/dir/pic2.jpg", "jpeg_test", ssl_ca) ? "true" : "false") << "\n";
    std::cout << "Test3 (bad extension): " << (store_image_to_mysql(host, port, db, user, pass, table, base, "note.txt", "badext_test", ssl_ca) ? "true" : "false") << "\n";
    std::cout << "Test4 (path traversal attempt): " << (store_image_to_mysql(host, port, db, user, pass, table, base, "../etc/passwd", "traversal", ssl_ca) ? "true" : "false") << "\n";
    std::cout << "Test5 (too large): " << (store_image_to_mysql(host, port, db, user, pass, table, base, "big.webp", "big_test", ssl_ca) ? "true" : "false") << "\n";
    return 0;
}