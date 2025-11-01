
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <regex.h>

/* Maximum email length to prevent buffer overflow */
#define MAX_EMAIL_LENGTH 254
#define MAX_PATH_LENGTH 4096

/* Email validation pattern (RFC 5322 simplified) */
static const char* EMAIL_PATTERN = 
    "^[a-zA-Z0-9_+&*-]+([.][a-zA-Z0-9_+&*-]+)*@([a-zA-Z0-9-]+[.])+[a-zA-Z]{2,7}$";

/**
 * Validates email format and length
 * 
 * @param email The email to validate
 * @return 1 if valid, 0 otherwise
 */
static int validate_email(const char* email) {
    regex_t regex;
    int result;
    
    /* Check for NULL pointer */
    if (email == NULL) {
        return 0;
    }
    
    /* Check length to prevent buffer overflow */
    size_t len = strnlen(email, MAX_EMAIL_LENGTH + 1);
    if (len == 0 || len > MAX_EMAIL_LENGTH) {
        return 0;
    }
    
    /* Compile and execute regex */
    result = regcomp(&regex, EMAIL_PATTERN, REG_EXTENDED | REG_NOSUB);
    if (result != 0) {
        return 0;
    }
    
    result = regexec(&regex, email, 0, NULL, 0);
    regfree(&regex);
    
    return (result == 0) ? 1 : 0;
}

/**
 * Validates path to prevent path traversal
 * 
 * @param path The path to validate
 * @return 1 if valid, 0 otherwise
 */
static int validate_path(const char* path) {
    /* Check for NULL pointer */
    if (path == NULL) {
        return 0;
    }
    
    /* Check length */
    size_t len = strnlen(path, MAX_PATH_LENGTH + 1);
    if (len == 0 || len > MAX_PATH_LENGTH) {
        return 0;
    }
    
    /* Check for path traversal attempts */
    if (strstr(path, "..") != NULL || strstr(path, "~") != NULL) {
        return 0;
    }
    
    return 1;
}

/**
 * Calls the unsubscribe script with validated email parameter
 * 
 * @param email Validated email address
 * @param script_path Path to unsubscribe script
 * @return 1 if script returns 0, otherwise 0
 */
static int call_unsubscribe_script(const char* email, const char* script_path) {
    pid_t pid;
    int status;
    
    /* Validate inputs */
    if (email == NULL || script_path == NULL) {
        return 0;
    }
    
    pid = fork();
    
    if (pid == -1) {
        /* Fork failed */
        fprintf(stderr, "Process creation failed\\n");
        return 0;
    }
    
    if (pid == 0) {
        /* Child process */
        /* Clear environment to prevent information leakage */
        clearenv();
        
        /* Execute script with email as argument (prevents command injection) */
        char* args[] = {(char*)script_path, (char*)email, NULL};
        execv(script_path, args);
        
        /* If execv returns, it failed */
        _exit(1);
    } else {
        /* Parent process */
        /* Wait for child process */
        pid_t result = waitpid(pid, &status, 0);
        
        if (result == -1) {
            fprintf(stderr, "Wait failed\\n");
            return 0;
        }
        
        /* Check if process exited normally */
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            /* Return 1 if script returns 0 (success), otherwise 0 */
            return (exit_code == 0) ? 1 : 0;
        }
        
        return 0;
    }
}

/**
 * Checks if email exists in database and unsubscribes if found.
 * Returns 1 if unsubscribe successful, 0 otherwise.
 * 
 * @param email The email address to unsubscribe
 * @param db_path Path to SQLite database
 * @param script_path Path to unsubscribe script
 * @return 1 if successful, 0 otherwise
 */
int unsubscribe_email(const char* email, const char* db_path, 
                      const char* script_path) {
    sqlite3* db = NULL;
    sqlite3_stmt* stmt = NULL;
    int rc;
    int result = 0;
    
    /* Input validation */
    if (!validate_email(email)) {
        return 0;
    }
    
    /* Validate paths */
    if (!validate_path(db_path) || !validate_path(script_path)) {
        return 0;
    }
    
    /* Open database connection */
    rc = sqlite3_open_v2(db_path, &db, SQLITE_OPEN_READONLY, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Database error occurred\\n");
        if (db != NULL) {
            sqlite3_close(db);
        }
        return 0;
    }
    
    /* Prepare parameterized query to prevent SQL injection */
    const char* query = "SELECT COUNT(*) FROM subscribers WHERE email = ?";
    rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Query preparation failed\\n");
        sqlite3_close(db);
        return 0;
    }
    
    /* Bind email parameter safely */
    rc = sqlite3_bind_text(stmt, 1, email, -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Parameter binding failed\\n");
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 0;
    }
    
    /* Execute query */
    rc = sqlite3_step(stmt);
    
    if (rc == SQLITE_ROW) {
        int count = sqlite3_column_int(stmt, 0);
        
        if (count > 0) {
            /* Email exists, call unsubscribe script */
            result = call_unsubscribe_script(email, script_path);
        }
    }
    
    /* Clean up resources */
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return result;
}

int main(void) {
    /* Test cases with safe mock values */
    const char* db_path = ":memory:";
    const char* script_path = "/usr/local/bin/unsubscribe.sh";
    
    /* Test case 1: Valid email format */
    printf("Test 1 (valid email): %d\\n", 
           unsubscribe_email("user@example.com", db_path, script_path));
    
    /* Test case 2: NULL email */
    printf("Test 2 (NULL email): %d\\n", 
           unsubscribe_email(NULL, db_path, script_path));
    
    /* Test case 3: Empty email */
    printf("Test 3 (empty email): %d\\n", 
           unsubscribe_email("", db_path, script_path));
    
    /* Test case 4: Invalid email format (SQL injection attempt) */
    printf("Test 4 (injection attempt): %d\\n", 
           unsubscribe_email("' OR '1'='1", db_path, script_path));
    
    /* Test case 5: Email too long */
    char long_email[400];
    memset(long_email, 'a', 300);
    strcpy(long_email + 300, "@example.com");
    printf("Test 5 (too long): %d\\n", 
           unsubscribe_email(long_email, db_path, script_path));
    
    return 0;
}
