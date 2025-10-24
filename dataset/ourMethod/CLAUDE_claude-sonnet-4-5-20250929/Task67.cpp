
#include <iostream>
#include <string>
#include <cstring>
#include <memory>
#include <sqlite3.h>
#include <vector>

// Customer structure to hold retrieved data
struct Customer {
    int id;
    std::string username;
    std::string email;
    std::string fullname;
    
    Customer() : id(0) {}
};

// Callback function for SQLite query execution
// CWE-200 Prevention: Only returns expected columns, no sensitive data exposure
static int callback(void* data, int argc, char** argv, char** colNames) {
    Customer* customer = static_cast<Customer*>(data);
    
    // Validate callback parameters (CWE-20: Input Validation)
    if (!customer || argc == 0 || !argv || !colNames) {
        return SQLITE_ERROR;
    }
    
    // Securely parse each column with bounds checking
    for (int i = 0; i < argc; i++) {
        if (colNames[i] && argv[i]) {
            std::string colName(colNames[i]);
            
            if (colName == "id") {
                try {
                    customer->id = std::stoi(argv[i]);
                } catch (const std::exception&) {
                    // Invalid integer format - set to 0
                    customer->id = 0;
                }
            } else if (colName == "username") {
                // CWE-116: Validate username length to prevent buffer issues
                if (std::strlen(argv[i]) <= 255) {
                    customer->username = argv[i];
                }
            } else if (colName == "email") {
                if (std::strlen(argv[i]) <= 255) {
                    customer->email = argv[i];
                }
            } else if (colName == "fullname") {
                if (std::strlen(argv[i]) <= 255) {
                    customer->fullname = argv[i];
                }
            }
        }
    }
    
    return SQLITE_OK;
}

// Function to retrieve customer by username
// CWE-89 Prevention: Uses parameterized queries to prevent SQL injection
bool getCustomerByUsername(const std::string& dbPath, const std::string& username, Customer& customer) {
    sqlite3* db = nullptr;
    sqlite3_stmt* stmt = nullptr;
    bool success = false;
    
    // CWE-20: Validate username input (length and allowed characters)
    if (username.empty() || username.length() > 255) {
        std::cerr << "Error: Invalid username length" << std::endl;
        return false;
    }
    
    // Basic alphanumeric validation (adjust regex pattern as needed)
    for (char c : username) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_' && c != '-' && c != '.') {
            std::cerr << "Error: Username contains invalid characters" << std::endl;
            return false;
        }
    }
    
    // Open database connection with proper error handling
    int rc = sqlite3_open(dbPath.c_str(), &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Error: Cannot open database" << std::endl;
        if (db) {
            sqlite3_close(db);
        }
        return false;
    }
    
    // CWE-89 Prevention: Use parameterized query to prevent SQL injection
    const char* sql = "SELECT id, username, email, fullname FROM customer WHERE username = ? LIMIT 1;";
    
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Error: Failed to prepare statement" << std::endl;
        sqlite3_close(db);
        return false;
    }
    
    // Bind parameter safely (CWE-89 Prevention)
    rc = sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        std::cerr << "Error: Failed to bind parameter" << std::endl;
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return false;
    }
    
    // Execute query and retrieve results
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        // Safely extract data with type checking
        customer.id = sqlite3_column_int(stmt, 0);
        
        const unsigned char* usernameText = sqlite3_column_text(stmt, 1);
        if (usernameText) {
            customer.username = reinterpret_cast<const char*>(usernameText);
        }
        
        const unsigned char* emailText = sqlite3_column_text(stmt, 2);
        if (emailText) {
            customer.email = reinterpret_cast<const char*>(emailText);
        }
        
        const unsigned char* fullnameText = sqlite3_column_text(stmt, 3);
        if (fullnameText) {
            customer.fullname = reinterpret_cast<const char*>(fullnameText);
        }
        
        success = true;
    } else if (rc == SQLITE_DONE) {
        std::cerr << "Error: No customer found with username: " << username << std::endl;
    } else {
        std::cerr << "Error: Query execution failed" << std::endl;
    }
    
    // Clean up resources (RAII-like cleanup)
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return success;
}

int main(int argc, char* argv[]) {
    // CWE-20: Validate command line arguments
    if (argc != 2) {
        std::cerr << "Usage: " << (argc > 0 ? argv[0] : "program") << " <customer_username>" << std::endl;
        return 1;
    }
    
    // CWE-20: Validate argument is not null and not empty
    if (!argv[1] || std::strlen(argv[1]) == 0) {
        std::cerr << "Error: Username cannot be empty" << std::endl;
        return 1;
    }
    
    std::string username(argv[1]);
    std::string dbPath = "customer.db"; // Use configuration or environment variable in production
    
    Customer customer;
    
    // Test cases for demonstration
    std::vector<std::string> testUsernames = {"john_doe", "jane_smith", "test_user", "admin", "user123"};
    
    std::cout << "=== Running Test Cases ===" << std::endl;
    for (const auto& testUser : testUsernames) {
        Customer testCustomer;
        std::cout << "\\nTest: Retrieving customer '" << testUser << "'" << std::endl;
        
        if (getCustomerByUsername(dbPath, testUser, testCustomer)) {
            std::cout << "Success! Customer found:" << std::endl;
            std::cout << "  ID: " << testCustomer.id << std::endl;
            std::cout << "  Username: " << testCustomer.username << std::endl;
            std::cout << "  Email: " << testCustomer.email << std::endl;
            std::cout << "  Full Name: " << testCustomer.fullname << std::endl;
        } else {
            std::cout << "Customer not found or error occurred" << std::endl;
        }
    }
    
    // Retrieve customer from command line argument
    std::cout << "\\n=== Retrieving Customer from Command Line ===" << std::endl;
    if (getCustomerByUsername(dbPath, username, customer)) {
        std::cout << "Customer Information:" << std::endl;
        std::cout << "  ID: " << customer.id << std::endl;
        std::cout << "  Username: " << customer.username << std::endl;
        std::cout << "  Email: " << customer.email << std::endl;
        std::cout << "  Full Name: " << customer.fullname << std::endl;
        return 0;
    } else {
        std::cerr << "Failed to retrieve customer information" << std::endl;
        return 1;
    }
}
