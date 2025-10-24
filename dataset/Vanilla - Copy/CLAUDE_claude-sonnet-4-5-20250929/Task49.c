
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

#define DB_NAME "users.db"

void init_database() {
    sqlite3* db;
    char* err_msg = NULL;
    
    int rc = sqlite3_open(DB_NAME, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
    
    const char* sql = "CREATE TABLE IF NOT EXISTS users ("
                     "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                     "name TEXT NOT NULL,"
                     "email TEXT NOT NULL,"
                     "age INTEGER);";
    
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\\n", err_msg);
        sqlite3_free(err_msg);
    }
    
    sqlite3_close(db);
}

char* add_user(const char* name, const char* email, int age) {
    sqlite3* db;
    sqlite3_stmt* stmt;
    char* result = malloc(256);
    
    int rc = sqlite3_open(DB_NAME, &db);
    if (rc != SQLITE_OK) {
        sprintf(result, "{\\"status\\":\\"error\\",\\"message\\":\\"Cannot open database\\"}");
        return result;
    }
    
    const char* sql = "INSERT INTO users (name, email, age) VALUES (?, ?, ?);";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    
    if (rc != SQLITE_OK) {
        sprintf(result, "{\\"status\\":\\"error\\",\\"message\\":\\"Failed to prepare statement\\"}");
        sqlite3_close(db);
        return result;
    }
    
    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, email, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, age);
    
    rc = sqlite3_step(stmt);
    
    if (rc == SQLITE_DONE) {
        long long id = sqlite3_last_insert_rowid(db);
        sprintf(result, "{\\"status\\":\\"success\\",\\"id\\":%lld}", id);
    } else {
        sprintf(result, "{\\"status\\":\\"error\\",\\"message\\":\\"Failed to insert user\\"}");
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return result;
}

char* get_users() {
    sqlite3* db;
    sqlite3_stmt* stmt;
    char* result = malloc(4096);
    strcpy(result, "[");
    
    int rc = sqlite3_open(DB_NAME, &db);
    if (rc != SQLITE_OK) {
        sprintf(result, "{\\"status\\":\\"error\\",\\"message\\":\\"Cannot open database\\"}");
        return result;
    }
    
    const char* sql = "SELECT * FROM users;";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    
    if (rc != SQLITE_OK) {
        sprintf(result, "{\\"status\\":\\"error\\",\\"message\\":\\"Failed to prepare statement\\"}");
        sqlite3_close(db);
        return result;
    }
    
    int first = 1;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (!first) strcat(result, ",");
        
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char* name = sqlite3_column_text(stmt, 1);
        const unsigned char* email = sqlite3_column_text(stmt, 2);
        int age = sqlite3_column_int(stmt, 3);
        
        char user_str[512];
        sprintf(user_str, "{\\"id\\":%d,\\"name\\":\\"%s\\",\\"email\\":\\"%s\\",\\"age\\":%d}",
                id, name, email, age);
        strcat(result, user_str);
        
        first = 0;
    }
    
    strcat(result, "]");
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return result;
}

int main() {
    init_database();
    
    printf("Test Case 1: Add user John\\n");
    char* result1 = add_user("John Doe", "john@example.com", 30);
    printf("%s\\n", result1);
    free(result1);
    
    printf("\\nTest Case 2: Add user Jane\\n");
    char* result2 = add_user("Jane Smith", "jane@example.com", 25);
    printf("%s\\n", result2);
    free(result2);
    
    printf("\\nTest Case 3: Add user Bob\\n");
    char* result3 = add_user("Bob Johnson", "bob@example.com", 35);
    printf("%s\\n", result3);
    free(result3);
    
    printf("\\nTest Case 4: Add user Alice\\n");
    char* result4 = add_user("Alice Brown", "alice@example.com", 28);
    printf("%s\\n", result4);
    free(result4);
    
    printf("\\nTest Case 5: Get all users\\n");
    char* result5 = get_users();
    printf("%s\\n", result5);
    free(result5);
    
    return 0;
}
