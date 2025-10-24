#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite3.h"

/*
Note: To compile and run this C code, you need the SQLite3 library.
1. Install SQLite3 development libraries (e.g., `sudo apt-get install libsqlite3-dev` on Debian/Ubuntu).
2. Compile: gcc -o task67_c Task67.c -lsqlite3
3. Run: ./task67_c [optional_username]
*/

const char* DB_NAME = "customers.db";

// Struct to hold customer data
typedef struct {
    int id;
    char* username;
    char* firstName;
    char* lastName;
    char* email;
} Customer;

// Utility to safely duplicate a string, handling NULLs
char* safe_strdup(const unsigned char* text) {
    if (text == NULL) return NULL;
    return strdup((const char*)text);
}

// Function to free the memory allocated for a Customer struct's members
void free_customer(Customer* cust) {
    if (cust) {
        free(cust->username);
        free(cust->firstName);
        free(cust->lastName);
        free(cust->email);
    }
}

// Function to print customer details
void print_customer(const Customer* cust) {
    printf("Customer{id=%d, username='%s', firstName='%s', lastName='%s', email='%s'}\n",
           cust->id, cust->username, cust->firstName, cust->lastName, cust->email);
}

// Sets up the database, creates the table, and inserts sample data
void setup_database() {
    sqlite3* db;
    char* zErrMsg = 0;

    if (sqlite3_open(DB_NAME, &db)) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    const char* sql = 
        "CREATE TABLE IF NOT EXISTS customer ("
        "customerid INTEGER PRIMARY KEY AUTOINCREMENT, "
        "customerusername TEXT NOT NULL UNIQUE, "
        "firstname TEXT, "
        "lastname TEXT, "
        "email TEXT);"

        "INSERT OR IGNORE INTO customer(customerusername, firstname, lastname, email) VALUES('jdoe', 'John', 'Doe', 'john.doe@email.com');"
        "INSERT OR IGNORE INTO customer(customerusername, firstname, lastname, email) VALUES('asmith', 'Alice', 'Smith', 'alice.s@web.com');"
        "INSERT OR IGNORE INTO customer(customerusername, firstname, lastname, email) VALUES('bwhite', 'Bob', 'White', 'bwhite@mail.org');"
        "INSERT OR IGNORE INTO customer(customerusername, firstname, lastname, email) VALUES('cjones', 'Carol', 'Jones', 'carolj@email.com');";

    if (sqlite3_exec(db, sql, 0, 0, &zErrMsg) != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    
    sqlite3_close(db);
}

/**
 * Retrieves customer information into a provided struct.
 * @param customer_username The username to search for.
 * @param out_customer Pointer to a Customer struct to be filled.
 * @return 1 if found, 0 if not found, -1 on error.
 */
int get_customer_by_username(const char* customer_username, Customer* out_customer) {
    sqlite3* db;
    sqlite3_stmt* stmt;
    int rc;
    int result = 0; // 0 = not found

    if (sqlite3_open(DB_NAME, &db) != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return -1; // Error
    }

    const char* sql = "SELECT customerid, customerusername, firstname, lastname, email FROM customer WHERE customerusername = ?;";
    
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, customer_username, -1, SQLITE_STATIC);
        
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            out_customer->id = sqlite3_column_int(stmt, 0);
            // We must copy the strings as the pointers returned by sqlite3_column_text are temporary
            out_customer->username = safe_strdup(sqlite3_column_text(stmt, 1));
            out_customer->firstName = safe_strdup(sqlite3_column_text(stmt, 2));
            out_customer->lastName = safe_strdup(sqlite3_column_text(stmt, 3));
            out_customer->email = safe_strdup(sqlite3_column_text(stmt, 4));
            result = 1; // Found
        }
    } else {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        result = -1; // Error
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return result;
}

int main(int argc, char* argv[]) {
    setup_database();

    printf("--- Running 5 Test Cases ---\n");
    const char* test_usernames[] = {"asmith", "jdoe", "nonexistent", "bwhite", "imnotreal"};
    for (int i = 0; i < 5; ++i) {
        printf("\nTest Case %d: Searching for '%s'\n", (i + 1), test_usernames[i]);
        Customer cust = {0}; // Initialize to zero
        int found = get_customer_by_username(test_usernames[i], &cust);
        if (found == 1) {
            printf("Found: ");
            print_customer(&cust);
            free_customer(&cust); // IMPORTANT: Free the allocated strings
        } else if (found == 0) {
            printf("Customer not found.\n");
        } else {
            printf("An error occurred during search.\n");
        }
    }
    printf("----------------------------\n\n");

    if (argc > 1) {
        const char* username_from_arg = argv[1];
        printf("--- Running with Command-Line Argument ---\n");
        printf("Searching for '%s'\n", username_from_arg);
        Customer cust = {0};
        int found = get_customer_by_username(username_from_arg, &cust);
        if (found == 1) {
            printf("Found: ");
            print_customer(&cust);
            free_customer(&cust);
        } else if (found == 0) {
            printf("Customer not found.\n");
        } else {
            printf("An error occurred during search.\n");
        }
        printf("------------------------------------------\n");
    }

    return 0;
}