#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <mysql/mysql.h>
#include <regex.h>

// Rule #1: Validate username format: 3-20 characters, alphanumeric and underscores.
// Returns true if valid, false otherwise.
bool is_username_valid(const char* username) {
    if (username == NULL) return false;
    regex_t regex;
    int reti;
    bool is_match = false;

    // Compile regex
    reti = regcomp(&regex, "^[a-zA-Z0-9_]{3,20}$", REG_EXTENDED);
    if (reti) {
        fprintf(stderr, "Could not compile regex\n");
        return false;
    }

    // Execute regex
    reti = regexec(&regex, username, 0, NULL, 0);
    if (!reti) {
        is_match = true;
    } else if (reti != REG_NOMATCH) {
        char msgbuf[100];
        regerror(reti, &regex, msgbuf, sizeof(msgbuf));
        fprintf(stderr, "Regex match failed: %s\n", msgbuf);
    }
    
    regfree(&regex);
    return is_match;
}


/**
 * Connects to a MySQL database and checks if a username exists.
 * This function uses environment variables for database credentials.
 * export DB_HOST="localhost"
 * export DB_USER="youruser"
 * export DB_PASS="yourpassword"
 * export DB_NAME="testdb"
 *
 * @param username The username to check.
 * @return true if the username exists, false otherwise.
 */
bool username_exists(const char* username) {
    // Rule #1: Ensure all input is validated.
    if (!is_username_valid(username)) {
        fprintf(stderr, "Invalid username format.\n");
        return false;
    }

    // Rule #5 & #6: Use environment variables for credentials.
    const char* db_host = getenv("DB_HOST");
    const char* db_user = getenv("DB_USER");
    const char* db_pass = getenv("DB_PASS");
    const char* db_name = getenv("DB_NAME");

    if (!db_host || !db_user || !db_pass || !db_name) {
        fprintf(stderr, "Database environment variables (DB_HOST, DB_USER, DB_PASS, DB_NAME) are not set.\n");
        return false;
    }

    MYSQL* conn = NULL;
    MYSQL_STMT* stmt = NULL;
    bool exists = false;

    conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        return false;
    }

    if (mysql_real_connect(conn, db_host, db_user, db_pass, db_name, 0, NULL, 0) == NULL) {
        fprintf(stderr, "mysql_real_connect() failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        return false;
    }
    
    stmt = mysql_stmt_init(conn);
    if (!stmt) {
        fprintf(stderr, "mysql_stmt_init() failed\n");
        mysql_close(conn);
        return false;
    }

    // Using a prepared statement to prevent SQL injection (CWE-89).
    const char* query = "SELECT username FROM users WHERE username = ? LIMIT 1";
    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, "mysql_stmt_prepare() failed: %s\n", mysql_stmt_error(stmt));
        goto cleanup;
    }

    MYSQL_BIND bind[1];
    memset(bind, 0, sizeof(bind));
    unsigned long username_len = strlen(username);

    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char*)username;
    bind[0].buffer_length = username_len;

    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, "mysql_stmt_bind_param() failed: %s\n", mysql_stmt_error(stmt));
        goto cleanup;
    }

    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, "mysql_stmt_execute() failed: %s\n", mysql_stmt_error(stmt));
        goto cleanup;
    }

    if (mysql_stmt_store_result(stmt)) {
         fprintf(stderr, "mysql_stmt_store_result() failed: %s\n", mysql_stmt_error(stmt));
         goto cleanup;
    }

    if (mysql_stmt_num_rows(stmt) > 0) {
        exists = true;
    }

cleanup:
    if (stmt) mysql_stmt_close(stmt);
    if (conn) mysql_close(conn);
    return exists;
}

// Helper to set up the database.
void setup_database_for_testing() {
    const char* db_host = getenv("DB_HOST");
    const char* db_user = getenv("DB_USER");
    const char* db_pass = getenv("DB_PASS");
    const char* db_name = getenv("DB_NAME");

    if (!db_host || !db_user || !db_pass || !db_name) return;

    MYSQL* conn = mysql_init(NULL);
    if (!conn) return;

    if (mysql_real_connect(conn, db_host, db_user, db_pass, db_name, 0, NULL, 0)) {
        const char* createTableSQL = "CREATE TABLE IF NOT EXISTS users (id INT AUTO_INCREMENT PRIMARY KEY, username VARCHAR(255) NOT NULL UNIQUE);";
        const char* insertSQL = "INSERT IGNORE INTO users (username) VALUES ('alice'), ('bob');";
        mysql_query(conn, createTableSQL);
        mysql_query(conn, insertSQL);
        printf("Database setup for testing complete.\n");
    }
    mysql_close(conn);
}


int main() {
    printf("--- Running C Username Existence Check ---\n");
    printf("This test requires a MySQL database and the following environment variables:\n");
    printf("export DB_HOST=\"localhost\"\n");
    printf("export DB_USER=\"youruser\"\n");
    printf("export DB_PASS=\"yourpassword\"\n");
    printf("export DB_NAME=\"testdb\"\n");
    printf("Compile with: gcc your_file.c -o your_app $(mysql_config --cflags --libs)\n");
    printf("-------------------------------------------------\n");
    
    setup_database_for_testing();

    const char* test_usernames[] = {
        "alice",        // Test case 1: Existing user
        "charlie",      // Test case 2: Non-existent user
        "dave_has_a_very_long_name_that_is_invalid", // Test case 3: Invalid (too long)
        "eve-vil",      // Test case 4: Invalid (contains '-')
        ""              // Test case 5: Invalid (empty)
    };
    
    for (int i = 0; i < 5; ++i) {
        const char* username = test_usernames[i];
        printf("Test Case %d: Checking for username '%s'...\n", i + 1, username);
        bool exists = username_exists(username);
        printf("Result: Username '%s' %s.\n\n", username, exists ? "exists" : "does not exist or is invalid");
    }

    return 0;
}