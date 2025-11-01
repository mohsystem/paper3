
#include <iostream>
#include <string>
#include <mysql/mysql.h>
#include <cstring>

class Lse141 {
private:
    static const std::string SERVER_NAME;
    static const std::string DATABASE_NAME;
    static const std::string DATABASE_USER;
    static const std::string DATABASE_PASSWORD;
    static const int PORT = 3306;

public:
    static MYSQL* getDatabaseConnection() {
        MYSQL* conn = mysql_init(nullptr);
        if (conn == nullptr) {
            throw std::runtime_error("MySQL initialization failed");
        }

        mysql_ssl_set(conn, nullptr, nullptr, nullptr, nullptr, nullptr);
        
        if (mysql_real_connect(conn, 
                              getServerName().c_str(),
                              getDatabaseUser().c_str(),
                              getDatabasePassword().c_str(),
                              getDatabaseName().c_str(),
                              PORT, nullptr, CLIENT_SSL) == nullptr) {
            std::string error = mysql_error(conn);
            mysql_close(conn);
            throw std::runtime_error("Connection failed: " + error);
        }
        
        return conn;
    }

    static std::string getServerName() {
        return SERVER_NAME;
    }

    static std::string getDatabaseName() {
        return DATABASE_NAME;
    }

    static std::string getDatabaseUser() {
        return DATABASE_USER;
    }

    static std::string getDatabasePassword() {
        return DATABASE_PASSWORD;
    }
};

const std::string Lse141::SERVER_NAME = "localhost";
const std::string Lse141::DATABASE_NAME = "myDatabase";
const std::string Lse141::DATABASE_USER = "dbUser";
const std::string Lse141::DATABASE_PASSWORD = "securePassword123";

int main() {
    // Test case 1: Get server name
    std::cout << "Test 1 - Server Name: " << Lse141::getServerName() << std::endl;

    // Test case 2: Get database name
    std::cout << "Test 2 - Database Name: " << Lse141::getDatabaseName() << std::endl;

    // Test case 3: Get database user
    std::cout << "Test 3 - Database User: " << Lse141::getDatabaseUser() << std::endl;

    // Test case 4: Get database password (masked for security)
    std::string masked(Lse141::getDatabasePassword().length(), '*');
    std::cout << "Test 4 - Database Password: " << masked << std::endl;

    // Test case 5: Attempt to get database connection
    try {
        MYSQL* conn = Lse141::getDatabaseConnection();
        if (conn != nullptr) {
            std::cout << "Test 5 - Database Connection: Successful" << std::endl;
            mysql_close(conn);
        }
    } catch (const std::exception& e) {
        std::cout << "Test 5 - Database Connection: Failed - " << e.what() << std::endl;
    }

    return 0;
}
