
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

typedef struct {
    char username[100];
    char name[100];
    char email[100];
    char phone[50];
} Customer;

Customer* getCustomerByUsername(const char* customerUsername) {
    sqlite3* db = NULL;
    sqlite3_stmt* stmt = NULL;
    Customer* customer = NULL;
    
    int rc = sqlite3_open("customer.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\\n", sqlite3_errmsg(db));
        return NULL;
    }
    
    const char* sql = "SELECT * FROM customer WHERE customerusername = ?";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, customerUsername, -1, SQLITE_TRANSIENT);
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            customer = (Customer*)malloc(sizeof(Customer));
            if (customer) {
                strncpy(customer->username, (const char*)sqlite3_column_text(stmt, 0), 99);
                customer->username[99] = '\\0';
                strncpy(customer->name, (const char*)sqlite3_column_text(stmt, 1), 99);
                customer->name[99] = '\\0';
                strncpy(customer->email, (const char*)sqlite3_column_text(stmt, 2), 99);
                customer->email[99] = '\\0';
                strncpy(customer->phone, (const char*)sqlite3_column_text(stmt, 3), 49);
                customer->phone[49] = '\\0';
            }
        }
    } else {
        fprintf(stderr, "SQL error: %s\\n", sqlite3_errmsg(db));
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return customer;
}

void setupTestDatabase() {
    sqlite3* db = NULL;
    char* errMsg = NULL;
    
    int rc = sqlite3_open("customer.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\\n", sqlite3_errmsg(db));
        return;
    }
    
    const char* sql = 
        "DROP TABLE IF EXISTS customer;"
        "CREATE TABLE customer ("
        "customerusername TEXT PRIMARY KEY, "
        "name TEXT, "
        "email TEXT, "
        "phone TEXT);"
        "INSERT INTO customer VALUES ('user1', 'John Doe', 'john@email.com', '123-456-7890');"
        "INSERT INTO customer VALUES ('user2', 'Jane Smith', 'jane@email.com', '098-765-4321');"
        "INSERT INTO customer VALUES ('user3', 'Bob Johnson', 'bob@email.com', '555-555-5555');"
        "INSERT INTO customer VALUES ('user4', 'Alice Brown', 'alice@email.com', '444-444-4444');"
        "INSERT INTO customer VALUES ('user5', 'Charlie Davis', 'charlie@email.com', '333-333-3333');";
    
    rc = sqlite3_exec(db, sql, NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\\n", errMsg);
        sqlite3_free(errMsg);
    }
    
    sqlite3_close(db);
}

void printCustomer(const Customer* customer) {
    printf("Customer{username='%s', name='%s', email='%s', phone='%s'}\\n",
           customer->username, customer->name, customer->email, customer->phone);
}

int main(int argc, char* argv[]) {
    setupTestDatabase();
    
    const char* testCases[] = {"user1", "user2", "user3", "user4", "nonexistent"};
    int numTests = 5;
    
    if (argc > 1) {
        Customer* customer = getCustomerByUsername(argv[1]);
        if (customer) {
            printf("Found: ");
            printCustomer(customer);
            free(customer);
        } else {
            printf("Customer not found with username: %s\\n", argv[1]);
        }
    } else {
        printf("Running test cases:\\n");
        for (int i = 0; i < numTests; i++) {
            printf("\\nSearching for: %s\\n", testCases[i]);
            Customer* customer = getCustomerByUsername(testCases[i]);
            if (customer) {
                printf("Found: ");
                printCustomer(customer);
                free(customer);
            } else {
                printf("Customer not found\\n");
            }
        }
    }
    
    return 0;
}
