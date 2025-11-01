
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <ctype.h>

#define MAX_USERNAME_LENGTH 50
#define MAX_MESSAGE_LENGTH 500
#define MAX_HTML_LENGTH 10000
#define DB_PATH "messages.db"

/* Secure string copy with null termination guarantee */
static int safe_strncpy(char* dest, const char* src, size_t dest_size) {
    if (!dest || !src || dest_size == 0) {
        return -1;
    }
    size_t i;
    for (i = 0; i < dest_size - 1 && src[i] != '\\0'; i++) {
        dest[i] = src[i];
    }
    dest[i] = '\\0';
    return 0;
}

/* Trim whitespace from string */
static void trim_string(char* str) {
    if (!str) return;
    
    char* start = str;
    while (isspace((unsigned char)*start)) start++;
    
    if (*start == '\\0') {
        str[0] = '\\0';
        return;
    }
    
    char* end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) end--;
    
    size_t len = end - start + 1;
    if (start != str) {
        memmove(str, start, len);
    }
    str[len] = '\\0';
}

/* HTML encode to prevent XSS attacks */
static char* html_encode(const char* input) {
    if (!input) return NULL;
    
    size_t input_len = strlen(input);
    /* Allocate enough space for worst case (all special chars) */
    char* output = malloc(input_len * 6 + 1);
    if (!output) return NULL;
    
    size_t j = 0;
    for (size_t i = 0; i < input_len; i++) {
        switch (input[i]) {
            case '&':
                memcpy(output + j, "&amp;", 5);
                j += 5;
                break;
            case '<':
                memcpy(output + j, "&lt;", 4);
                j += 4;
                break;
            case '>':
                memcpy(output + j, "&gt;", 4);
                j += 4;
                break;
            case '"':
                memcpy(output + j, "&quot;", 6);
                j += 6;
                break;
            case '\\'':
                memcpy(output + j, "&#x27;", 6);
                j += 6;
                break;
            case '/':
                memcpy(output + j, "&#x2F;", 6);
                j += 6;
                break;
            default:
                output[j++] = input[i];
                break;
        }
    }
    output[j] = '\\0';
    return output;
}

/* Initialize database with proper schema */
static int init_database(void) {
    sqlite3* db = NULL;
    int rc = sqlite3_open(DB_PATH, &db);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Database open failed\\n");
        if (db) sqlite3_close(db);
        return 0;
    }
    
    const char* sql = 
        "CREATE TABLE IF NOT EXISTS messages ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "username TEXT NOT NULL, "
        "message TEXT NOT NULL, "
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP)";
    
    char* err_msg = NULL;
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Database initialization failed\\n");
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 0;
    }
    
    sqlite3_close(db);
    return 1;
}

/* Secure database insertion using parameterized queries to prevent SQL injection */
static int insert_user_message_in_db(const char* username, const char* message) {
    /* Validate inputs before database operation */
    if (!username || !message) {
        return 0;
    }
    
    /* Create mutable copies for trimming */
    char clean_username[MAX_USERNAME_LENGTH + 1];
    char clean_message[MAX_MESSAGE_LENGTH + 1];
    
    if (safe_strncpy(clean_username, username, sizeof(clean_username)) != 0) {
        return 0;
    }
    if (safe_strncpy(clean_message, message, sizeof(clean_message)) != 0) {
        return 0;
    }
    
    trim_string(clean_username);
    trim_string(clean_message);
    
    if (clean_username[0] == '\\0' || clean_message[0] == '\\0') {
        return 0;
    }
    
    if (strlen(clean_username) > MAX_USERNAME_LENGTH || 
        strlen(clean_message) > MAX_MESSAGE_LENGTH) {
        return 0;
    }
    
    sqlite3* db = NULL;
    int rc = sqlite3_open(DB_PATH, &db);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Database open failed\\n");
        if (db) sqlite3_close(db);
        return 0;
    }
    
    /* Use parameterized query to prevent SQL injection */
    const char* sql = "INSERT INTO messages (username, message) VALUES (?, ?)";
    sqlite3_stmt* stmt = NULL;
    
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Statement preparation failed\\n");
        sqlite3_close(db);
        return 0;
    }
    
    sqlite3_bind_text(stmt, 1, clean_username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, clean_message, -1, SQLITE_TRANSIENT);
    
    rc = sqlite3_step(stmt);
    int success = (rc == SQLITE_DONE);
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return success;
}

