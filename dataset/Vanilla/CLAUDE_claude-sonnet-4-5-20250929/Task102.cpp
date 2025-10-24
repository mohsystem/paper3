
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <mysql/mysql.h>
#include <openssl/sha.h>

using namespace std;

const char* DB_HOST = "localhost";
const char* DB_USER = "root";
const char* DB_PASSWORD = "password";
const char* DB_NAME = "userdb";

string hashPassword(const string& password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password.c_str(), password.length(), hash);
    
    stringstream ss;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    return ss.str();
}

bool updatePassword(const string& username, const string& newPassword) {
    string hashedPassword = hashPassword(newPassword);
    
    MYSQL* conn = mysql_init(NULL);
    if (conn == NULL) {
        cerr << "mysql_init() failed" << endl;
        return false;
    }
    
    if (mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASSWORD, DB_NAME, 0, NULL, 0) == NULL) {
        cerr << "mysql_real_connect() failed: " << mysql_error(conn) << endl;
        mysql_close(conn);
        return false;
    }
    
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    string query = "UPDATE users SET password = ? WHERE username = ?";
    
    if (mysql_stmt_prepare(stmt, query.c_str(), query.length())) {
        cerr << "mysql_stmt_prepare() failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }
    
    MYSQL_BIND bind[2];
    memset(bind, 0, sizeof(bind));
    
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char*)hashedPassword.c_str();
    bind[0].buffer_length = hashedPassword.length();
    
    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = (char*)username.c_str();
    bind[1].buffer_length = username.length();
    
    if (mysql_stmt_bind_param(stmt, bind)) {
        cerr << "mysql_stmt_bind_param() failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }
    
    if (mysql_stmt_execute(stmt)) {
        cerr << "mysql_stmt_execute() failed: " << mysql_stmt_error(stmt) << endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }
    
    int rowsAffected = mysql_stmt_affected_rows(stmt);
    
    mysql_stmt_close(stmt);
    mysql_close(conn);
    
    return rowsAffected > 0;
}

int main() {
    // Test cases
    cout << "Test Case 1:" << endl;
    bool result1 = updatePassword("john_doe", "newPassword123");
    cout << "Update result for john_doe: " << (result1 ? "true" : "false") << endl;
    
    cout << "\\nTest Case 2:" << endl;
    bool result2 = updatePassword("jane_smith", "SecurePass456!");
    cout << "Update result for jane_smith: " << (result2 ? "true" : "false") << endl;
    
    cout << "\\nTest Case 3:" << endl;
    bool result3 = updatePassword("admin", "Admin@2024");
    cout << "Update result for admin: " << (result3 ? "true" : "false") << endl;
    
    cout << "\\nTest Case 4:" << endl;
    bool result4 = updatePassword("test_user", "TestPass789#");
    cout << "Update result for test_user: " << (result4 ? "true" : "false") << endl;
    
    cout << "\\nTest Case 5:" << endl;
    bool result5 = updatePassword("guest", "GuestPassword2024");
    cout << "Update result for guest: " << (result5 ? "true" : "false") << endl;
    
    return 0;
}
