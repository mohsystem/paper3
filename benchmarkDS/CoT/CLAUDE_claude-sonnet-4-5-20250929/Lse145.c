
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <ctype.h>
#include <stdbool.h>
#include <regex.h>

#define DB_NAME "stocks.db"
#define MAX_STOCK_NAME 10

void trim(char* str) {
    if (str == NULL) return;
    
    char* start = str;
    while (isspace(*start)) start++;
    
    if (*start == 0) {
        *str = 0;
        return;
    }
    
    char* end = start + strlen(start) - 1;
    while (end > start && isspace(*end)) end--;
    
    size_t len = end - start + 1;
    memmove(str, start, len);
    str[len] = 0;
}

void toUpper(char* str) {
    if (str == NULL) return;
    for (int i = 0; str[i]; i++) {
        str[i] = toupper((unsigned char)str[i]);
    }
}

void buy_function(const char* stock_name) {
    if (stock_name == NULL || strlen(stock_name) == 0) {
        printf("Error: Invalid stock name in buy_function\\n");
        return;
    }
    printf("Processing purchase for stock: %s\\n", stock_name);
}

bool buy_stock(const char* stock_name, int quantity) {
    if (stock_name == NULL || strlen(stock_name) == 0) {
        printf("Error: Stock name cannot be empty\\n");
        return false;
    }
    
    char clean_stock_name[MAX_STOCK_NAME];
    strncpy(clean_stock_name, stock_name, MAX_STOCK_NAME - 1);
    clean_stock_name[MAX_STOCK_NAME - 1] = '\\0';
    trim(clean_stock_name);
    toUpper(clean_stock_name);
    
    if (strlen(clean_stock_name) == 0 || strlen(clean_stock_name) > 5) {
        printf("Error: Invalid stock name format\\n");
        return false;
    }
    
    regex_t regex;
    int reti = regcomp(&regex, "^[A-Z]{1,5}$", REG_EXTENDED);
    if (reti != 0) {
        printf("Error: Could not compile regex\\n");
        return false;
    }
    
    reti = regexec(&regex, clean_stock_name, 0, NULL, 0);
    regfree(&regex);
    
    if (reti != 0) {
        printf("Error: Invalid stock name format\\n");
        return false;
    }
    
    if (quantity <= 0 || quantity > 10000) {
        printf("Error: Quantity must be between 1 and 10000\\n");
        return false;
    }
    
    sqlite3* db;
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_open(DB_NAME, &db);
    if (rc != SQLITE_OK) {
        printf("Database error: %s\\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return false;
    }
    
    const char* sql = "INSERT INTO stocks (stock_name, quantity) VALUES (?, ?)";
    
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        printf("Database error: %s\\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, clean_stock_name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, quantity);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    if (rc != SQLITE_DONE) {
        printf("Database error during insert\\n");
        return false;
    }
    
    buy_function(clean_stock_name);
    printf("Stock purchased successfully\\n");
    return true;
}

void init_database() {
    sqlite3* db;
    char* err_msg = NULL;
    
    int rc = sqlite3_open(DB_NAME, &db);
    if (rc != SQLITE_OK) {
        printf("Error opening database\\n");
        return;
    }
    
    const char* sql = "CREATE TABLE IF NOT EXISTS stocks ("
                     "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                     "stock_name TEXT NOT NULL, "
                     "quantity INTEGER NOT NULL)";
    
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        printf("Error creating table: %s\\n", err_msg);
        sqlite3_free(err_msg);
    }
    
    sqlite3_close(db);
}

int main() {
    init_database();
    
    printf("Test 1: Valid purchase\\n");
    buy_stock("AAPL", 100);
    
    printf("\\nTest 2: Valid purchase\\n");
    buy_stock("GOOGL", 50);
    
    printf("\\nTest 3: Invalid stock name (too long)\\n");
    buy_stock("TOOLONG", 10);
    
    printf("\\nTest 4: Invalid quantity (negative)\\n");
    buy_stock("MSFT", -5);
    
    printf("\\nTest 5: Invalid stock name (empty)\\n");
    buy_stock("", 20);
    
    return 0;
}
