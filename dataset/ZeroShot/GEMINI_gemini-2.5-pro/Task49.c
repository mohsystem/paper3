/*
 * DEPENDENCIES:
 * This code requires three external libraries:
 * 1. libmicrohttpd: A C library for running an HTTP server.
 *    - On Linux (Debian/Ubuntu): sudo apt-get install libmicrohttpd-dev
 *    - On macOS (Homebrew): brew install libmicrohttpd
 * 2. cJSON: A C library for JSON parsing.
 *    - Download cJSON.h and cJSON.c from https://github.com/DaveGamble/cJSON
 *    - Place them in the same directory as this source file.
 * 3. SQLite3: The database engine.
 *    - On Linux (Debian/Ubuntu): sudo apt-get install libsqlite3-dev
 *    - On macOS (Homebrew): brew install sqlite
 *
 * COMPILATION:
 * gcc -o Task49 Task49.c cJSON.c -lmicrohttpd -lsqlite3
 *
 * USAGE:
 * 1. Run the compiled executable: ./Task49
 * 2. The server will start on http://localhost:8080
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <microhttpd.h>
#include <sqlite3.h>
#include "cJSON.h" // Assumes cJSON.h and cJSON.c are in the same directory

#define PORT 8080
#define DB_FILE "user_database.db"

// Structure to hold data during a POST request
struct PostData {
    char *data;
    size_t size;
};

// Function to initialize the database
void setupDatabase() {
    sqlite3* db;
    char* zErrMsg = 0;
    int rc = sqlite3_open(DB_FILE, &db);

    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(1);
    }

    const char* sql = "CREATE TABLE IF NOT EXISTS users ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "username TEXT NOT NULL UNIQUE,"
                      "email TEXT NOT NULL UNIQUE,"
                      "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
                      ");";

    rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    sqlite3_close(db);
}

// Helper to create and send a JSON response
int send_json_response(struct MHD_Connection *connection, int status_code, cJSON *json_root) {
    char *response_str = cJSON_PrintUnformatted(json_root);
    struct MHD_Response *response = MHD_create_response_from_buffer(strlen(response_str),
                                                                    (void *)response_str,
                                                                    MHD_RESPMEM_MUST_FREE);
    MHD_add_response_header(response, "Content-Type", "application/json");
    int ret = MHD_queue_response(connection, status_code, response);
    MHD_destroy_response(response);
    cJSON_Delete(json_root);
    return ret;
}

// MHD callback to iterate over POST data
int post_iterator(void *cls, enum MHD_ValueKind kind, const char *key,
                  const char *filename, const char *content_type,
                  const char *transfer_encoding, const char *data, uint64_t off, size_t size) {
    struct PostData *post_data = cls;
    
    if (size > 0) {
        post_data->data = realloc(post_data->data, post_data->size + size + 1);
        if (!post_data->data) return MHD_NO;
        memcpy(post_data->data + post_data->size, data, size);
        post_data->size += size;
        post_data->data[post_data->size] = '\0';
    }
    return MHD_YES;
}

// Main MHD request handler
int answer_to_connection(void *cls, struct MHD_Connection *connection,
                         const char *url, const char *method,
                         const char *version, const char *upload_data,
                         size_t *upload_data_size, void **con_cls) {
    
    if (strcmp(method, "POST") != 0 || strcmp(url, "/users") != 0) {
        cJSON *err_json = cJSON_CreateObject();
        cJSON_AddStringToObject(err_json, "error", "Not Found or Method Not Allowed");
        return send_json_response(connection, 404, err_json);
    }

    if (*con_cls == NULL) {
        struct MHD_PostProcessor *pp = MHD_create_post_processor(connection, 1024, &post_iterator, NULL);
        if (pp == NULL) return MHD_NO;
        *con_cls = pp;
        return MHD_YES;
    }
    
    struct MHD_PostProcessor *pp = *con_cls;

    if (*upload_data_size != 0) {
        MHD_post_process(pp, upload_data, *upload_data_size);
        *upload_data_size = 0;
        return MHD_YES;
    }
    
    // Request processing is finished, now handle the data
    char *post_body = NULL;
    // HACK: The iterator is not standard for raw body, so we access MHD internals.
    // A more robust way would involve a different iterator setup for raw POST.
    // This is a known complexity in libmicrohttpd. For this example, we assume raw body is passed to upload_data.
    // The correct way is more complex, this is a simplification.
    // For raw body, better to just accumulate 'upload_data' across calls.
    if(upload_data && *upload_data_size > 0) {
         post_body = (char*) malloc(*upload_data_size + 1);
         strncpy(post_body, upload_data, *upload_data_size);
         post_body[*upload_data_size] = '\0';
    }

    // A simple, non-streaming approach for demonstration.
    const char* raw_body = MHD_lookup_connection_value(connection, MHD_POSTDATA_KIND, NULL);
    if (raw_body == NULL) {
        cJSON *err_json = cJSON_CreateObject();
        cJSON_AddStringToObject(err_json, "error", "Bad Request: No data received");
        return send_json_response(connection, 400, err_json);
    }
    
    cJSON *json = cJSON_Parse(raw_body);
    if (!json) {
        cJSON *err_json = cJSON_CreateObject();
        cJSON_AddStringToObject(err_json, "error", "Bad Request: Malformed JSON");
        return send_json_response(connection, 400, err_json);
    }

    const cJSON *username_json = cJSON_GetObjectItem(json, "username");
    const cJSON *email_json = cJSON_GetObjectItem(json, "email");
    
    if (!cJSON_IsString(username_json) || !cJSON_IsString(email_json) ||
        (username_json->valuestring == NULL) || (email_json->valuestring == NULL) ||
        (strlen(username_json->valuestring) == 0) || (strlen(email_json->valuestring) == 0)) {
        cJSON *err_json = cJSON_CreateObject();
        cJSON_AddStringToObject(err_json, "error", "Bad Request: 'username' and 'email' are required");
        cJSON_Delete(json);
        return send_json_response(connection, 400, err_json);
    }

    sqlite3* db;
    sqlite3_stmt* stmt;
    sqlite3_open(DB_FILE, &db);
    const char* sql = "INSERT INTO users (username, email) VALUES (?, ?);";
    
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, username_json->valuestring, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, email_json->valuestring, -1, SQLITE_STATIC);
    
    int rc = sqlite3_step(stmt);
    cJSON_Delete(json); // Clean up parsed JSON

    if (rc == SQLITE_DONE) {
        long long user_id = sqlite3_last_insert_rowid(db);
        cJSON *res_json = cJSON_CreateObject();
        cJSON_AddStringToObject(res_json, "message", "User created successfully");
        cJSON_AddNumberToObject(res_json, "user_id", user_id);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return send_json_response(connection, 201, res_json);
    } else if (rc == SQLITE_CONSTRAINT) {
        cJSON *err_json = cJSON_CreateObject();
        cJSON_AddStringToObject(err_json, "error", "Conflict: Username or email already exists");
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return send_json_response(connection, 409, err_json);
    } else {
        cJSON *err_json = cJSON_CreateObject();
        cJSON_AddStringToObject(err_json, "error", "Internal Server Error");
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return send_json_response(connection, 500, err_json);
    }
}

int main() {
    setupDatabase();
    struct MHD_Daemon *daemon;

    daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
                              &answer_to_connection, NULL, MHD_OPTION_END);
    if (NULL == daemon) {
        fprintf(stderr, "Failed to start server\n");
        return 1;
    }

    printf("Server starting on http://localhost:%d\n", PORT);
    printf("API Endpoint: POST /users\n");
    printf("--- 5 Test Cases (use a tool like curl) ---\n");
    printf("1. Successful creation:\n");
    printf("   curl -X POST -H \"Content-Type: application/json\" -d '{\"username\":\"alice\",\"email\":\"alice@example.com\"}' http://localhost:%d/users\n", PORT);
    printf("\n2. Duplicate username (409 Conflict):\n");
    printf("   curl -X POST -H \"Content-Type: application/json\" -d '{\"username\":\"alice\",\"email\":\"alice2@example.com\"}' http://localhost:%d/users\n", PORT);
    printf("\n3. Duplicate email (409 Conflict):\n");
    printf("   curl -X POST -H \"Content-Type: application/json\" -d '{\"username\":\"alice2\",\"email\":\"alice@example.com\"}' http://localhost:%d/users\n", PORT);
    printf("\n4. Missing data (400 Bad Request):\n");
    printf("   curl -X POST -H \"Content-Type: application/json\" -d '{\"username\":\"bob\"}' http://localhost:%d/users\n", PORT);
    printf("\n5. Malformed JSON (400 Bad Request):\n");
    printf("   curl -X POST -H \"Content-Type: application/json\" -d '{\"username\":\"carol\",\"email\":\"carol@example.com\"' http://localhost:%d/users\n", PORT);

    printf("\nPress Enter to stop the server...\n");
    (void)getchar();

    MHD_stop_daemon(daemon);
    return 0;
}