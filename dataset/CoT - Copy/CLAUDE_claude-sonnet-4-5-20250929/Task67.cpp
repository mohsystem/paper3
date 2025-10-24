
#include <iostream>
#include <string>
#include <regex>
#include <memory>
#include <mysql/mysql.h>

class Customer {
public:
    int id;
    std::string username;
    std::string email;
    std::string fullName;
    
    Customer(int id, const std::string& username, const std::string& email, const std::string& fullName)
        : id(id), username(username), email(email), fullName(fullName) {}
    
    friend std::ostream& operator<<(std::ostream& os, const Customer& c) {
        os << "Customer{id=" << c.id << ", username='" << c.username 
           << "', email='" << c.email << "', fullName='" << c.fullName << "'}";
        return os;
    }
};

class DatabaseConnection {
private:
    MYSQL* conn;
    const char* DB_HOST = "localhost";
    const char* DB_USER = "root";
    const char* DB_PASSWORD = "password";
    const char* DB_NAME = "customerdb";
    
public:
    DatabaseConnection() : conn(nullptr) {
        conn = mysql_init(nullptr);
        if (!conn) {
            std::cerr << "MySQL initialization failed" << std::endl;
        }
    }
    
    ~DatabaseConnection() {
        if (conn) {
            mysql_close(conn);
        }
    }
    
    bool connect() {
        if (!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASSWORD, DB_NAME, 0, nullptr, 0)) {
            std::cerr << "Database connection failed" << std::endl;
            return false;
        }
        return true;
    }
    
    MYSQL* getConnection() {
        return conn;
    }
};

bool validateUsername(const std::string& username) {
    // Input validation
    if (username.empty() || username.length() > 50) {
        std::cerr << "Error: Username is empty or too long" << std::endl;
        return false;
    }
    
    // Sanitize input (allow only alphanumeric and underscore)
    std::regex pattern("^[a-zA-Z0-9_]{1,50}$");
    if (!std::regex_match(username, pattern)) {
        std::cerr << "Error: Invalid username format" << std::endl;
        return false;
    }
    
    return true;
}

std::unique_ptr<Customer> getCustomerByUsername(const std::string& customerUsername) {
    // Input validation
    if (!validateUsername(customerUsername)) {
        return nullptr;
    }
    
    DatabaseConnection dbConn;
    if (!dbConn.connect()) {
        return nullptr;
    }
    
    MYSQL* conn = dbConn.getConnection();
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    
    if (!stmt) {
        std::cerr << "Statement initialization failed" << std::endl;
        return nullptr;
    }
    
    // Use prepared statement to prevent SQL injection
    const char* query = "SELECT id, username, email, full_name FROM customer WHERE username = ?";
    
    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        std::cerr << "Statement preparation failed" << std::endl;
        mysql_stmt_close(stmt);
        return nullptr;
    }
    
    // Bind parameters
    MYSQL_BIND bind[1];
    memset(bind, 0, sizeof(bind));
    
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char*)customerUsername.c_str();
    bind[0].buffer_length = customerUsername.length();
    
    if (mysql_stmt_bind_param(stmt, bind)) {
        std::cerr << "Parameter binding failed" << std::endl;
        mysql_stmt_close(stmt);
        return nullptr;
    }
    
    // Execute query
    if (mysql_stmt_execute(stmt)) {
        std::cerr << "Query execution failed" << std::endl;
        mysql_stmt_close(stmt);
        return nullptr;
    }
    
    // Bind results
    MYSQL_BIND result[4];
    int id;
    char username[51], email[101], fullName[101];
    unsigned long length[4];
    my_bool is_null[4];
    
    memset(result, 0, sizeof(result));
    
    result[0].buffer_type = MYSQL_TYPE_LONG;
    result[0].buffer = &id;
    result[0].is_null = &is_null[0];
    result[0].length = &length[0];
    
    result[1].buffer_type = MYSQL_TYPE_STRING;
    result[1].buffer = username;
    result[1].buffer_length = sizeof(username);
    result[1].is_null = &is_null[1];
    result[1].length = &length[1];
    
    result[2].buffer_type = MYSQL_TYPE_STRING;
    result[2].buffer = email;
    result[2].buffer_length = sizeof(email);
    result[2].is_null = &is_null[2];
    result[2].length = &length[2];
    
    result[3].buffer_type = MYSQL_TYPE_STRING;
    result[3].buffer = fullName;
    result[3].buffer_length = sizeof(fullName);
    result[3].is_null = &is_null[3];
    result[3].length = &length[3];
    
    if (mysql_stmt_bind_result(stmt, result)) {
        std::cerr << "Result binding failed" << std::endl;
        mysql_stmt_close(stmt);
        return nullptr;
    }
    
    std::unique_ptr<Customer> customer = nullptr;
    
    if (mysql_stmt_fetch(stmt) == 0) {
        customer = std::make_unique<Customer>(id, std::string(username), std::string(email), std::string(fullName));
    } else {
        std::cout << "No customer found with username: " << customerUsername << std::endl;
    }
    
    mysql_stmt_close(stmt);
    return customer;
}

int main(int argc, char* argv[]) {
    // Test cases
    std::string testUsernames[] = {"john_doe", "jane_smith", "admin123", "test_user", "nonexistent"};
    
    if (argc > 1) {
        // Use command line argument
        auto customer = getCustomerByUsername(argv[1]);
        if (customer) {
            std::cout << "Retrieved: " << *customer << std::endl;
        }
    } else {
        // Run test cases
        std::cout << "Running test cases...\\n" << std::endl;
        for (const auto& username : testUsernames) {
            std::cout << "Testing username: " << username << std::endl;
            auto customer = getCustomerByUsername(username);
            if (customer) {
                std::cout << "Result: " << *customer << std::endl;
            }
            std::cout << "---" << std::endl;
        }
    }
    
    return 0;
}
