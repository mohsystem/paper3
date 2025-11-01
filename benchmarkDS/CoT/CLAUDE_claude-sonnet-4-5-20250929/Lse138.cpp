
#include <iostream>
#include <string>
#include <cstdlib>
#include <sqlite3.h>
#include <cstring>
#include <microhttpd.h>

class Lse138 {
private:
    static std::string getEnv(const char* name, const char* defaultValue) {
        const char* value = std::getenv(name);
        return value ? value : defaultValue;
    }

public:
    static bool checkIfModerator(const std::string& username) {
        if (username.empty()) {
            return false;
        }

        std::string dbUrl = getEnv("DB_URL", "test.db");
        sqlite3* db;
        sqlite3_stmt* stmt;
        bool isModerator = false;

        if (sqlite3_open(dbUrl.c_str(), &db) != SQLITE_OK) {
            std::cerr << "Database error occurred" << std::endl;
            return false;
        }

        // Use parameterized query to prevent SQL injection
        const char* query = "SELECT is_moderator FROM users WHERE username = ?";
        
        if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
            
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                isModerator = sqlite3_column_int(stmt, 0) == 1;
            }
            
            sqlite3_finalize(stmt);
        }
        
        sqlite3_close(db);
        return isModerator;
    }

    static int handleRequest(void* cls, struct MHD_Connection* connection,
                           const char* url, const char* method,
                           const char* version, const char* upload_data,
                           size_t* upload_data_size, void** con_cls) {
        if (std::strcmp(url, "/check-moderator") == 0) {
            const char* username = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "username");
            bool isModerator = checkIfModerator(username ? username : "");
            
            std::string response = "{\\"is_moderator\\": " + std::string(isModerator ? "true" : "false") + "}";
            struct MHD_Response* mhdResponse = MHD_create_response_from_buffer(
                response.length(), (void*)response.c_str(), MHD_RESPMEM_MUST_COPY);
            
            MHD_add_response_header(mhdResponse, "Content-Type", "application/json");
            int ret = MHD_queue_response(connection, MHD_HTTP_OK, mhdResponse);
            MHD_destroy_response(mhdResponse);
            return ret;
        }
        return MHD_NO;
    }

    static void initTestDatabase() {
        std::string dbUrl = getEnv("DB_URL", "test.db");
        sqlite3* db;
        
        if (sqlite3_open(dbUrl.c_str(), &db) == SQLITE_OK) {
            sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS users (username TEXT PRIMARY KEY, is_moderator BOOLEAN)", nullptr, nullptr, nullptr);
            sqlite3_exec(db, "DELETE FROM users", nullptr, nullptr, nullptr);
            sqlite3_exec(db, "INSERT INTO users VALUES ('admin', 1)", nullptr, nullptr, nullptr);
            sqlite3_exec(db, "INSERT INTO users VALUES ('user1', 0)", nullptr, nullptr, nullptr);
            sqlite3_exec(db, "INSERT INTO users VALUES ('moderator1', 1)", nullptr, nullptr, nullptr);
            sqlite3_exec(db, "INSERT INTO users VALUES ('user2', 0)", nullptr, nullptr, nullptr);
            sqlite3_close(db);
        }
    }
};

int main() {
    Lse138::initTestDatabase();
    
    // Test cases
    std::cout << "Test Case 1: " << Lse138::checkIfModerator("admin") << std::endl;
    std::cout << "Test Case 2: " << Lse138::checkIfModerator("user1") << std::endl;
    std::cout << "Test Case 3: " << Lse138::checkIfModerator("moderator1") << std::endl;
    std::cout << "Test Case 4: " << Lse138::checkIfModerator("") << std::endl;
    std::cout << "Test Case 5: " << Lse138::checkIfModerator("nonexistent") << std::endl;
    
    // Note: Web server requires libmicrohttpd library
    // Uncomment below to start server (requires linking with -lmicrohttpd)
    /*
    struct MHD_Daemon* daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, 8000, 
                                                  nullptr, nullptr, 
                                                  &Lse138::handleRequest, nullptr, 
                                                  MHD_OPTION_END);
    if (daemon) {
        std::cout << "Server started on port 8000" << std::endl;
        getchar();
        MHD_stop_daemon(daemon);
    }
    */
    
    return 0;
}
