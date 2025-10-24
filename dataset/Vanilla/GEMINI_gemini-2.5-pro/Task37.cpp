/*
NOTE: This code requires the MySQL C++ Connector.
You must install the connector and link against it during compilation.
For example, on Linux:
g++ -o task37 your_code.cpp -lmysqlcppconn

DATABASE SETUP:
Before running, you must have a MySQL server running and execute the following SQL:

CREATE DATABASE IF NOT EXISTS testdb;
USE testdb;
CREATE TABLE IF NOT EXISTS users (
    id INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(50) NOT NULL UNIQUE,
    email VARCHAR(100) NOT NULL,
    full_name VARCHAR(100)
);
INSERT INTO users (username, email, full_name) VALUES
('alice', 'alice@example.com', 'Alice Smith'),
('bob', 'bob@example.com', 'Bob Johnson'),
('charlie', 'charlie@example.com', 'Charlie Brown'),
('diana', 'diana@example.com', 'Diana Prince')
ON DUPLICATE KEY UPDATE email=VALUES(email), full_name=VALUES(full_name);

*/
#include <iostream>
#include <string>
#include <memory>
#include <stdexcept>

// MySQL Connector/C++ specific headers
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/exception.h>

// Database connection details - CHANGE THESE
const std::string DB_HOST = "tcp://127.0.0.1:3306";
const std::string DB_USER = "user";
const std::string DB_PASS = "password";
const std::string DB_NAME = "testdb";

/**
 * Connects to the database and retrieves user information.
 * @param username The username to search for.
 * @return A string containing user information or a "not found" message.
 */
std::string getUserInfo(const std::string& username) {
    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        std::unique_ptr<sql::Connection> con(driver->connect(DB_HOST, DB_USER, DB_PASS));
        con->setSchema(DB_NAME);

        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("SELECT username, email, full_name FROM users WHERE username = ?"));
        pstmt->setString(1, username);

        std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

        if (res->next()) {
            std::string result = "User Found:\n";
            result += "  Username: " + res->getString("username") + "\n";
            result += "  Email: " + res->getString("email") + "\n";
            result += "  Full Name: " + res->getString("full_name");
            return result;
        } else {
            return "User '" + username + "' not found.";
        }
    } catch (sql::SQLException &e) {
        std::string error_msg = "SQL Error: ";
        error_msg += e.what();
        error_msg += " (MySQL error code: " + std::to_string(e.getErrorCode());
        error_msg += ", SQLState: " + e.getSQLState() + ")";
        return error_msg;
    } catch (const std::runtime_error& e) {
        return std::string("Runtime Error: ") + e.what();
    }
}

int main() {
    // 5 Test Cases
    std::string testUsernames[] = {"alice", "bob", "eve", "charlie", "diana"};

    for (int i = 0; i < 5; ++i) {
        std::cout << "\n--- Test Case " << (i + 1) << " ---" << std::endl;
        std::cout << "Searching for user: " << testUsernames[i] << std::endl;
        std::string userInfo = getUserInfo(testUsernames[i]);
        std::cout << userInfo << std::endl;
    }

    return 0;
}