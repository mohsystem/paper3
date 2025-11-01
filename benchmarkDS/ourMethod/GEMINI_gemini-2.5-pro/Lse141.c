#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// To run this example, you need to set the following environment variables:
// export DB_HOST="your_db_host"
// export DB_NAME="your_db_name"
// export DB_USER="your_db_user"
// export DB_PASSWORD="your_db_password"

// Mock structure for a database connection. In a real application,
// this would be a type from a database library (e.g., PGconn for libpq).
typedef struct {
    char* conn_info;
} DbConnection;

/**
 * @brief Simulates creating and running a web application.
 * In a real-world C application, a library like libmicrohttpd or a custom
 * socket-based server would be implemented here.
 */
void createWebApp() {
    printf("Web App Placeholder: Server is 'running'.\n");
}

/**
 * @brief Retrieves the server name from an environment variable.
 * @return A read-only string with the server name, or NULL if not set.
 */
const char* getServerName() {
    return getenv("DB_HOST");
}

/**
 * @brief Retrieves the database name from an environment variable.
 * @return A read-only string with the database name, or NULL if not set.
 */
const char* getDbName() {
    return getenv("DB_NAME");
}

/**
 * @brief Retrieves the database user from an environment variable.
 * @return A read-only string with the database user, or NULL if not set.
 */
const char* getDbUser() {
    return getenv("DB_USER");
}

/**
 * @brief Retrieves the database password from an environment variable.
 * @return A read-only string with the database password, or NULL if not set.
 */
const char* getDbPassword() {
    return getenv("DB_PASSWORD");
}

/**
 * @brief Securely clears a memory buffer.
 * A volatile pointer is used to prevent the compiler from optimizing away the memory write.
 * This is a fallback for when `memset_s` is not available.
 * @param v Pointer to the memory to clear.
 * @param c The character to fill with (should be 0 for clearing).
 * @param n The number of bytes to clear.
 */
static void secure_clear_memory(void* v, size_t n) {
    volatile unsigned char* p = v;
    while (n--) {
        *p++ = 0;
    }
}


/**
 * @brief Creates a database connection object using credentials from environment variables.
 * This function demonstrates secure construction of a connection string and handling of passwords.
 * @return A pointer to a mock DbConnection, or NULL on failure.
 */
DbConnection* getDbConnection() {
    const char* serverName = getServerName();
    const char* dbName = getDbName();
    const char* user = getDbUser();
    const char* password = getDbPassword();

    if (!serverName || !dbName || !user || !password) {
        fprintf(stderr, "Error: Database environment variables are not fully set.\n");
        return NULL;
    }

    // Securely build the connection string to prevent buffer overflows.
    // 1. Calculate the required size.
    int required_size = snprintf(NULL, 0, "host=%s dbname=%s user=%s password=%s",
                                 serverName, dbName, user, password);
    if (required_size < 0) {
        fprintf(stderr, "Error calculating connection string size.\n");
        return NULL;
    }
    
    size_t buffer_size = (size_t)required_size + 1;
    char* conn_str = malloc(buffer_size);
    if (!conn_str) {
        fprintf(stderr, "Error: Failed to allocate memory for connection string.\n");
        return NULL;
    }

    // 2. Format the string into the allocated buffer.
    int written = snprintf(conn_str, buffer_size, "host=%s dbname=%s user=%s password=%s",
                           serverName, dbName, user, password);
    if (written < 0 || (size_t)written >= buffer_size) {
        fprintf(stderr, "Error formatting connection string.\n");
        free(conn_str);
        return NULL;
    }
    
    // In a real application, the connection string (especially the password part)
    // should be cleared from memory as soon as it's no longer needed.
    // Example: libpq's PQconnectdb copies the string, so we can clear our copy.
    
    printf("Demonstration: Would attempt connection with a securely built string.\n");
    // e.g., PGconn* conn = PQconnectdb(conn_str);

    DbConnection* mock_conn = malloc(sizeof(DbConnection));
    if(!mock_conn) {
        fprintf(stderr, "Error: Failed to allocate memory for DbConnection.\n");
        secure_clear_memory(conn_str, buffer_size);
        free(conn_str);
        return NULL;
    }
    mock_conn->conn_info = conn_str; // Store for demonstration

    // After connecting, the connection string in memory should be cleared.
    secure_clear_memory(conn_str, buffer_size); 
    // The library might have its own copy; refer to its documentation for secure handling.
    // We will free it later when we destroy mock_conn.

    return mock_conn;
}

void destroyDbConnection(DbConnection* conn) {
    if (conn) {
        if (conn->conn_info) {
            // Ensure memory is cleared before freeing
            secure_clear_memory(conn->conn_info, strlen(conn->conn_info));
            free(conn->conn_info);
        }
        free(conn);
    }
}


void run_tests() {
    printf("--- Running 5 Test Cases ---\n");

    // Test Case 1: Get Server Name
    const char* server = getServerName();
    printf("1. Server Name: %s\n", server ? server : "Not Set");

    // Test Case 2: Get DB Name
    const char* db = getDbName();
    printf("2. Database Name: %s\n", db ? db : "Not Set");

    // Test Case 3: Get DB User
    const char* user = getDbUser();
    printf("3. Database User: %s\n", user ? user : "Not Set");

    // Test Case 4: Check for DB Password
    const char* password = getDbPassword();
    printf("4. Database Password: %s\n", (password && password[0] != '\0') ? "Set" : "Not Set");

    // Test Case 5: Attempt to get a DB connection object
    printf("5. Attempting to get DB Connection object...\n");
    DbConnection* conn = getDbConnection();
    if (conn) {
        printf("   Successfully created connection object.\n");
        destroyDbConnection(conn);
    } else {
        printf("   Failed to create connection object.\n");
    }

    printf("--- Test Cases Finished ---\n");
}

int main() {
    createWebApp();
    printf("\n");
    run_tests();
    return 0;
}