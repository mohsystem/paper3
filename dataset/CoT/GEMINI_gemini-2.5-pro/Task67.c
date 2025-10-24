#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite3.h"

// Struct to hold customer data. Using fixed-size arrays for simplicity.
// Be aware of potential truncation if data from DB is larger.
struct Customer {
    int id;
    char username[256];
    char fullName[256];
    char email[256];
};

// Sets up the in-memory database: creates table and inserts sample data.
void setup_database(sqlite3* db) {
    char* err_msg = 0;
    const char* sql = 
        "CREATE TABLE customer ("
        "id INT PRIMARY KEY, "
        "username TEXT NOT NULL UNIQUE, "
        "fullname TEXT, "
        "email TEXT);"
        "INSERT INTO customer VALUES (1, 'jdoe', 'John Doe', 'john.doe@example.com');"
        "INSERT INTO customer VALUES (2, 'asmith', 'Alice Smith', 'alice.smith@example.com');"
        "INSERT INTO customer VALUES (3, 'bwhite', 'Bob White', 'bob.white@example.com');"
        "INSERT INTO customer VALUES (4, 'peterpan', 'Peter Pan', 'peter.pan@example.com');";

    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL setup error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
}

/**
 * @brief Retrieves a customer by username using a secure prepared statement.
 * @param db The SQLite database connection handle.
 * @param customerUsername The username to search for.
 * @param customer_out A pointer to a Customer struct to be filled with data.
 * @return 0 on success (found), 1 if not found, -1 on error.
 */
int getCustomerByUsername(sqlite3* db, const char* customerUsername, struct Customer* customer_out) {
    sqlite3_stmt* stmt = NULL;
    // SQL query with a placeholder (?) to prevent SQL injection
    const char* sql = "SELECT id, username, fullname, email FROM customer WHERE username = ?;";
    int result_status = 1; // Default to "not found"

    // Prepare the statement
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return -1; // Error
    }

    // Bind the username parameter to the placeholder. This is the key security step.
    rc = sqlite3_bind_text(stmt, 1, customerUsername, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to bind parameter: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt); // Clean up
        return -1; // Error
    }
    
    // Execute the statement
    rc = sqlite3_step(stmt);

    if (rc == SQLITE_ROW) { // A row was found
        customer_out->id = sqlite3_column_int(stmt, 0);
        // Safely copy string data into the struct's fixed-size buffers
        strncpy(customer_out->username, (const char*)sqlite3_column_text(stmt, 1), sizeof(customer_out->username) - 1);
        customer_out->username[sizeof(customer_out->username) - 1] = '\0'; // Ensure null termination
        strncpy(customer_out->fullName, (const char*)sqlite3_column_text(stmt, 2), sizeof(customer_out->fullName) - 1);
        customer_out->fullName[sizeof(customer_out->fullName) - 1] = '\0';
        strncpy(customer_out->email, (const char*)sqlite3_column_text(stmt, 3), sizeof(customer_out->email) - 1);
        customer_out->email[sizeof(customer_out->email) - 1] = '\0';
        result_status = 0; // Success
    } else if (rc != SQLITE_DONE) { // An error occurred
        fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
        result_status = -1; // Error
    }
    
    // Finalize the statement to free resources
    sqlite3_finalize(stmt);
    return result_status;
}

void process_username(sqlite3* db, const char* username) {
    printf("\nSearching for username: %s\n", username);
    struct Customer customer;
    int status = getCustomerByUsername(db, username, &customer);

    if (status == 0) { // Success
        printf("Found: Customer [ID=%d, Username=%s, FullName=%s, Email=%s]\n",
               customer.id, customer.username, customer.fullName, customer.email);
    } else if (status == 1) { // Not found
        printf("Customer with username '%s' not found.\n", username);
    } else { // Error
        printf("An error occurred while searching for username '%s'.\n", username);
    }
}

int main(int argc, char* argv[]) {
    // To compile: gcc your_file_name.c -o your_program_name -lsqlite3
    sqlite3* db;
    int rc = sqlite3_open(":memory:", &db); // Use in-memory database

    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    setup_database(db);

    if (argc > 1) {
        // If a command-line argument is provided, use it
        printf("Searching for username from command line: %s\n", argv[1]);
        process_username(db, argv[1]);
    } else {
        // Otherwise, run predefined test cases
        printf("--- Running Predefined Test Cases ---\n");
        const char* testUsernames[] = {
            "jdoe",             // Test Case 1: Existing user
            "asmith",           // Test Case 2: Existing user
            "nonexistent",      // Test Case 3: Non-existing user
            "peterpan",         // Test Case 4: Existing user
            "admin' OR '1'='1"  // Test Case 5: SQL injection attempt
        };
        int num_tests = sizeof(testUsernames) / sizeof(testUsernames[0]);
        for (int i = 0; i < num_tests; ++i) {
            process_username(db, testUsernames[i]);
        }
    }

    sqlite3_close(db);
    return 0;
}