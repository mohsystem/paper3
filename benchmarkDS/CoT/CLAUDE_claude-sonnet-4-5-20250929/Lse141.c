
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Returns server name from environment variable
const char* getServerName() {
    const char* server = getenv("DB_SERVER");
    return server ? server : "localhost";
}

// Returns database name from environment variable
const char* getDatabaseName() {
    const char* dbname = getenv("DB_NAME");
    return dbname ? dbname : "defaultdb";
}

// Returns database user from environment variable
const char* getDatabaseUser() {
    const char* user = getenv("DB_USER");
    return user ? user : "root";
}

// Private function - Returns database password from environment variable
static const char* getDatabasePassword() {
    const char* password = getenv("DB_PASSWORD");
    return password ? password : "";
}

// Simulates database connection with security checks
int getDatabaseConnection() {
    const char* server = getServerName();
    const char* dbname = getDatabaseName();
    const char* user = getDatabaseUser();
    const char* password = getDatabasePassword();
    
    if (!server || !dbname || !user || !password || 
        strlen(server) == 0 || strlen(dbname) == 0 || 
        strlen(user) == 0 || strlen(password) == 0) {
        fprintf(stderr, "Error: Database configuration not properly set in environment variables\\n");
        return 0;
    }
    
    // In a real application, use proper database connector library
    // Example: MySQL C API, PostgreSQL libpq, SQLite, etc.
    printf("Attempting connection to: %s/%s\\n", server, dbname);
    return 1; // Simulated success
}

int main() {
    printf("Test Case 1: Get Server Name\\n");
    printf("Server: %s\\n", getServerName());
    
    printf("\\nTest Case 2: Get Database Name\\n");
    printf("Database: %s\\n", getDatabaseName());
    
    printf("\\nTest Case 3: Get Database User\\n");
    printf("User: %s\\n", getDatabaseUser());
    
    printf("\\nTest Case 4: Configuration Check\\n");
    int configSet = (getenv("DB_SERVER") && getenv("DB_NAME") && 
                     getenv("DB_USER") && getenv("DB_PASSWORD"));
    printf("All configuration set: %s\\n", configSet ? "true" : "false");
    
    printf("\\nTest Case 5: Database Connection Test\\n");
    int connected = getDatabaseConnection();
    printf("Connection status: %s\\n", connected ? "Success" : "Failed");
    
    return 0;
}
