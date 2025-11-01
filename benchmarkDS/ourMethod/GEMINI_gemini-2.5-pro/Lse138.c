#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <microhttpd.h>
#include <errno.h>

/*
 * Note: This example requires libmicrohttpd and libsqlite3.
 * On Debian/Ubuntu, install with: sudo apt-get install libmicrohttpd-dev libsqlite3-dev
 * Compile with: gcc -o Lse138 Lse138.c -lmicrohttpd -lsqlite3
 */

#define PORT 8888
const char* DB_NAME = "user_database.db";

void setup_database() {
    sqlite3* db;
    char* zErrMsg = NULL;
    int rc;

    // Rule #7: In a real app, this path should come from a config file/env var.
    rc = sqlite3_open(DB_NAME, &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    const char* dropSql = "DROP TABLE IF EXISTS users;";
    sqlite3_exec(db, dropSql, 0, 0, &zErrMsg);

    const char* sql = "CREATE TABLE IF NOT EXISTS users("
                      "id INTEGER PRIMARY KEY, "
                      "username TEXT NOT NULL, "
                      "is_moderator INTEGER NOT NULL);";
    rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        sqlite3_close(db);
        exit(1);
    }

    const char* insertSql = "INSERT INTO users (id, username, is_moderator) VALUES "
                            "(1, 'Alice', 1), "
                            "(2, 'Bob', 0);";
    rc = sqlite3_exec(db, insertSql, 0, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        sqlite3_close(db);
        exit(1);
    }
    
    printf("Database initialized successfully.\n");
    sqlite3_close(db);
}

static int answer_to_connection(void* cls, struct MHD_Connection* connection,
                                const char* url, const char* method,
                                const char* version, const char* upload_data,
                                size_t* upload_data_size, void** con_cls) {
    if (0 != strcmp(method, "GET")) {
        return MHD_NO; // Only GET is allowed
    }

    struct MHD_Response* response;
    int ret;
    int http_status = 500; // Default to internal server error
    char response_buffer[256];
    memset(response_buffer, 0, sizeof(response_buffer));


    const char* user_id_str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "user_id");

    if (user_id_str == NULL) {
        snprintf(response_buffer, sizeof(response_buffer), "{\"error\": \"user_id parameter is required\"}");
        http_status = 400;
    } else {
        // Rule #1: Input validation
        char* endptr;
        errno = 0;
        long user_id_long = strtol(user_id_str, &endptr, 10);
        
        if (errno != 0 || *endptr != '\0' || user_id_long > 2147483647 || user_id_long < -2147483648) {
            snprintf(response_buffer, sizeof(response_buffer), "{\"error\": \"Invalid user_id format, must be an integer\"}");
            http_status = 400;
        } else {
            int user_id = (int)user_id_long;
            sqlite3* db;
            sqlite3_stmt* stmt = NULL;
            int found = 0;

            if (sqlite3_open(DB_NAME, &db) != SQLITE_OK) {
                fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
                snprintf(response_buffer, sizeof(response_buffer), "{\"error\": \"Internal server error\"}");
                http_status = 500;
            } else {
                // Rule #2: Use prepared statements
                const char* sql = "SELECT is_moderator FROM users WHERE id = ?";
                if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
                    sqlite3_bind_int(stmt, 1, user_id);

                    int rc = sqlite3_step(stmt);
                    if (rc == SQLITE_ROW) {
                        found = 1;
                        int is_mod = sqlite3_column_int(stmt, 0);
                        // Rule #3: Use boundary checks (snprintf)
                        snprintf(response_buffer, sizeof(response_buffer),
                                 "{\"user_id\": %d, \"is_moderator\": %s}", user_id, is_mod ? "true" : "false");
                        http_status = 200;
                    } else if (rc == SQLITE_DONE) {
                        found = 0;
                        snprintf(response_buffer, sizeof(response_buffer), "{\"error\": \"User with id %d not found\"}", user_id);
                        http_status = 404;
                    } else {
                        fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
                        snprintf(response_buffer, sizeof(response_buffer), "{\"error\": \"Internal server error\"}");
                        http_status = 500;
                    }
                } else {
                    fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
                    snprintf(response_buffer, sizeof(response_buffer), "{\"error\": \"Internal server error\"}");
                    http_status = 500;
                }
                sqlite3_finalize(stmt);
                sqlite3_close(db);
            }
        }
    }
    
    response = MHD_create_response_from_buffer(strlen(response_buffer),
                                               (void*)response_buffer, MHD_RESPMEM_MUST_COPY);
    MHD_add_response_header(response, "Content-Type", "application/json");
    ret = MHD_queue_response(connection, http_status, response);
    MHD_destroy_response(response);

    return ret;
}

int main() {
    setup_database();
    
    struct MHD_Daemon* daemon;
    
    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
                              &answer_to_connection, NULL, MHD_OPTION_END);
    if (NULL == daemon) {
        fprintf(stderr, "Failed to start server\n");
        return 1;
    }
    
    printf("Server running on port %d\n", PORT);
    printf("\n--- Test Cases ---\n");
    printf("1. Moderator user: curl \"http://localhost:8888/is_moderator?user_id=1\"\n");
    printf("2. Non-moderator user: curl \"http://localhost:8888/is_moderator?user_id=2\"\n");
    printf("3. Non-existent user: curl \"http://localhost:8888/is_moderator?user_id=99\"\n");
    printf("4. Invalid input: curl \"http://localhost:8888/is_moderator?user_id=abc\"\n");
    printf("5. Missing input: curl \"http://localhost:8888/is_moderator\"\n");

    printf("\nPress Enter to stop the server.\n");
    (void)getchar();
    
    MHD_stop_daemon(daemon);
    printf("Server stopped.\n");
    return 0;
}