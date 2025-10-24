
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>

typedef struct {
    char username[100];
    char name[100];
    char email[100];
    char phone[20];
} Customer;

Customer* getCustomerByUsername(const char* customerUsername) {
    if (customerUsername == NULL || strlen(customerUsername) == 0) {
        return NULL;
    }
    
    sqlite3* db;
    sqlite3_stmt* stmt;
    const char* dbPath = "customer.db";
    
    if (sqlite3_open(dbPath, &db) != SQLITE_OK) {
        fprintf(stderr, "Database error: %s\\n", sqlite3_errmsg(db));
        return NULL;
    }
    
    const char* query = "SELECT username, name, email, phone FROM customer WHERE username = ?";
    
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Query preparation error: %s\\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return NULL;
    }
    
    sqlite3_bind_text(stmt, 1, customerUsername, -1, SQLITE_TRANSIENT);
    
    Customer* customer = NULL;
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        customer = (Customer*)malloc(sizeof(Customer));
        if (customer != NULL) {
            strncpy(customer->username, (const char*)sqlite3_column_text(stmt, 0), 99);
            customer->username[99] = '\\0';
            strncpy(customer->name, (const char*)sqlite3_column_text(stmt, 1), 99);
            customer->name[99] = '\\0';
            strncpy(customer->email, (const char*)sqlite3_column_text(stmt, 2), 99);
            customer->email[99] = '\\0';
            strncpy(customer->phone, (const char*)sqlite3_column_text(stmt, 3), 19);
            customer->phone[19] = '\\0';
        }
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return customer;
}

void initializeDatabase() {
    sqlite3* db;
    char* errMsg = NULL;
    
    if (sqlite3_open("customer.db", &db) != SQLITE_OK) {
        fprintf(stderr, "Database initialization error\\n");
        return;
    }
    
    const char* createTable = 
        "CREATE TABLE IF NOT EXISTS customer ("
        "username TEXT PRIMARY KEY, "
        "name TEXT, "
        "email TEXT, "
        "phone TEXT)";
    
    sqlite3_exec(db, createTable, NULL, NULL, &errMsg);
    sqlite3_exec(db, "DELETE FROM customer", NULL, NULL, &errMsg);
    
    sqlite3_exec(db, "INSERT INTO customer VALUES ('john_doe', 'John Doe', 'john@example.com', '555-0101')", NULL, NULL, &errMsg);
    sqlite3_exec(db, "INSERT INTO customer VALUES ('jane_smith', 'Jane Smith', 'jane@example.com', '555-0102')", NULL, NULL, &errMsg);
    sqlite3_exec(db, "INSERT INTO customer VALUES ('bob_wilson', 'Bob Wilson', 'bob@example.com', '555-0103')", NULL, NULL, &errMsg);
    sqlite3_exec(db, "INSERT INTO customer VALUES ('alice_brown', 'Alice Brown', 'alice@example.com', '555-0104')", NULL, NULL, &errMsg);
    sqlite3_exec(db, "INSERT INTO customer VALUES ('charlie_davis', 'Charlie Davis', 'charlie@example.com', '555-0105')", NULL, NULL, &errMsg);
    
    sqlite3_close(db);
}

void printCustomer(const Customer* customer) {
    printf("Customer{username='%s', name='%s', email='%s', phone='%s'}\\n",
           customer->username, customer->name, customer->email, customer->phone);
}

int main(int argc, char* argv[]) {
    initializeDatabase();
    
    const char* testCases[] = {"john_doe", "jane_smith", "bob_wilson", "nonexistent_user", ""};
    
    if (argc > 1) {
        Customer* customer = getCustomerByUsername(argv[1]);
        if (customer != NULL) {
            printf("Found: ");
            printCustomer(customer);
            free(customer);
        } else {
            printf("Customer not found: %s\\n", argv[1]);
        }
    } else {
        printf("Running test cases:\\n");
        for (int i = 0; i < 5; i++) {
            printf("\\nSearching for: '%s'\\n", testCases[i]);
            Customer* customer = getCustomerByUsername(testCases[i]);
            if (customer != NULL) {
                printf("Result: ");
                printCustomer(customer);
                free(customer);
            } else {
                printf("Result: Customer not found\\n");
            }
        }
    }
    
    return 0;
}