/* Generate HTML page with XSS protection */
static char* generate_html_page(void) {
    sqlite3* db = NULL;
    int rc = sqlite3_open(DB_PATH, &db);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Database open failed\\n");
        if (db) sqlite3_close(db);
        return NULL;
    }
    
    const char* sql = "SELECT username, message, created_at FROM messages "
                     "ORDER BY created_at DESC LIMIT 100";
    sqlite3_stmt* stmt = NULL;
    
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Statement preparation failed\\n");
        sqlite3_close(db);
        return NULL;
    }
    
    char* html = malloc(MAX_HTML_LENGTH);
    if (!html) {
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return NULL;
    }
    
    /* Initialize HTML with safe fixed content */
    const char* header = "<!DOCTYPE html><html><head><meta charset=\\"UTF-8\\">"
                        "<title>Messages</title></head><body>"
                        "<h1>Post a Message</h1>"
                        "<form method=\\"POST\\" action=\\"/post\\">"
                        "Username: <input type=\\"text\\" name=\\"username\\" maxlength=\\"50\\" required><br>"
                        "Message: <textarea name=\\"message\\" maxlength=\\"500\\" required></textarea><br>"
                        "<input type=\\"submit\\" value=\\"Post\\"></form>"
                        "<h2>Messages:</h2><ul>";
    
    safe_strncpy(html, header, MAX_HTML_LENGTH);
    size_t current_len = strlen(html);
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* username = sqlite3_column_text(stmt, 0);
        const unsigned char* message = sqlite3_column_text(stmt, 1);
        const unsigned char* created_at = sqlite3_column_text(stmt, 2);
        
        if (username && message) {
            char* enc_username = html_encode((const char*)username);
            char* enc_message = html_encode((const char*)message);
            char* enc_time = created_at ? html_encode((const char*)created_at) : NULL;
            
            if (enc_username && enc_message) {
                char entry[2000];
                snprintf(entry, sizeof(entry), "<li><strong>%s</strong>: %s", 
                        enc_username, enc_message);
                
                if (enc_time) {
                    char time_part[200];
                    snprintf(time_part, sizeof(time_part), " <em>(%s)</em>", enc_time);
                    strncat(entry, time_part, sizeof(entry) - strlen(entry) - 1);
                }
                strncat(entry, "</li>", sizeof(entry) - strlen(entry) - 1);
                
                if (current_len + strlen(entry) < MAX_HTML_LENGTH - 100) {
                    strncat(html, entry, MAX_HTML_LENGTH - current_len - 1);
                    current_len += strlen(entry);
                }
            }
            
            free(enc_username);
            free(enc_message);
            free(enc_time);
        }
    }
    
    strncat(html, "</ul></body></html>", MAX_HTML_LENGTH - current_len - 1);
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return html;
}

int main(void) {
    if (!init_database()) {
        fprintf(stderr, "Failed to initialize database\\n");
        return 1;
    }
    
    /* Test cases */
    printf("Test 1: Valid input: %d\\n", insert_user_message_in_db("Alice", "Hello World"));
    printf("Test 2: Valid input: %d\\n", insert_user_message_in_db("Bob", "Test message"));
    printf("Test 3: NULL username: %d\\n", insert_user_message_in_db(NULL, "Message"));
    printf("Test 4: Empty message: %d\\n", insert_user_message_in_db("User", ""));
    
    char long_name[200];
    memset(long_name, 'a', sizeof(long_name) - 1);
    long_name[sizeof(long_name) - 1] = '\\0';
    printf("Test 5: Too long username: %d\\n", insert_user_message_in_db(long_name, "msg"));
    
    /* Generate and display HTML page */
    char* html_page = generate_html_page();
    if (html_page) {
        printf("\\nGenerated HTML Page:\\n%s\\n", html_page);
        free(html_page);
    }
    
    return 0;
}
