#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <sqlite3.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

static const char* DB_PATH = "task46_cpp.db";
static const int ITERATIONS = 120000;
static const int SALT_LENGTH = 16;
static const int KEY_LENGTH = 32; // bytes

bool init_db(const std::string& db_path) {
    sqlite3* db = nullptr;
    if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return false;
    }
    const char* sql =
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "email TEXT NOT NULL UNIQUE,"
        "password_hash BLOB NOT NULL,"
        "salt BLOB NOT NULL,"
        "iterations INTEGER NOT NULL"
        ")";
    char* errmsg = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errmsg);
    if (rc != SQLITE_OK) {
        if (errmsg) sqlite3_free(errmsg);
        sqlite3_close(db);
        return false;
    }
    sqlite3_close(db);
    return true;
}

bool valid_name(const std::string& name) {
    std::string n = name;
    // trim
    n.erase(0, n.find_first_not_of(" \t\r\n"));
    n.erase(n.find_last_not_of(" \t\r\n") + 1);
    return !n.empty() && n.size() <= 100;
}

bool valid_email(const std::string& email) {
    std::string e = email;
    // trim
    e.erase(0, e.find_first_not_of(" \t\r\n"));
    e.erase(e.find_last_not_of(" \t\r\n") + 1);
    std::regex re("^[A-Za-z0-9._%+-]{1,64}@[A-Za-z0-9.-]{1,253}\\.[A-Za-z]{2,63}$");
    return std::regex_match(e, re);
}

bool valid_password(const std::string& password) {
    if (password.size() < 8) return false;
    bool hasUpper=false, hasLower=false, hasDigit=false, hasSpecial=false;
    std::string specials = "!@#$%^&*()-_=+[]{}|;:'\",.<>?/`~\\";
    for (unsigned char c : password) {
        if (std::isupper(c)) hasUpper = true;
        else if (std::islower(c)) hasLower = true;
        else if (std::isdigit(c)) hasDigit = true;
        else if (specials.find(c) != std::string::npos) hasSpecial = true;
    }
    return hasUpper && hasLower && hasDigit && hasSpecial;
}

bool pbkdf2_sha256(const std::vector<unsigned char>& password,
                   const std::vector<unsigned char>& salt,
                   int iterations, int keyLen,
                   std::vector<unsigned char>& out_key) {
    out_key.assign(keyLen, 0);
    const EVP_MD* md = EVP_sha256();
    if (!PKCS5_PBKDF2_HMAC(reinterpret_cast<const char*>(password.data()),
                           static_cast<int>(password.size()),
                           salt.data(),
                           static_cast<int>(salt.size()),
                           iterations, md, keyLen, out_key.data())) {
        return false;
    }
    return true;
}

bool register_user(const std::string& db_path, const std::string& name,
                   const std::string& email, const std::string& password) {
    if (!valid_name(name) || !valid_email(email) || !valid_password(password)) {
        return false;
    }

    sqlite3* db = nullptr;
    if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return false;
    }

    std::vector<unsigned char> salt(SALT_LENGTH);
    if (RAND_bytes(salt.data(), SALT_LENGTH) != 1) {
        sqlite3_close(db);
        return false;
    }

    std::vector<unsigned char> pwd(password.begin(), password.end());
    std::vector<unsigned char> hash;
    bool ok = pbkdf2_sha256(pwd, salt, ITERATIONS, KEY_LENGTH, hash);
    // Wipe password buffer
    if (!pwd.empty()) OPENSSL_cleanse(pwd.data(), pwd.size());
    if (!ok) {
        sqlite3_close(db);
        return false;
    }

    const char* sql = "INSERT INTO users(name, email, password_hash, salt, iterations) VALUES(?,?,?,?,?)";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        return false;
    }

    std::string name_trim = name;
    name_trim.erase(0, name_trim.find_first_not_of(" \t\r\n"));
    name_trim.erase(name_trim.find_last_not_of(" \t\r\n") + 1);
    std::string email_trim = email;
    email_trim.erase(0, email_trim.find_first_not_of(" \t\r\n"));
    email_trim.erase(email_trim.find_last_not_of(" \t\r\n") + 1);
    for (auto& c : email_trim) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

    bool result = false;
    if (sqlite3_bind_text(stmt, 1, name_trim.c_str(), -1, SQLITE_TRANSIENT) == SQLITE_OK &&
        sqlite3_bind_text(stmt, 2, email_trim.c_str(), -1, SQLITE_TRANSIENT) == SQLITE_OK &&
        sqlite3_bind_blob(stmt, 3, hash.data(), static_cast<int>(hash.size()), SQLITE_TRANSIENT) == SQLITE_OK &&
        sqlite3_bind_blob(stmt, 4, salt.data(), static_cast<int>(salt.size()), SQLITE_TRANSIENT) == SQLITE_OK &&
        sqlite3_bind_int(stmt, 5, ITERATIONS) == SQLITE_OK) {
        int rc = sqlite3_step(stmt);
        result = (rc == SQLITE_DONE);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    if (!hash.empty()) OPENSSL_cleanse(hash.data(), hash.size());
    if (!salt.empty()) OPENSSL_cleanse(salt.data(), salt.size());

    return result;
}

int main() {
    init_db(DB_PATH);
    std::cout << (register_user(DB_PATH, "Alice Smith", "alice@example.com", "Str0ng!Pass1") ? "1" : "0") << std::endl;
    std::cout << (register_user(DB_PATH, "Bob Johnson", "bob@example.com", "An0ther#Good2") ? "1" : "0") << std::endl;
    std::cout << (register_user(DB_PATH, "Charlie", "bad-email", "Weak") ? "1" : "0") << std::endl; // invalid
    std::cout << (register_user(DB_PATH, "Dana White", "dana.white@example.org", "Secur3$Pass4") ? "1" : "0") << std::endl;
    std::cout << (register_user(DB_PATH, "Eve Adams", "alice@example.com", "Str0ng!Pass1") ? "1" : "0") << std::endl; // duplicate
    return 0;
}