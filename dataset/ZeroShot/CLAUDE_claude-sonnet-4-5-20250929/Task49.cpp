
#include <iostream>
#include <string>
#include <regex>
#include <sqlite3.h>

class Task49 {
private:
    static const std::string DB_NAME;
    
    static std::string sanitizeInput(const std::string& input) {
        std::string result = input;
        result.erase(0, result.find_first_not_of(" \\t\\n\\r"));
        result.erase(result.find_last_not_of(" \\t\\n\\r") + 1);
        
        std::string dangerous = "<>\\"';";
        for (char c : dangerous) {
            result.erase(std::remove(result.begin(), result.end(), c), result.end());
        }
        return result;
    }
    
    static bool validateInput(const std::string& name, const std::string& email, int age) {
        if (name.empty() || name.length() > 100) {
            return false;
        }
        
        std::regex emailPattern("^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$");
        if (!std::regex_match(email, emailPattern)) {
            return false;
        }
        
        if (age < 0 || age > 150) {
            return false;
        }
        
        return true;
    }

public:
    struct ApiResponse {
        bool success;
        std::string message;
        
        ApiResponse(bool s, const std::string& m) : success(s), message(m) {}
    };
    
    static void initDatabase() {
        sqlite3* db;
        char* errMsg = nullptr;
        
        int rc = sqlite3_open(DB_NAME.c_str(), &db);
        if (rc != SQLITE_OK) {
            std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
            return;
        }
        
        const char* sql = "CREATE TABLE IF NOT EXISTS users ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                         "name TEXT NOT NULL,"
                         "email TEXT NOT NULL UNIQUE,"
                         "age INTEGER NOT NULL)";
        
        rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        }
        
        sqlite3_close(db);
    }
    
    static ApiResponse storeUserData(std::string name, std::string email, int age) {
        name = sanitizeInput(name);
        email = sanitizeInput(email);
        
        if (!validateInput(name, email, age)) {
            return ApiResponse(false, "Invalid input data");
        }
        
        sqlite3* db;
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_open(DB_NAME.c_str(), &db);
        if (rc != SQLITE_OK) {
            return ApiResponse(false, "Database connection error");
        }
        
        const char* sql = "INSERT INTO users (name, email, age) VALUES (?, ?, ?)";
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        
        if (rc != SQLITE_OK) {
            sqlite3_close(db);
            return ApiResponse(false, "Statement preparation error");
        }
        
        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, email.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 3, age);
        
        rc = sqlite3_step(stmt);
        
        ApiResponse response(false, "Unknown error");
        if (rc == SQLITE_DONE) {
            response = ApiResponse(true, "User data stored successfully");
        } else if (rc == SQLITE_CONSTRAINT) {
            response = ApiResponse(false, "Email already exists");
        } else {
            response = ApiResponse(false, std::string("Database error: ") + sqlite3_errmsg(db));
        }
        
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        return response;
    }
};

const std::string Task49::DB_NAME = "users.db";

int main() {
    Task49::initDatabase();
    
    std::cout << "=== API Endpoint Test Cases ===" << std::endl << std::endl;
    
    // Test Case 1: Valid user data
    std::cout << "Test 1 - Valid user:" << std::endl;
    auto response1 = Task49::storeUserData("John Doe", "john@example.com", 25);
    std::cout << "Success: " << response1.success << ", Message: " << response1.message << std::endl << std::endl;
    
    // Test Case 2: Another valid user
    std::cout << "Test 2 - Another valid user:" << std::endl;
    auto response2 = Task49::storeUserData("Jane Smith", "jane@example.com", 30);
    std::cout << "Success: " << response2.success << ", Message: " << response2.message << std::endl << std::endl;
    
    // Test Case 3: Duplicate email
    std::cout << "Test 3 - Duplicate email:" << std::endl;
    auto response3 = Task49::storeUserData("John Clone", "john@example.com", 35);
    std::cout << "Success: " << response3.success << ", Message: " << response3.message << std::endl << std::endl;
    
    // Test Case 4: Invalid email format
    std::cout << "Test 4 - Invalid email:" << std::endl;
    auto response4 = Task49::storeUserData("Invalid User", "notanemail", 28);
    std::cout << "Success: " << response4.success << ", Message: " << response4.message << std::endl << std::endl;
    
    // Test Case 5: Invalid age
    std::cout << "Test 5 - Invalid age:" << std::endl;
    auto response5 = Task49::storeUserData("Old Person", "old@example.com", 200);
    std::cout << "Success: " << response5.success << ", Message: " << response5.message << std::endl << std::endl;
    
    return 0;
}
