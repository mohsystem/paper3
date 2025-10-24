#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Third-party libraries are expected to be available.
// On Debian/Ubuntu: sudo apt-get install libmicrohttpd-dev libsqlite3-dev libcjson-dev
#include <microhttpd.h>
#include <sqlite3.h>
#include <cjson/cJSON.h>

#define PORT 8081
#define MAX_POST_SIZE 4096 // 4 KB limit for request body

// --- Database Setup ---
sqlite3* g_db = NULL;

int initialize_database() {
    if (sqlite3_open("users.db", &g_db)) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(g_db));
        return 1;
    }

    char* err_msg = NULL;
    const char* sql = "CREATE TABLE IF NOT EXISTS users("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "name TEXT NOT NULL, "
                      "email TEXT NOT NULL UNIQUE);";

    int rc = sqlite3_exec(g_db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(g_db);
        g_db = NULL;
        return 1;
    }
    return 0;
}

// --- Input Validation ---
bool is_valid_email(const char* email) {
    if (email == NULL) return false;
    size_t len = strlen(email);
    if (len == 0 || len > 254) return false;
    
    const char* at = strchr(email, '@');
    if (at == NULL || at == email || at == email + len - 1) return false;

    const char* dot = strrchr(at, '.');
    if (dot == NULL || dot == at + 1 || dot == email + len - 1) return false;
    
    return true;
}

// --- HTTP Server Logic ---
typedef struct {
    char* buffer;
    size_t size;
} PostData;

void request_completed(void* cls, struct MHD_Connection* connection,
                       void** con_cls, enum MHD_RequestTerminationCode toe) {
    PostData* post_data = *con_cls;
    if (post_data) {
        if (post_data->buffer) {
            free(post_data->buffer);
        }
        free(post_data);
    }
    *con_cls = NULL;
}

int send_response(struct MHD_Connection* connection, int status_code, const char* content) {
    struct MHD_Response* response = MHD_create_response_from_buffer(strlen(content), (void*)content, MHD_RESPMEM_MUST_COPY);
    if (!response) {
        return MHD_NO;
    }
    int ret = MHD_queue_response(connection, status_code, response);
    MHD_destroy_response(response);
    return ret;
}

int handle_post_request(struct MHD_Connection* connection, const char* data, size_t size) {
    cJSON* json = cJSON_ParseWithLength(data, size);
    if (json == NULL) {
        return send_response(connection, MHD_HTTP_BAD_REQUEST, "Invalid JSON format.");
    }

    const cJSON* name_json = cJSON_GetObjectItemCaseSensitive(json, "name");
    const cJSON* email_json = cJSON_GetObjectItemCaseSensitive(json, "email");

    if (!cJSON_IsString(name_json) || !cJSON_IsString(email_json) || (name_json->valuestring == NULL) || (email_json->valuestring == NULL)) {
        cJSON_Delete(json);
        return send_response(connection, MHD_HTTP_BAD_REQUEST, "Missing or invalid 'name' or 'email' fields.");
    }

    char* name = name_json->valuestring;
    char* email = email_json->valuestring;
    size_t name_len = strlen(name);

    if (name_len == 0 || name_len > 100) {
        cJSON_Delete(json);
        return send_response(connection, MHD_HTTP_BAD_REQUEST, "Name must be between 1 and 100 characters.");
    }
    if (!is_valid_email(email)) {
        cJSON_Delete(json);
        return send_response(connection, MHD_HTTP_BAD_REQUEST, "Invalid email format.");
    }

    sqlite3_stmt* stmt = NULL;
    const char* sql = "INSERT INTO users (name, email) VALUES (?, ?);";
    int rc = sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(g_db));
        cJSON_Delete(json);
        return send_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, "Database internal error.");
    }

    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, email, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    cJSON_Delete(json);

    if (rc == SQLITE_DONE) {
        return send_response(connection, MHD_HTTP_CREATED, "User created successfully.");
    } else if (rc == SQLITE_CONSTRAINT) {
        return send_response(connection, MHD_HTTP_CONFLICT, "Email already exists.");
    } else {
        fprintf(stderr, "Failed to create user: %s\n", sqlite3_errmsg(g_db));
        return send_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, "Failed to create user.");
    }
}

