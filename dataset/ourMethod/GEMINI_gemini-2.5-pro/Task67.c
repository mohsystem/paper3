#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "sqlite3.h"

// To compile and run this code, you need to link against the SQLite3 library.
// Example: gcc Task67.c -o Task67 -lsqlite3

#define USERNAME_MAX_LEN 50
#define NAME_MAX_LEN 50
#define EMAIL_MAX_LEN 100

typedef struct {
    int id;
    char username[USERNAME_MAX_LEN + 1];
    char first_name[NAME_MAX_LEN + 1];
    char last_name[NAME_MAX_LEN + 1];
    char email[EMAIL_MAX_LEN + 1];
} Customer;

// Simple username validation: Alphanumeric, 3-20 characters.
bool isValidUsername(const char* username) {
    if (username == NULL) return false;
    size_t len = strlen(username);
    if (len < 3 || len > 20) return false;
    for (size_t i = 0; i < len; ++i) {
        if (!((username[i] >= 'a' && username[i] <= 'z') ||
              (username[i] >= 'A' && username[i] <= 'Z') ||
              (username[i] >= '0' && username[i] <= '9'))) {
            return false;
        }
    }
    return true;
}

void printCustomer(const Customer* customer) {
    if (customer == NULL) return;
    printf("Customer{id=%d, username='%s', firstName='%s', lastName='%s', email='%s'}\n",
           customer->id, customer->username, customer->first_name,
           customer->last_name, customer->email);
}

int setupDatabase(sqlite3* db) {
    char* errMsg = NULL;
    const char* createTableSQL =
        "CREATE TABLE IF NOT EXISTS customer ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT NOT NULL UNIQUE,"
        "first_name TEXT NOT NULL,"
        "last_name TEXT NOT NULL,"
        "email TEXT NOT NULL UNIQUE"
        ");";

    const char* insertSQLs[] = {
        "INSERT OR IGNORE INTO customer(username, first_name, last_name, email) VALUES('jdoe', 'John', 'Doe', 'john.doe@example.com');",
        "INSERT OR IGNORE INTO customer(username, first_name, last_name, email) VALUES('asmith', 'Alice', 'Smith', 'alice.smith@example.com');",
        "INSERT OR IGNORE INTO customer(username, first_name, last_name, email) VALUES('bwilliams', 'Bob', 'Williams', 'bob.williams@example.com');",
        "INSERT OR IGNORE INTO customer(username, first_name, last_name, email) VALUES('mjones', 'Mary', 'Jones', 'mary.jones@example.com');",
        NULL
    };

    if (sqlite3_exec(db, createTableSQL, 0, 0, &errMsg) != SQLITE_OK) {
        fprintf(stderr, "SQL error (CREATE): %s\n", errMsg);
        sqlite3_free(errMsg);
        return -1;
    }

    for (int i = 0; insertSQLs[i] != NULL; ++i) {
        if (sqlite3_exec(db, insertSQLs[i], 0, 0, &errMsg) != SQLITE_OK) {
            fprintf(stderr, "SQL error (INSERT): %s\n", errMsg);
            sqlite3_free(errMsg);
            return -1;
        }
    }
    return 0;
}

// Returns 0 if found, 1 if not found, -1 on error.
int getCustomerByUsername(sqlite3* db, const char* username, Customer* out_customer) {
    if (db == NULL || username == NULL || out_customer == NULL) {
        return -1;
    }
    if (!isValidUsername(username)) {
        fprintf(stderr, "Invalid username format provided.\n");
        return -1;
    }

    sqlite3_stmt* stmt = NULL;
    const char* sql = "SELECT id, username, first_name, last_name, email FROM customer WHERE username = ?;";
    int result_code = 1; // Default to not found

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    if (sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC) != SQLITE_OK) {
        fprintf(stderr, "Failed to bind parameter: %s\n", sqlite3_errmsg(db));
        result_code = -1;
        goto cleanup;
    }

    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        out_customer->id = sqlite3_column_int(stmt, 0);
        const unsigned char* u = sqlite3_column_text(stmt, 1);
        const unsigned char* f = sqlite3_column_text(stmt, 2);
        const unsigned char* l = sqlite3_column_text(stmt, 3);
        const unsigned char* e = sqlite3_column_text(stmt, 4);

        if (u) snprintf(out_customer->username, sizeof(out_customer->username), "%s", u);
        if (f) snprintf(out_customer->first_name, sizeof(out_customer->first_name), "%s", f);
        if (l) snprintf(out_customer->last_name, sizeof(out_customer->last_name), "%s", l);
        if (e) snprintf(out_customer->email, sizeof(out_customer->email), "%s", e);

        result_code = 0; // Found
    } else if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to step statement: %s\n", sqlite3_errmsg(db));
        result_code = -1;
    }

cleanup:
    sqlite3_finalize(stmt);
    return result_code;
}

int main(int argc, char* argv[]) {
    sqlite3* db;
    if (sqlite3_open(":memory:", &db)) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    if (setupDatabase(db) != 0) {
        sqlite3_close(db);
        return 1;
    }

    if (argc > 1) {
        printf("Searching for username from command line argument: %s\n", argv[1]);
        Customer customer;
        memset(&customer, 0, sizeof(Customer));
        int result = getCustomerByUsername(db, argv[1], &customer);
        if (result == 0) {
            printf("Found: ");
            printCustomer(&customer);
        } else if (result == 1) {
            printf("Customer not found.\n");
        } else {
            fprintf(stderr, "An error occurred.\n");
        }
    } else {
        printf("Running test cases...\n");
        const char* testUsernames[] = {"jdoe", "asmith", "bwilliams", "enonexistent", "mjones"};
        int num_tests = sizeof(testUsernames) / sizeof(testUsernames[0]);

        for (int i = 0; i < num_tests; ++i) {
            printf("\n--- Test Case %d: Searching for username '%s' ---\n", i + 1, testUsernames[i]);
            Customer customer;
            memset(&customer, 0, sizeof(Customer));
            int result = getCustomerByUsername(db, testUsernames[i], &customer);
            if (result == 0) {
                printf("Found: ");
                printCustomer(&customer);
            } else if (result == 1) {
                printf("Customer not found.\n");
            } else {
                fprintf(stderr, "An error occurred during search.\n");
            }
        }
    }

    sqlite3_close(db);
    return 0;
}