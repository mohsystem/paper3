
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <mysql/mysql.h>
#include <microhttpd.h>

/* Security: Never hard-code credentials. Read from environment variables or secret manager. */
/* Rules#3: Avoid hard-coded passwords or secrets */

/* Validates string contains only allowed characters */
/* Rules#2: Input validation */
static int validate_alphanumeric_dot_hyphen(const char* str) {
    if (str == NULL) return 0;
    while (*str) {
        if (!isalnum(*str) && *str != '.' && *str != '-') {
            return 0;
        }
        str++;
    }
    return 1;
}

static int validate_alphanumeric_underscore(const char* str) {
    if (str == NULL) return 0;
    while (*str) {
        if (!isalnum(*str) && *str != '_') {
            return 0;
        }
        str++;
    }
    return 1;
}

/* Returns database server name from environment variable with validation */
/* Caller must free returned string */
/* Rules#8: Check all allocation results, validate bounds */
char* getServerName(void) {
    const char* server = getenv("DB_SERVER");
    if (server == NULL || strlen(server) == 0) {
        fprintf(stderr, "DB_SERVER environment variable not set\\n");
        return NULL;
    }
    
    size_t len = strlen(server);
    if (len > 253) {
        fprintf(stderr, "Server name too long\\n");
        return NULL;
    }
    
    /* Validate server name format */
    if (!validate_alphanumeric_dot_hyphen(server)) {
        fprintf(stderr, "Invalid server name format\\n");
        return NULL;
    }
    
    /* Rules#8: Check allocation result */
    char* result = malloc(len + 1);
    if (result == NULL) {
        fprintf(stderr, "Memory allocation failed\\n");
        return NULL;
    }
    
    /* Rules#8: Bound copy and ensure null termination */
    strncpy(result, server, len);
    result[len] = '\\0';
    
    return result;
}

/* Returns database name from environment variable with validation */
/* Caller must free returned string */
char* getDatabaseName(void) {
    const char* dbName = getenv("DB_NAME");
    if (dbName == NULL || strlen(dbName) == 0) {
        fprintf(stderr, "DB_NAME environment variable not set\\n");
        return NULL;
    }
    
    size_t len = strlen(dbName);
    if (len > 64) {
        fprintf(stderr, "Database name too long\\n");
        return NULL;
    }
    
    /* Validate database name format */
    if (!validate_alphanumeric_underscore(dbName)) {
        fprintf(stderr, "Invalid database name format\\n");
        return NULL;
    }
    
    char* result = malloc(len + 1);
    if (result == NULL) {
        fprintf(stderr, "Memory allocation failed\\n");
        return NULL;
    }
    
    strncpy(result, dbName, len);
    result[len] = '\\0';
    
    return result;
}

/* Returns database user from environment variable with validation */
/* Caller must free returned string */
char* getDatabaseUser(void) {
    const char* user = getenv("DB_USER");
    if (user == NULL || strlen(user) == 0) {
        fprintf(stderr, "DB_USER environment variable not set\\n");
        return NULL;
    }
    
    size_t len = strlen(user);
    if (len > 32) {
        fprintf(stderr, "Username too long\\n");
        return NULL;
    }
    
    /* Validate username format */
    if (!validate_alphanumeric_underscore(user)) {
        fprintf(stderr, "Invalid username format\\n");
        return NULL;
    }
    
    char* result = malloc(len + 1);
    if (result == NULL) {
        fprintf(stderr, "Memory allocation failed\\n");
        return NULL;
    }
    
    strncpy(result, user, len);
    result[len] = '\\0';
    
    return result;
}

/* Returns database password from environment variable */
/* Security: Never log or expose passwords (Rules#10) */
/* Caller must free returned string and clear it with memset_s or explicit_bzero */
char* getDatabasePassword(void) {
    const char* password = getenv("DB_PASSWORD");
    if (password == NULL || strlen(password) == 0) {
        fprintf(stderr, "DB_PASSWORD environment variable not set\\n");
        return NULL;
    }
    
    size_t len = strlen(password);
    if (len > 128) {
        fprintf(stderr, "Password too long\\n");
        return NULL;
    }
    
    char* result = malloc(len + 1);
    if (result == NULL) {
        fprintf(stderr, "Memory allocation failed\\n");
        return NULL;
    }
    
    strncpy(result, password, len);
    result[len] = '\\0';
    
    return result;
}

