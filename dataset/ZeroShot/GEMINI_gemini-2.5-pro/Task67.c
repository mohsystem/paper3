#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite3.h"

// Note: To compile this code, you need to link against the SQLite3 library.
// Example: gcc your_source_file.c -o your_executable -lsqlite3

const char* DB_NAME = "customers.db";

typedef struct {
    char* username;
    char* firstName;
    char* lastName;
    char* email;
} Customer;

// Helper to free memory for a Customer struct
void free_customer(Customer* c) {
    if (c) {
        free(c->username);
        free(c->firstName);
        free(c->lastName);
        free(c->email);
        free(c);
    }
}

// Helper to duplicate a string
char* str_dup(const char* s) {
    if (!s) return NULL;
    char* d = malloc(strlen(s) + 1);
    if (d) {
        strcpy(d, s);
    }
    return d;
}

void setup_database(sqlite3* db) {
    char* zErrMsg = 0;
    
    const char* createTableSQL = "CREATE TABLE IF NOT EXISTS customer ("
        "customer_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "customerusername TEXT NOT NULL UNIQUE,"
        "first_name TEXT NOT NULL,"
        "last_name TEXT NOT NULL,"
        "email TEXT NOT NULL UNIQUE);";
    
    int rc = sqlite3_exec(db, createTableSQL, NULL, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error (CREATE): %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    
    sqlite3_exec(db, "DELETE FROM customer;", NULL, 0, &zErrMsg);
    
    const char* insertSQL =
        "INSERT INTO customer(customerusername, first_name, last_name, email) VALUES"
        "('jdoe', 'John', 'Doe', 'john.doe@example.com'),"
        "('asmith', 'Anna', 'Smith', 'anna.smith@example.com'),"
        "('sjobs', 'Steve', 'Jobs', 's.jobs@example.com'),"
        "('bgates', 'Bill', 'Gates', 'bill.g@example.com');";

    rc = sqlite3_exec(db, insertSQL, NULL, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error (INSERT): %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
}

/**
 * Retrieves a customer from the database by username. Uses prepared statements.
 * @param db The SQLite database connection handle.
 * @param username The username to search for.
 * @return A pointer to a dynamically allocated Customer struct if found, otherwise NULL.
 *         The caller is responsible for freeing the returned struct using free_customer().
 */
Customer* get_customer_by_username(sqlite3* db, const char* username) {
    sqlite3_stmt* stmt = NULL;
    const char* sql = "SELECT customerusername, first_name, last_name, email FROM customer WHERE customerusername = ?;";
    Customer* customer = NULL;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return NULL;
    }

    // Bind the username to the placeholder '?' to prevent SQL injection
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        customer = (Customer*)malloc(sizeof(Customer));
        if (customer) {
            customer->username = str_dup((const char*)sqlite3_column_text(stmt, 0));
            customer->firstName = str_dup((const char*)sqlite3_column_text(stmt, 1));
            customer->lastName = str_dup((const char*)sqlite3_column_text(stmt, 2));
            customer->email = str_dup((const char*)sqlite3_column_text(stmt, 3));
        }
    } else if (rc != SQLITE_DONE) {
        fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
    return customer;
}

int main(int argc, char* argv[]) {
    sqlite3* db;
    int rc = sqlite3_open(DB_NAME, &db);

    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    setup_database(db);

    const char* test_usernames[5];
    int num_tests = 0;

    if (argc > 1) {
        printf("--- Running with provided command-line argument ---\n");
        test_usernames[0] = argv[1];
        num_tests = 1;
    } else {
        printf("--- Running 5 built-in test cases ---\n");
        test_usernames[0] = "jdoe";
        test_usernames[1] = "asmith";
        test_usernames[2] = "nonexistent";
        test_usernames[3] = "sjobs";
        test_usernames[4] = "' OR 1=1; --";
        num_tests = 5;
    }

    for (int i = 0; i < num_tests; ++i) {
        const char* username = test_usernames[i];
        printf("\nSearching for username: '%s'\n", username);
        Customer* cust = get_customer_by_username(db, username);
        if (cust) {
            printf("Found: Customer{username='%s', firstName='%s', lastName='%s', email='%s'}\n",
                   cust->username, cust->firstName, cust->lastName, cust->email);
            free_customer(cust); // Free the memory allocated by the function
        } else {
            printf("Result: Customer not found.\n");
        }
    }
    
    sqlite3_close(db);
    return 0;
}