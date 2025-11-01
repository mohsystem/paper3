
#include <iostream>
#include <mysql/mysql.h>
#include <string>
#include <cstdlib>

class Lse87 {
public:
    static double* temperatureForLocation(double latitude, double longitude, const std::string& date) {
        MYSQL* conn;
        MYSQL_STMT* stmt;
        MYSQL_BIND bind[3];
        MYSQL_BIND result_bind[1];
        double temperature;
        bool is_null;
        double* result = nullptr;
        
        conn = mysql_init(nullptr);
        
        if (!mysql_real_connect(conn, "localhost", "root", "password", "weather_db", 0, nullptr, 0)) {
            std::cerr << "Connection failed: " << mysql_error(conn) << std::endl;
            return nullptr;
        }
        
        const char* query = "SELECT temperature FROM weather_data WHERE latitude = ? AND longitude = ? AND date = ?";
        stmt = mysql_stmt_init(conn);
        
        if (mysql_stmt_prepare(stmt, query, strlen(query))) {
            std::cerr << "Prepare failed: " << mysql_stmt_error(stmt) << std::endl;
            mysql_close(conn);
            return nullptr;
        }
        
        memset(bind, 0, sizeof(bind));
        
        bind[0].buffer_type = MYSQL_TYPE_DOUBLE;
        bind[0].buffer = &latitude;
        
        bind[1].buffer_type = MYSQL_TYPE_DOUBLE;
        bind[1].buffer = &longitude;
        
        bind[2].buffer_type = MYSQL_TYPE_STRING;
        bind[2].buffer = (char*)date.c_str();
        bind[2].buffer_length = date.length();
        
        mysql_stmt_bind_param(stmt, bind);
        
        if (mysql_stmt_execute(stmt)) {
            std::cerr << "Execute failed: " << mysql_stmt_error(stmt) << std::endl;
            mysql_stmt_close(stmt);
            mysql_close(conn);
            return nullptr;
        }
        
        memset(result_bind, 0, sizeof(result_bind));
        
        result_bind[0].buffer_type = MYSQL_TYPE_DOUBLE;
        result_bind[0].buffer = &temperature;
        result_bind[0].is_null = &is_null;
        
        mysql_stmt_bind_result(stmt, result_bind);
        
        if (mysql_stmt_fetch(stmt) == 0 && !is_null) {
            result = new double(temperature);
        }
        
        mysql_stmt_close(stmt);
        mysql_close(conn);
        
        return result;
    }
};

int main() {
    // Test case 1
    double* temp1 = Lse87::temperatureForLocation(40.7128, -74.0060, "2024-01-15");
    std::cout << "Test 1 - Temperature: " << (temp1 ? *temp1 : -999) << std::endl;
    delete temp1;
    
    // Test case 2
    double* temp2 = Lse87::temperatureForLocation(34.0522, -118.2437, "2024-02-20");
    std::cout << "Test 2 - Temperature: " << (temp2 ? *temp2 : -999) << std::endl;
    delete temp2;
    
    // Test case 3
    double* temp3 = Lse87::temperatureForLocation(51.5074, -0.1278, "2024-03-10");
    std::cout << "Test 3 - Temperature: " << (temp3 ? *temp3 : -999) << std::endl;
    delete temp3;
    
    // Test case 4
    double* temp4 = Lse87::temperatureForLocation(35.6762, 139.6503, "2024-04-05");
    std::cout << "Test 4 - Temperature: " << (temp4 ? *temp4 : -999) << std::endl;
    delete temp4;
    
    // Test case 5
    double* temp5 = Lse87::temperatureForLocation(-33.8688, 151.2093, "2024-05-12");
    std::cout << "Test 5 - Temperature: " << (temp5 ? *temp5 : -999) << std::endl;
    delete temp5;
    
    return 0;
}