int answer_to_connection(void* cls, struct MHD_Connection* connection,
                         const char* url, const char* method,
                         const char* version, const char* upload_data,
                         size_t* upload_data_size, void** con_cls) {
    if (*con_cls == NULL) {
        PostData* post_data = malloc(sizeof(PostData));
        if (post_data == NULL) return MHD_NO;
        post_data->buffer = NULL;
        post_data->size = 0;
        *con_cls = post_data;
        return MHD_YES;
    }

    if (strcmp(method, "POST") != 0 || strcmp(url, "/api/users") != 0) {
        return send_response(connection, MHD_HTTP_NOT_FOUND, "Not Found.");
    }

    PostData* post_data = *con_cls;

    if (*upload_data_size > 0) {
        if (post_data->size + *upload_data_size > MAX_POST_SIZE) {
            return send_response(connection, MHD_HTTP_BAD_REQUEST, "Request body too large.");
        }
        char* new_buffer = realloc(post_data->buffer, post_data->size + *upload_data_size);
        if (new_buffer == NULL) {
            fprintf(stderr, "Out of memory\n");
            return MHD_NO;
        }
        post_data->buffer = new_buffer;
        memcpy(post_data->buffer + post_data->size, upload_data, *upload_data_size);
        post_data->size += *upload_data_size;
        *upload_data_size = 0;
        return MHD_YES;
    } else { // End of data
        return handle_post_request(connection, post_data->buffer, post_data->size);
    }
}


int main() {
    if (initialize_database() != 0) {
        return 1;
    }
    
    // NOTE: This is an HTTP server. For production use, you must use MHD_USE_SSL flag
    // and provide key/cert options via MHD_OPTION_SSL_CERT_MEM etc.
    struct MHD_Daemon* daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL,
                                                &answer_to_connection, NULL,
                                                MHD_OPTION_NOTIFY_COMPLETED, &request_completed, NULL,
                                                MHD_OPTION_END);

    if (NULL == daemon) {
        fprintf(stderr, "Failed to start server daemon.\n");
        sqlite3_close(g_db);
        return 1;
    }

    printf("Server running on http://localhost:%d\n", PORT);
    printf("You can use curl to test the endpoint.\n");
    printf("\n--- Test Cases ---\n");
    printf("1. Create a new user (Success):\n");
    printf("   curl -X POST -H \"Content-Type: application/json\" -d '{\"name\": \"John Doe\", \"email\": \"john.doe@example.com\"}' http://localhost:%d/api/users\n\n", PORT);
    printf("2. Attempt to create a duplicate user (Conflict):\n");
    printf("   curl -X POST -H \"Content-Type: application/json\" -d '{\"name\": \"John Doe\", \"email\": \"john.doe@example.com\"}' http://localhost:%d/api/users\n\n", PORT);
    printf("3. Send invalid JSON (Bad Request):\n");
    printf("   curl -X POST -H \"Content-Type: application/json\" -d '{\"name\": \"Jane Doe\", \"email\": \"jane.doe@example.com\"' http://localhost:%d/api/users\n\n", PORT);
    printf("4. Send data with missing fields (Bad Request):\n");
    printf("   curl -X POST -H \"Content-Type: application/json\" -d '{\"name\": \"Jane Doe\"}' http://localhost:%d/api/users\n\n", PORT);
    printf("5. Send data with invalid email format (Bad Request):\n");
    printf("   curl -X POST -H \"Content-Type: application/json\" -d '{\"name\": \"Invalid Email\", \"email\": \"invalid-email\"}' http://localhost:%d/api/users\n\n", PORT);

    printf("Press ENTER to stop the server\n");
    (void)getchar();

    MHD_stop_daemon(daemon);
    sqlite3_close(g_db);
    return 0;
}