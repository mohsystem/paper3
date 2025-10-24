
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <ctype.h>
#include <stdbool.h>

#define DB_PATH "products.db"
#define MAX_INPUT_LENGTH 50

bool validate_input(const char* input) {
    if (input == NULL || strlen(input) == 0 || strlen(input) > MAX_INPUT_LENGTH) {
        return false;
    }
    
    for (size_t i = 0; i < strlen(input); i++) {
        if (!isalnum(input[i]) && input[i] != '_' && input[i] != '-') {
            return false;
        }
    }
    
    return true;
}

void initialize_database() {
    sqlite3* db;
    char* err_msg = NULL;
    
    if (sqlite3_open(DB_PATH, &db) != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\\n", sqlite3_errmsg(db));
        return;
    }
    
    const char* drop_table = "DROP TABLE IF EXISTS products";
    sqlite3_exec(db, drop_table, NULL, NULL, &err_msg);
    
    const char* create_table = 
        "CREATE TABLE products ("
        "id TEXT PRIMARY KEY, "
        "name TEXT NOT NULL, "
        "description TEXT, "
        "price REAL NOT NULL, "
        "stock INTEGER NOT NULL)";
    
    if (sqlite3_exec(db, create_table, NULL, NULL, &err_msg) != SQLITE_OK) {
        fprintf(stderr, "Create table error: %s\\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return;
    }
    
    const char* insert_sql = "INSERT INTO products (id, name, description, price, stock) VALUES (?, ?, ?, ?, ?)";
    sqlite3_stmt* stmt;
    
    const char* products[][5] = {
        {"PROD001", "Laptop", "High-performance laptop", "999.99", "50"},
        {"PROD002", "Mouse", "Wireless mouse", "29.99", "200"},
        {"PROD003", "Keyboard", "Mechanical keyboard", "79.99", "150"},
        {"PROD004", "Monitor", "27-inch 4K monitor", "399.99", "75"},
        {"PROD005", "Headphones", "Noise-cancelling headphones", "199.99", "100"}
    };
    
    for (int i = 0; i < 5; i++) {
        if (sqlite3_prepare_v2(db, insert_sql, -1, &stmt, NULL) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, products[i][0], -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, products[i][1], -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, products[i][2], -1, SQLITE_TRANSIENT);
            sqlite3_bind_double(stmt, 4, atof(products[i][3]));
            sqlite3_bind_int(stmt, 5, atoi(products[i][4]));
            
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }
    }
    
    sqlite3_close(db);
}

void get_product_details(const char* product_id) {
    if (!validate_input(product_id)) {
        printf("Invalid product ID format. Please use alphanumeric characters, hyphens, or underscores only.\\n");
        return;
    }
    
    sqlite3* db;
    if (sqlite3_open(DB_PATH, &db) != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\\n", sqlite3_errmsg(db));
        return;
    }
    
    const char* query = "SELECT id, name, description, price, stock FROM products WHERE id = ?";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, product_id, -1, SQLITE_TRANSIENT);
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            printf("Product Found:\\n");
            printf("ID: %s\\n", sqlite3_column_text(stmt, 0));
            printf("Name: %s\\n", sqlite3_column_text(stmt, 1));
            printf("Description: %s\\n", sqlite3_column_text(stmt, 2));
            printf("Price: $%.2f\\n", sqlite3_column_double(stmt, 3));
            printf("Stock: %d\\n", sqlite3_column_int(stmt, 4));
        } else {
            printf("Product not found with ID: %s\\n", product_id);
        }
        
        sqlite3_finalize(stmt);
    } else {
        fprintf(stderr, "Query preparation error: %s\\n", sqlite3_errmsg(db));
    }
    
    sqlite3_close(db);
}

int main() {
    initialize_database();
    
    printf("Test Case 1: Valid product ID\\n");
    get_product_details("PROD001");
    
    printf("\\nTest Case 2: Valid product ID\\n");
    get_product_details("PROD002");
    
    printf("\\nTest Case 3: Non-existent product\\n");
    get_product_details("PROD999");
    
    printf("\\nTest Case 4: Invalid input (SQL injection attempt)\\n");
    get_product_details("PROD001' OR '1'='1");
    
    printf("\\nTest Case 5: Empty input\\n");
    get_product_details("");
    
    return 0;
}
