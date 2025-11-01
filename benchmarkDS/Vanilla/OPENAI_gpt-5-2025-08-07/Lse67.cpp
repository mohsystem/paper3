#include <mysql/mysql.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cctype>

static bool valid_identifier(const std::string& s) {
    if (s.empty()) return false;
    for (char c : s) {
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_')) return false;
    }
    return true;
}

static bool exec_query(MYSQL* conn, const std::string& q) {
    if (mysql_query(conn, q.c_str()) != 0) {
        std::cerr << "MySQL query error: " << mysql_error(conn) << std::endl;
        return false;
    }
    return true;
}

bool storeImage(const std::string& filePath,
                const std::string& host,
                const std::string& user,
                const std::string& pass,
                const std::string& db,
                unsigned int port,
                const std::string& table,
                int id)
{
    if (!valid_identifier(table)) {
        std::cerr << "Invalid table name" << std::endl;
        return false;
    }

    std::ifstream in(filePath, std::ios::binary);
    if (!in) {
        std::cerr << "File not found: " << filePath << std::endl;
        return false;
    }
    std::vector<char> data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    std::string filename;
    {
        size_t pos = filePath.find_last_of("/\\");
        filename = (pos == std::string::npos) ? filePath : filePath.substr(pos + 1);
    }

    MYSQL* conn = mysql_init(nullptr);
    if (!conn) {
        std::cerr << "mysql_init failed" << std::endl;
        return false;
    }

    if (!mysql_real_connect(conn, host.c_str(), user.c_str(), pass.c_str(), db.c_str(), port, nullptr, 0)) {
        std::cerr << "MySQL connect error: " << mysql_error(conn) << std::endl;
        mysql_close(conn);
        return false;
    }

    std::string createSql = "CREATE TABLE IF NOT EXISTS " + table + " ("
                            "id INT PRIMARY KEY, "
                            "filename VARCHAR(255), "
                            "data LONGBLOB"
                            ")";
    if (!exec_query(conn, createSql)) {
        mysql_close(conn);
        return false;
    }

    std::string insertSql = "INSERT INTO " + table + " (id, filename, data) VALUES (?, ?, ?) "
                            "ON DUPLICATE KEY UPDATE filename=VALUES(filename), data=VALUES(data)";
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    if (!stmt) {
        std::cerr << "mysql_stmt_init failed" << std::endl;
        mysql_close(conn);
        return false;
    }
    if (mysql_stmt_prepare(stmt, insertSql.c_str(), static_cast<unsigned long>(insertSql.size())) != 0) {
        std::cerr << "mysql_stmt_prepare error: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }

    MYSQL_BIND bind[3] = {};
    memset(bind, 0, sizeof(bind));

    // id
    bind[0].buffer_type = MYSQL_TYPE_LONG;
    bind[0].buffer = (char*)&id;

    // filename
    unsigned long filename_len = static_cast<unsigned long>(filename.size());
    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = (char*)filename.data();
    bind[1].buffer_length = filename_len;
    bind[1].length = &filename_len;

    // data
    unsigned long data_len = static_cast<unsigned long>(data.size());
    bind[2].buffer_type = MYSQL_TYPE_LONG_BLOB;
    bind[2].buffer = (char*)data.data();
    bind[2].buffer_length = data_len;
    bind[2].length = &data_len;

    if (mysql_stmt_bind_param(stmt, bind) != 0) {
        std::cerr << "mysql_stmt_bind_param error: " << mysql_stmt_error(stmt) << std::endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }

    bool ok = true;
    if (mysql_stmt_execute(stmt) != 0) {
        std::cerr << "mysql_stmt_execute error: " << mysql_stmt_error(stmt) << std::endl;
        ok = false;
    }

    mysql_stmt_close(stmt);
    mysql_close(conn);
    return ok;
}

int main() {
    std::string host = "localhost";
    std::string user = "root";
    std::string pass = "root";
    std::string db   = "testdb";
    unsigned int port = 3306;
    std::string table = "images";

    std::cout << "Test 1: " << (storeImage("test1.jpg", host, user, pass, db, port, table, 1) ? "ok" : "fail") << std::endl;
    std::cout << "Test 2: " << (storeImage("test2.png", host, user, pass, db, port, table, 2) ? "ok" : "fail") << std::endl;
    std::cout << "Test 3: " << (storeImage("test3.gif", host, user, pass, db, port, table, 3) ? "ok" : "fail") << std::endl;
    std::cout << "Test 4: " << (storeImage("test4.bmp", host, user, pass, db, port, table, 4) ? "ok" : "fail") << std::endl;
    std::cout << "Test 5: " << (storeImage("test5.webp", host, user, pass, db, port, table, 5) ? "ok" : "fail") << std::endl;

    return 0;
}