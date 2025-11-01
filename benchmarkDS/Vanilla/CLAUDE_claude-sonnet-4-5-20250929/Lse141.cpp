
#include <iostream>
#include <string>
#include <mysql/mysql.h>

class Lse141 {
private:
    static const std::string SERVER_NAME;
    static const std::string DATABASE_NAME;
    static const std::string DATABASE_USER;
    static const std::string DATABASE_PASSWORD;

public:
    // Function that returns a database connection
    static MYSQL* getDatabaseConnection() {
        MYSQL* conn = mysql_init(NULL);
        
        if (conn == NULL) {
            std::cerr << "MySQL initialization failed" << std::endl;
            return NULL;
        }
        
        if (mysql_real_connect(conn, getServerName().c_str(), 
                              getDatabaseUser().c_str(),
                              getDatabasePassword().c_str(),
                              getDatabaseName().c_str(), 
                              0, NULL, 0) == NULL) {
            std::cerr << "Database connection failed: " << mysql_error(conn) << std::endl;
            mysql_close(conn);
            return NULL;
        }
        
        return conn;
    }
    
    // Function that returns the server name
    static std::string getServerName() {
        return SERVER_NAME;
    }
    
    // Function that returns the database name
    static std::string getDatabaseName() {
        return DATABASE_NAME;
    }
    
    // Function that returns the database user
    static std::string getDatabaseUser() {
        return DATABASE_USER;
    }
    
    // Function that returns the database password
    static std::string getDatabasePassword() {
        return DATABASE_PASSWORD;
    }
};

const std::string Lse141::SERVER_NAME = "localhost";
const std::string Lse141::DATABASE_NAME = "myDatabase";
const std::string Lse141::DATABASE_USER = "root";
const std::string Lse141::DATABASE_PASSWORD = "password123";

int main() {
    // Test case 1: Get server name
    std::cout << "Test 1 - Server Name: " << Lse141::getServerName() << std::endl;
    
    // Test case 2: Get database name
    std::cout << "Test 2 - Database Name: " << Lse141::getDatabaseName() << std::endl;
    
    // Test case 3: Get database user
    std::cout << "Test 3 - Database User: " << Lse141::getDatabaseUser() << std::endl;
    
    // Test case 4: Get database password
    std::cout << "Test 4 - Database Password: " << Lse141::getDatabasePassword() << std::endl;
    
    // Test case 5: Get database connection
    MYSQL* conn = Lse141::getDatabaseConnection();
    if (conn != NULL) {
        std::cout << "Test 5 - Database Connection: Successful" << std::endl;
        mysql_close(conn);
    } else {
        std::cout << "Test 5 - Database Connection: Failed" << std::endl;
    }
    
    return 0;
}
