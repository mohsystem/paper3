
#include <iostream>
#include <string>
#include <regex>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <sqlite3.h>
#include <memory>

// Constants for validation
const size_t MAX_STOCK_NAME_LENGTH = 50;
const int MAX_QUANTITY = 1000000;
const int MIN_QUANTITY = 1;

// Secure string handling - bounds checked copy
void secure_string_copy(char* dest, size_t dest_size, const char* src) {
    if (dest == nullptr || src == nullptr || dest_size == 0) {
        return;
    }
    
    // Use strncpy with explicit null termination
    strncpy(dest, src, dest_size - 1);
    dest[dest_size - 1] = '\\0'; // Ensure null termination
}

// Get secure database connection
sqlite3* get_secure_connection() {
    sqlite3* db = nullptr;
    
    // Get database path from environment variable - never hardcode credentials
    const char* db_path = std::getenv("DB_PATH");
    if (db_path == nullptr) {
        db_path = "stocks.db"; // Safe default
    }
    
    // Validate path to prevent path traversal
    std::string path_str(db_path);
    if (path_str.find("..") != std::string::npos || path_str[0] == '/') {
        std::cerr << "Invalid database path" << std::endl;
        return nullptr;
    }
    
    // Open database with error checking
    int rc = sqlite3_open(db_path, &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database" << std::endl;
        if (db != nullptr) {
            sqlite3_close(db);
        }
        return nullptr;
    }
    
    // Enable foreign keys for data integrity
    sqlite3_exec(db, "PRAGMA foreign_keys = ON", nullptr, nullptr, nullptr);
    
    return db;
}

// Validates stock name - prevents injection attacks
bool is_valid_stock_name(const std::string& stock_name) {
    // Check for empty string
    if (stock_name.empty()) {
        return false;
    }
    
    // Check length bounds to prevent buffer overflow
    if (stock_name.length() > MAX_STOCK_NAME_LENGTH) {
        return false;
    }
    
    // Allow only alphanumeric, spaces, hyphens, underscores
    // Prevents SQL injection, command injection
    std::regex valid_pattern("^[a-zA-Z0-9\\\\s_-]+$");
    if (!std::regex_match(stock_name, valid_pattern)) {
        return false;
    }
    
    return true;
}

// Validates quantity - prevents integer overflow
bool is_valid_quantity(int quantity) {
    // Check bounds
    return quantity >= MIN_QUANTITY && quantity <= MAX_QUANTITY;
}

// Trim whitespace from string
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \\t\\n\\r");
    if (first == std::string::npos) {
        return "";
    }
    size_t last = str.find_last_not_of(" \\t\\n\\r");
    return str.substr(first, (last - first + 1));
}

// Buy function - processes stock name
std::string buy_function(const std::string& stock_name) {
    // Input validation - treat all input as untrusted
    if (!is_valid_stock_name(stock_name)) {
        return "Invalid stock name";
    }
    
    // Sanitize input
    std::string sanitized = trim(stock_name);
    
    // Business logic
    return "Processing purchase for stock: " + sanitized;
}

// Main stock purchase function
std::string buy_stock(const std::string& stock_name, int quantity) {
    sqlite3* db = nullptr;
    sqlite3_stmt* stmt = nullptr;
    
    try {
        // Input validation - fail early on invalid input
        if (!is_valid_stock_name(stock_name)) {
            return "Error: Invalid stock name format";
        }
        
        if (!is_valid_quantity(quantity)) {
            return "Error: Invalid quantity. Must be between " + 
                   std::to_string(MIN_QUANTITY) + " and " + 
                   std::to_string(MAX_QUANTITY);
        }
        
        // Sanitize inputs
        std::string sanitized_name = trim(stock_name);
        
        // Call buy_function
        std::string buy_result = buy_function(sanitized_name);
        
        // Get secure database connection
        db = get_secure_connection();
        if (db == nullptr) {
            return "Error: Unable to establish database connection";
        }
        
        // Use parameterized query to prevent SQL injection
        const char* sql = "INSERT INTO stock_purchases (stock_name, quantity, purchase_date) VALUES (?, ?, ?)";
        
        // Prepare statement - checks return value
        int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "Failed to prepare statement" << std::endl;
            sqlite3_close(db);
            return "Error: Unable to complete purchase";
        }
        
        // Bind parameters - type-safe binding prevents injection
        sqlite3_bind_text(stmt, 1, sanitized_name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, quantity);
        
        // Get current timestamp
        time_t now = time(nullptr);
        char timestamp[64] = {0}; // Initialize to zero
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
        sqlite3_bind_text(stmt, 3, timestamp, -1, SQLITE_TRANSIENT);
        
        // Execute statement
        rc = sqlite3_step(stmt);
        
        // Clean up statement
        sqlite3_finalize(stmt);
        stmt = nullptr;
        
        // Close database
        sqlite3_close(db);
        db = nullptr;
        
        if (rc == SQLITE_DONE) {
            return "Stock purchase successful: " + buy_result;
        } else {
            return "Stock purchase failed";
        }
        
    } catch (...) {
        // Clean up on exception
        if (stmt != nullptr) {
            sqlite3_finalize(stmt);
        }
        if (db != nullptr) {
            sqlite3_close(db);
        }
        return "Error: Unable to complete purchase";
    }
}

// Test cases
int main() {
    std::cout << "Test Case 1 - Valid stock purchase:" << std::endl;
    std::cout << buy_stock("AAPL", 10) << std::endl;
    
    std::cout << "\\nTest Case 2 - Valid stock with hyphen:" << std::endl;
    std::cout << buy_stock("TECH-CORP", 50) << std::endl;
    
    std::cout << "\\nTest Case 3 - Invalid stock name (SQL injection attempt):" << std::endl;
    std::cout << buy_stock("AAPL'; DROP TABLE stock_purchases; --", 10) << std::endl;
    
    std::cout << "\\nTest Case 4 - Invalid quantity (negative):" << std::endl;
    std::cout << buy_stock("GOOGL", -5) << std::endl;
    
    std::cout << "\\nTest Case 5 - Invalid quantity (too large):" << std::endl;
    std::cout << buy_stock("MSFT", 2000000) << std::endl;
    
    return 0;
}
