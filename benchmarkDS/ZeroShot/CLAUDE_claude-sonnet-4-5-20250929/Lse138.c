
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include <microhttpd.h>

#define PORT 8080
#define DB_HOST "localhost"
#define DB_USER "root"
#define DB_PASSWORD "password"
#define DB_NAME "webapp"

typedef struct {
    int is_moderator;
} CheckResult;

int checkIfModerator(const char* username) {
    if (username == NULL || strlen(username) == 0) {
        return 0;
    }
    
    MYSQL* conn = mysql_init(NULL);
    if (conn == NULL) {
        return 0;
    }
    
    if (mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASSWORD, 
                          DB_NAME, 0, NULL, 0) == NULL) {
        mysql_close(conn);
        return 0;
    }
    
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    const char* query = "SELECT is_moderator FROM users WHERE username = ?";
    
    if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0) {
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return 0;
    }
    
    MYSQL_BIND bind[1];
    memset(bind, 0, sizeof(bind));
    
    unsigned long username_length = strlen(username);
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char*)username;
    bind[0].buffer_length = username_length;
    bind[0].length = &username_length;
    
    mysql_stmt_bind_param(stmt, bind);
    mysql_stmt_execute(stmt);
    
    int is_moderator = 0;
    MYSQL_BIND result_bind[1];
    memset(result_bind, 0, sizeof(result_bind));
    
    result_bind[0].buffer_type = MYSQL_TYPE_LONG;
    result_bind[0].buffer = (char*)&is_moderator;
    
    mysql_stmt_bind_result(stmt, result_bind);
    
    if (mysql_stmt_fetch(stmt) != 0) {
        is_moderator = 0;
    }
    
    mysql_stmt_close(stmt);
    mysql_close(conn);
    
    return is_moderator;
}

static int answer_to_connection(void* cls, struct MHD_Connection* connection,
                                const char* url, const char* method,
                                const char* version, const char* upload_data,
                                size_t* upload_data_size, void** con_cls) {
    
    if (strcmp(method, "GET") != 0) {
        return MHD_NO;
    }
    
    const char* username = MHD_lookup_connection_value(connection, 
                                                       MHD_GET_ARGUMENT_KIND, 
                                                       "username");
    
    int isModerator = checkIfModerator(username ? username : "");
    
    char response[256];
    snprintf(response, sizeof(response), "{\\"isModerator\\": %s}", 
             isModerator ? "true" : "false");
    
    struct MHD_Response* mhd_response = MHD_create_response_from_buffer(
        strlen(response), (void*)response, MHD_RESPMEM_MUST_COPY);
    
    MHD_add_response_header(mhd_response, "Content-Type", "application/json");
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, mhd_response);
    MHD_destroy_response(mhd_response);
    
    return ret;
}

int main() {
    // Test cases
    const char* testUsers[] = {"admin", "moderator1", "user1", "testmod", "normaluser"};
    
    printf("Test Cases:\\n");
    for (int i = 0; i < 5; i++) {
        int result = checkIfModerator(testUsers[i]);
        printf("User: %s - Is Moderator: %d\\n", testUsers[i], result);
    }
    
    // Start web server
    struct MHD_Daemon* daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, 
                                                 PORT, NULL, NULL,
                                                 &answer_to_connection, NULL,
                                                 MHD_OPTION_END);
    
    if (daemon == NULL) {
        return 1;
    }
    
    printf("Server running on http://localhost:%d/checkModerator\\n", PORT);
    printf("Press ENTER to exit.\\n");
    getchar();
    
    MHD_stop_daemon(daemon);
    return 0;
}