/* Returns a database connection using validated configuration */
/* Rules#7: Cleanup pattern with single failure label */
/* Rules#8: Check all return values */
MYSQL* getDatabaseConnection(void) {
    MYSQL* conn = NULL;
    char* server = NULL;
    char* dbName = NULL;
    char* user = NULL;
    char* password = NULL;
    
    /* Get configuration with validation */
    server = getServerName();
    if (server == NULL) goto cleanup;
    
    dbName = getDatabaseName();
    if (dbName == NULL) goto cleanup;
    
    user = getDatabaseUser();
    if (user == NULL) goto cleanup;
    
    password = getDatabasePassword();
    if (password == NULL) goto cleanup;
    
    /* Initialize MySQL connection */
    conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "MySQL initialization failed\\n");
        goto cleanup;
    }
    
    /* Rules#6: Enforce SSL/TLS for database connections */
    mysql_ssl_set(conn, NULL, NULL, NULL, NULL, NULL);
    unsigned int ssl_mode = SSL_MODE_REQUIRED;
    mysql_options(conn, MYSQL_OPT_SSL_MODE, &ssl_mode);
    
    /* Establish connection */
    if (mysql_real_connect(conn, server, user, password, dbName, 
                          3306, NULL, CLIENT_SSL) == NULL) {
        /* Rules#11: Generic error message */
        fprintf(stderr, "Database connection failed\\n");
        mysql_close(conn);
        conn = NULL;
        goto cleanup;
    }
    
cleanup:
    /* Rules#8: Free exactly once */
    /* Rules#9: Clear sensitive data before free */
    if (password != NULL) {
        memset(password, 0, strlen(password));
        free(password);
    }
    if (user != NULL) {
        free(user);
    }
    if (dbName != NULL) {
        free(dbName);
    }
    if (server != NULL) {
        free(server);
    }
    
    return conn;
}

/* Web app request handler */
static int requestHandler(void* cls, struct MHD_Connection* connection,
                         const char* url, const char* method,
                         const char* version, const char* upload_data,
                         size_t* upload_data_size, void** con_cls) {
    /* Rules#2: Validate HTTP method */
    if (strcmp(method, "GET") != 0) {
        return MHD_NO;
    }
    
    const char* response = "Web application running";
    struct MHD_Response* mhd_response = MHD_create_response_from_buffer(
        strlen(response), (void*)response, MHD_RESPMEM_PERSISTENT);
    
    if (mhd_response == NULL) {
        return MHD_NO;
    }
    
    MHD_add_response_header(mhd_response, "Content-Type", "text/plain; charset=utf-8");
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, mhd_response);
    MHD_destroy_response(mhd_response);
    
    return ret;
}

int main(void) {
    printf("=== Lse141 Web App Test Cases ===\\n\\n");
    
    /* Test Case 1: Missing environment variables */
    printf("Test 1: Missing environment variables\\n");
    char* server = getServerName();
    if (server == NULL) {
        printf("PASS: Correctly detected missing DB_SERVER\\n");
    } else {
        printf("FAIL: Should have returned NULL\\n");
        free(server);
    }
    
    /* Test Case 2: Valid configuration check */
    printf("\\nTest 2: Valid configuration check\\n");
    printf("Note: Set environment variables DB_SERVER, DB_NAME, DB_USER, DB_PASSWORD before running\\n");
    
    /* Test Case 3: Invalid server name format */
    printf("\\nTest 3: Server name validation\\n");
    setenv("DB_SERVER", "invalid;server", 1);
    server = getServerName();
    if (server == NULL) {
        printf("PASS: Server name validation works\\n");
    } else {
        printf("FAIL: Should have rejected invalid server name\\n");
        free(server);
    }
    unsetenv("DB_SERVER");
    
    /* Test Case 4: Start web server */
    printf("\\nTest 4: Starting web server\\n");
    struct MHD_Daemon* daemon = MHD_start_daemon(
        MHD_USE_SELECT_INTERNALLY, 8080, NULL, NULL,
        &requestHandler, NULL, MHD_OPTION_END);
    
    if (daemon != NULL) {
        printf("PASS: Web server created on port 8080\\n");
        MHD_stop_daemon(daemon);
    } else {
        printf("FAIL: Could not create web server\\n");
    }
    
    /* Test Case 5: Database connection (requires valid env vars) */
    printf("\\nTest 5: Database connection\\n");
    MYSQL* conn = getDatabaseConnection();
    if (conn != NULL) {
        printf("PASS: Database connection successful\\n");
        mysql_close(conn);
    } else {
        printf("INFO: Database connection requires valid environment variables\\n");
    }
    
    printf("\\n=== Tests Complete ===\\n");
    
    return 0;
}
