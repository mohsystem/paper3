
#include <iostream>
#include <string>
#include <mysql/mysql.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>

class Lse138 {
private:
    std::string db_host = "tcp://127.0.0.1:3306";
    std::string db_user = "root";
    std::string db_password = "password";
    std::string db_name = "webapp_db";
    
public:
    bool checkIfModerator(const std::string& userId) {
        bool result = false;
        
        try {
            sql::Driver* driver = get_driver_instance();
            sql::Connection* conn = driver->connect(db_host, db_user, db_password);
            conn->setSchema(db_name);
            
            sql::PreparedStatement* pstmt = conn->prepareStatement(
                "SELECT is_moderator FROM users WHERE user_id = ?"
            );
            pstmt->setString(1, userId);
            
            sql::ResultSet* rs = pstmt->executeQuery();
            
            if (rs->next()) {
                result = rs->getBoolean("is_moderator");
            }
            
            delete rs;
            delete pstmt;
            delete conn;
            
        } catch (sql::SQLException& e) {
            std::cerr << "SQL Error: " << e.what() << std::endl;
        }
        
        return result;
    }
};

int main() {
    Lse138 app;
    
    std::cout << "Test Case 1: " << app.checkIfModerator("1") << std::endl;
    std::cout << "Test Case 2: " << app.checkIfModerator("2") << std::endl;
    std::cout << "Test Case 3: " << app.checkIfModerator("3") << std::endl;
    std::cout << "Test Case 4: " << app.checkIfModerator("4") << std::endl;
    std::cout << "Test Case 5: " << app.checkIfModerator("5") << std::endl;
    
    return 0;
}
