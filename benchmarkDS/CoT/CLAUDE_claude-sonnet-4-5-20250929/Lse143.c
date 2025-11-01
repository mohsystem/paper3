
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <ctype.h>
#include <regex.h>

#define MAX_EMAIL_LENGTH 256

typedef struct {
    sqlite3* db;
    char db_path[256];
} Lse143;

void trim(char* str) {
    if (str == NULL) return;
    
    char* end;
    while(isspace((unsigned char)*str)) str++;
    
    if(*str == 0) return;
    
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    
    *(end + 1) = '\\0';
    
    memmove(str - (str - str), str, strlen(str) + 1);
}

void toLower(char* str) {
    if (str == NULL) return;
    for(int i = 0; str[i]; i++) {
        str[i] = tolower((unsigned char)str[i]);
    }
}

int isValidEmail(const char* email) {
    if (email == NULL || strlen(email) == 0) return 0;
    
    regex_t regex;
    int ret = regcomp(&regex, "^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$", 
                     REG_EXTENDED);
    if (ret) return 0;
    
    ret = regexec(&regex, email, 0, NULL, 0);
    regfree(&regex);
    
    return (ret == 0);
}

void initDatabase(Lse143* service) {
    int rc = sqlite3_open(service->db_path, &service->db);
    if (rc) {
        fprintf(stderr, "Cannot open database: %s\\n", sqlite3_errmsg(service->db));
        return;
    }
    
    const char* sql = "CREATE TABLE IF NOT EXISTS subscribers ("
                     "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                     "email TEXT UNIQUE NOT NULL)";
    
    char* errMsg = NULL;
    rc = sqlite3_exec(service->db, sql, NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\\n", errMsg);
        sqlite3_free(errMsg);
    }
    sqlite3_close(service->db);
}

char* unsubscribeUser(Lse143* service, const char* email) {
    static char result[512];
    
    if (email == NULL || strlen(email) == 0) {
        strcpy(result, "Invalid email parameter");
        return result;
    }
    
    char cleanEmail[MAX_EMAIL_LENGTH];
    strncpy(cleanEmail, email, MAX_EMAIL_LENGTH - 1);
    cleanEmail[MAX_EMAIL_LENGTH - 1] = '\\0';
    
    trim(cleanEmail);
    toLower(cleanEmail);
    
    if (!isValidEmail(cleanEmail)) {
        strcpy(result, "Invalid email format");
        return result;
    }
    
    int rc = sqlite3_open(service->db_path, &service->db);
    if (rc) {
        strcpy(result, "Database connection error");
        return result;
    }
    
    sqlite3_stmt* stmt;
    const char* checkSql = "SELECT COUNT(*) FROM subscribers WHERE email = ?";
    rc = sqlite3_prepare_v2(service->db, checkSql, -1, &stmt, NULL);
    
    if (rc != SQLITE_OK) {
        sqlite3_close(service->db);
        strcpy(result, "Database error");
        return result;
    }
    
    sqlite3_bind_text(stmt, 1, cleanEmail, -1, SQLITE_TRANSIENT);
    rc = sqlite3_step(stmt);
    
    int count = 0;
    if (rc == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    
    if (count > 0) {
        const char* deleteSql = "DELETE FROM subscribers WHERE email = ?";
        rc = sqlite3_prepare_v2(service->db, deleteSql, -1, &stmt, NULL);
        
        if (rc == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, cleanEmail, -1, SQLITE_TRANSIENT);
            rc = sqlite3_step(stmt);
            
            if (rc == SQLITE_DONE) {
                snprintf(result, sizeof(result), "Successfully unsubscribed: %s", cleanEmail);
            } else {
                strcpy(result, "Failed to unsubscribe");
            }
            sqlite3_finalize(stmt);
        } else {
            strcpy(result, "Database error");
        }
    } else {
        snprintf(result, sizeof(result), "Email not subscribed: %s", cleanEmail);
    }
    
    sqlite3_close(service->db);
    return result;
}

void addSubscriber(Lse143* service, const char* email) {
    int rc = sqlite3_open(service->db_path, &service->db);
    if (rc) return;
    
    char cleanEmail[MAX_EMAIL_LENGTH];
    strncpy(cleanEmail, email, MAX_EMAIL_LENGTH - 1);
    cleanEmail[MAX_EMAIL_LENGTH - 1] = '\\0';
    trim(cleanEmail);
    toLower(cleanEmail);
    
    sqlite3_stmt* stmt;
    const char* sql = "INSERT OR IGNORE INTO subscribers (email) VALUES (?)";
    
    rc = sqlite3_prepare_v2(service->db, sql, -1, &stmt, NULL);
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, cleanEmail, -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
    sqlite3_close(service->db);
}

int main() {
    Lse143 service;
    strcpy(service.db_path, "users.db");
    initDatabase(&service);
    
    printf("=== Test Case 1: Unsubscribe existing user ===\\n");
    addSubscriber(&service, "user1@example.com");
    printf("%s\\n", unsubscribeUser(&service, "user1@example.com"));
    
    printf("\\n=== Test Case 2: Unsubscribe non-existing user ===\\n");
    printf("%s\\n", unsubscribeUser(&service, "nonexistent@example.com"));
    
    printf("\\n=== Test Case 3: Invalid email format ===\\n");
    printf("%s\\n", unsubscribeUser(&service, "invalid-email"));
    
    printf("\\n=== Test Case 4: Empty email ===\\n");
    printf("%s\\n", unsubscribeUser(&service, ""));
    
    printf("\\n=== Test Case 5: Null email ===\\n");
    printf("%s\\n", unsubscribeUser(&service, NULL));
    
    return 0;
}
