
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <ctype.h>

/* Customer structure to hold retrieved data */
typedef struct {
    int id;
    char username[256];
    char email[256];
    char fullname[256];
} Customer;

/* Function to validate username contains only safe characters
 * CWE-20: Input Validation - prevents injection attacks */
int validate_username(const char* username) {
    size_t len;
    size_t i;
    
    if (!username) {
        return 0;
    }
    
    len = strlen(username);
    
    /* Check length constraints (CWE-129: Buffer bounds validation) */
    if (len == 0 || len > 255) {
        return 0;
    }
    
    /* Validate characters - only alphanumeric, underscore, dash, dot */
    for (i = 0; i < len; i++) {
        if (!isalnum((unsigned char)username[i]) && 
            username[i] != '_' && 
            username[i] != '-' && 
            username[i] != '.') {
            return 0;
        }
    }
    
    return 1;
}

/* Function to safely copy string with bounds checking
 * CWE-120: Buffer overflow prevention */
void safe_string_copy(char* dest, const char* src, size_t dest_size) {
    if (!dest || !src || dest_size == 0) {
        return;
    }
    
    /* Use snprintf for safe string copy with null termination */
    snprintf(dest, dest_size, "%s", src);
    dest[dest_size - 1] = '\\0'; /* Ensure null termination */
}

/* Function to retrieve customer by username from database
 * CWE-89 Prevention: Uses parameterized queries */
int get_customer_by_username(const char* db_path, const char* username, Customer* customer) {
    sqlite3* db = NULL;
    sqlite3_stmt* stmt = NULL;
    int rc;
    const char* sql;
    const unsigned char* text;
    
    /* CWE-20: Validate all inputs before processing */
    if (!db_path || !username || !customer) {
        fprintf(stderr, "Error: Invalid parameters\\n");
        return 0;
    }
    
    /* Initialize customer structure to zero (CWE-457: Uninitialized variable) */
    memset(customer, 0, sizeof(Customer));
    
    /* Validate username input */
    if (!validate_username(username)) {
        fprintf(stderr, "Error: Invalid username format\\n");
        return 0;
    }
    
    /* Open database connection with error checking */
    rc = sqlite3_open(db_path, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error: Cannot open database: %s\\n", 
                db ? sqlite3_errmsg(db) : "unknown error");
        if (db) {
            sqlite3_close(db);
        }
        return 0;
    }
    
    /* CWE-89 Prevention: Use parameterized query to prevent SQL injection */
    sql = "SELECT id, username, email, fullname FROM customer WHERE username = ? LIMIT 1;";
    
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error: Failed to prepare statement: %s\\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0;
    }
    
    /* Bind parameter safely (CWE-89 Prevention) */
    rc = sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error: Failed to bind parameter: %s\\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 0;
    }
    
    /* Execute query and retrieve results */
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        /* Safely extract data with bounds checking (CWE-120 Prevention) */
        customer->id = sqlite3_column_int(stmt, 0);
        
        text = sqlite3_column_text(stmt, 1);
        if (text) {
            safe_string_copy(customer->username, (const char*)text, sizeof(customer->username));
        }
        
        text = sqlite3_column_text(stmt, 2);
        if (text) {
            safe_string_copy(customer->email, (const char*)text, sizeof(customer->email));
        }
        
        text = sqlite3_column_text(stmt, 3);
        if (text) {
            safe_string_copy(customer->fullname, (const char*)text, sizeof(customer->fullname));
        }
        
        /* Clean up resources before returning success */
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1;
        
    } else if (rc == SQLITE_DONE) {
        fprintf(stderr, "Error: No customer found with username: %s\\n", username);
    } else {
        fprintf(stderr, "Error: Query execution failed: %s\\n", sqlite3_errmsg(db));
    }
    
    /* Clean up resources on failure */
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0;
}

int main(int argc, char* argv[]) {
    Customer customer;
    const char* db_path = "customer.db";
    const char* test_usernames[] = {"john_doe", "jane_smith", "test_user", "admin", "user123"};
    int i;
    int num_tests = 5;
    
    /* CWE-20: Validate command line arguments */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <customer_username>\\n", argc > 0 ? argv[0] : "program");
        return 1;
    }
    
    /* CWE-20: Validate argument is not null and not empty */
    if (!argv[1] || strlen(argv[1]) == 0) {
        fprintf(stderr, "Error: Username cannot be empty\\n");
        return 1;
    }
    
    /* Run test cases */
    printf("=== Running Test Cases ===\\n");
    for (i = 0; i < num_tests; i++) {
        printf("\\nTest: Retrieving customer '%s'\\n", test_usernames[i]);
        
        if (get_customer_by_username(db_path, test_usernames[i], &customer)) {
            printf("Success! Customer found:\\n");
            printf("  ID: %d\\n", customer.id);
            printf("  Username: %s\\n", customer.username);
            printf("  Email: %s\\n", customer.email);
            printf("  Full Name: %s\\n", customer.fullname);
        } else {
            printf("Customer not found or error occurred\\n");
        }
    }
    
    /* Retrieve customer from command line argument */
    printf("\\n=== Retrieving Customer from Command Line ===\\n");
    if (get_customer_by_username(db_path, argv[1], &customer)) {
        printf("Customer Information:\\n");
        printf("  ID: %d\\n", customer.id);
        printf("  Username: %s\\n", customer.username);
        printf("  Email: %s\\n", customer.email);
        printf("  Full Name: %s\\n", customer.fullname);
        return 0;
    } else {
        fprintf(stderr, "Failed to retrieve customer information\\n");
        return 1;
    }
}
