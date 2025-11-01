#include <iostream>
#include <string>
#include <cstdlib> // For std::getenv
#include <memory>

// To compile and run this code:
// 1. Download the single-header file "httplib.h" from https://github.com/yhirose/cpp-httplib
// 2. Place "httplib.h" in the same directory as this source file.
// 3. Compile: g++ -std=c++17 -o Lse141 Lse141.cpp -lpthread
// 4. Run: ./Lse141
#include "httplib.h"

// --- Database Configuration Functions ---

/**
 * @brief Securely retrieves the database server name from an environment variable.
 * @return The server name, or "localhost" if the environment variable is not set.
 */
std::string getServerName() {
    const char* server = std::getenv("DB_SERVER");
    return server ? std::string(server) : "localhost";
}

/**
 * @brief Securely retrieves the database name from an environment variable.
 * @return The database name, or "mydatabase" if the environment variable is not set.
 */
std::string getDatabaseName() {
    const char* dbName = std::getenv("DB_NAME");
    return dbName ? std::string(dbName) : "mydatabase";
}

/**
 * @brief Securely retrieves the database user from an environment variable.
 * @return The database user, or "admin" if the environment variable is not set.
 */
std::string getDatabaseUser() {
    const char* user = std::getenv("DB_USER");
    return user ? std::string(user) : "admin";
}

/**
 * @brief Securely retrieves the database password from an environment variable.
 * @return The database password, or a default insecure password if not set.
 */
std::string getDatabasePassword() {
    const char* password = std::getenv("DB_PASSWORD");
    // In a real application, you should throw an exception or exit if the password is not set.
    return password ? std::string(password) : "INSECURE_DEFAULT_PASSWORD";
}

/**
 * @brief Constructs a database connection string.
 * @return A formatted connection string with the password redacted.
 */
std::string getDatabaseConnection() {
    return "server=" + getServerName() +
           ";database=" + getDatabaseName() +
           ";user=" + getDatabaseUser() +
           ";password=[REDACTED]";
}

void runTestCases() {
    std::cout << "--- Running Test Cases ---" << std::endl;
    std::cout << "1. Server Name: " << getServerName() << std::endl;
    std::cout << "2. Database Name: " << getDatabaseName() << std::endl;
    std::cout << "3. Database User: " << getDatabaseUser() << std::endl;
    std::cout << "4. Database Password: " << getDatabasePassword() << std::endl;
    std::cout << "5. Full Connection String: " << getDatabaseConnection() << std::endl;
    std::cout << "--------------------------\n" << std::endl;
    std::cout << "To set these values, use environment variables like:" << std::endl;
    std::cout << "export DB_SERVER=db.example.com" << std::endl;
    std::cout << "export DB_NAME=prod_db" << std::endl;
    std::cout << "export DB_USER=myuser" << std::endl;
    std::cout << "export DB_PASSWORD='my_secret_password'\n" << std::endl;
}

int main() {
    // 1. Run Test Cases
    runTestCases();

    // 2. Setup and run web server
    httplib::Server svr;

    svr.Get("/config", [](const httplib::Request &, httplib::Response &res) {
        std::string html_body =
            "<html><body>"
            "<h1>Database Configuration</h1>"
            "<ul>"
            "<li><b>Server Name:</b> " + getServerName() + "</li>"
            "<li><b>Database Name:</b> " + getDatabaseName() + "</li>"
            "<li><b>User:</b> " + getDatabaseUser() + "</li>"
            "<li><b>Password:</b> [REDACTED FOR SECURITY]</li>"
            "<li><b>Connection String:</b> " + getDatabaseConnection() + "</li>"
            "</ul>"
            "</body></html>";
        res.set_content(html_body, "text/html");
    });
    
    int port = 8081;
    std::cout << "Server started on port " << port << std::endl;
    std::cout << "Visit http://localhost:8081/config to see the configuration." << std::endl;
    svr.listen("0.0.0.0", port);

    return 0;
}