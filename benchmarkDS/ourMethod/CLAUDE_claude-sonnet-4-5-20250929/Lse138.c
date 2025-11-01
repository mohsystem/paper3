
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <microhttpd.h>
#include <postgresql/libpq-fe.h>
#include <ctype.h>

#define MAX_USERNAME_LEN 50
#define PORT 8080

/* Database connection parameters from environment variables */
static const char* get_db_param(const char* env_var, const char* default_val) {
    const char* val = getenv(env_var);
    return val ? val : default_val;
}

/* Validates username: alphanumeric and underscore only, max 50 chars */
static int validate_username(const char* username) {
    size_t len;
    size_t i;
    
    if (!username) return 0;
    
    len = strlen(username);
    if (len == 0 || len > MAX_USERNAME_LEN) return 0;
    
    for (i = 0; i < len; i++) {
        if (!isalnum((unsigned char)username[i]) && username[i] != '_') {
            return 0;
        }
    }
    return 1;
}

/**
 * Checks if a user is a moderator by querying the database.
 * Uses parameterized query to prevent SQL injection.
 * 
 * @param username The username to check (already validated)
 * @return 1 if moderator, 0 if not, -1 on error
 */
static int check_moderator(const char* username) {
    PGconn* conn = NULL;
    PGresult* res = NULL;
    int result = -1;
    const char* db_host;
    const char* db_name;
    const char* db_user;
    const char* db_password;
    char conninfo[512];
    const char* sql;
    const char* params[1];
    const char* value;
    
    /* Validate input before database query */
    if (!validate_username(username)) {
        return 0;
    }
    
    /* Build connection string from environment variables */
    db_host = get_db_param("DB_HOST", "localhost");
    db_name = get_db_param("DB_NAME", "testdb");
    db_user = get_db_param("DB_USER", "testuser");
    db_password = get_db_param("DB_PASSWORD", "testpass");
    
    snprintf(conninfo, sizeof(conninfo), 
             "host=%s dbname=%s user=%s password=%s connect_timeout=5",
             db_host, db_name, db_user, db_password);
    
    /* Connect to database */
    conn = PQconnectdb(conninfo);
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Database connection failed\\n");
        goto cleanup;
    }
    
    /* Parameterized query prevents SQL injection */
    sql = "SELECT is_moderator FROM users WHERE username = $1";
    params[0] = username;
    
    res = PQexecParams(conn, sql, 1, NULL, params, NULL, NULL, 0);
    
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Database query failed\\n");
        goto cleanup;
    }
    
    if (PQntuples(res) > 0) {
        value = PQgetvalue(res, 0, 0);
        result = (value && (value[0] == 't' || value[0] == '1')) ? 1 : 0;
    } else {
        result = 0; /* User not found */
    }
    
cleanup:
    if (res) PQclear(res);
    if (conn) PQfinish(conn);
    return result;
}

/* HTTP request handler */
static enum MHD_Result handle_request(void* cls, struct MHD_Connection* connection,
                                     const char* url, const char* method,
                                     const char* version, const char* upload_data,
                                     size_t* upload_data_size, void** con_cls) {
    struct MHD_Response* response;
    enum MHD_Result ret;
    const char* username;
    int is_moderator;
    const char* response_text;
    int status_code;
    
    /* Only accept GET requests */
    if (strcmp(method, "GET") != 0) {
        response_text = "Method not allowed";
        status_code = MHD_HTTP_METHOD_NOT_ALLOWED;
        goto send_response;
    }
    
    /* Check route */
    if (strcmp(url, "/check-moderator") != 0) {
        response_text = "Not found";
        status_code = MHD_HTTP_NOT_FOUND;
        goto send_response;
    }
    
    /* Extract username parameter */
    username = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "username");
    
    if (!username || !validate_username(username)) {
        response_text = "Invalid request";
        status_code = MHD_HTTP_BAD_REQUEST;
        goto send_response;
    }
    
    /* Check moderator status */
    is_moderator = check_moderator(username);
    
    if (is_moderator < 0) {
        response_text = "Service unavailable";
        status_code = MHD_HTTP_SERVICE_UNAVAILABLE;
    } else if (is_moderator > 0) {
        response_text = "User is moderator";
        status_code = MHD_HTTP_OK;
    } else {
        response_text = "User is not moderator";
        status_code = MHD_HTTP_OK;
    }
    
send_response:
    response = MHD_create_response_from_buffer(strlen(response_text),
                                               (void*)response_text,
                                               MHD_RESPMEM_PERSISTENT);
    ret = MHD_queue_response(connection, status_code, response);
    MHD_destroy_response(response);
    return ret;
}

static void test_check_moderator(const char* username) {
    int result;
    printf("Testing username: %s\\n", username);
    result = check_moderator(username);
    if (result < 0) {
        printf("Result: Database error\\n");
    } else {
        printf("Result: %s\\n", result ? "Moderator" : "Not moderator");
    }
}

int main(void) {
    struct MHD_Daemon* daemon;
    
    /* Run test cases */
    test_check_moderator("admin");
    test_check_moderator("user123");
    test_check_moderator("moderator1");
    test_check_moderator("guest");
    test_check_moderator("invalid-user!");
    
    /* Start web server */
    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
                             &handle_request, NULL, MHD_OPTION_END);
    
    if (!daemon) {
        fprintf(stderr, "Failed to start server\\n");
        return 1;
    }
    
    printf("Server started on port %d\\n", PORT);
    getchar(); /* Wait for user input to stop */
    
    MHD_stop_daemon(daemon);
    return 0;
}
