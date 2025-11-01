
#include <iostream>
#include <string>
#include <sqlite3.h>
#include <regex>
#include <cmath>

class Lse85 {
private:
    static const std::string DB_PATH;
    
    static bool isValidLatitude(double lat) {
        return lat >= -90.0 && lat <= 90.0;
    }
    
    static bool isValidLongitude(double lon) {
        return lon >= -180.0 && lon <= 180.0;
    }
    
    static bool isValidDate(const std::string& date) {
        std::regex datePattern("^\\\\d{4}-\\\\d{2}-\\\\d{2}$");
        if (!std::regex_match(date, datePattern)) {
            return false;
        }
        
        int year, month, day;
        if (sscanf(date.c_str(), "%d-%d-%d", &year, &month, &day) != 3) {
            return false;
        }
        
        if (month < 1 || month > 12 || day < 1 || day > 31) {
            return false;
        }
        
        return true;
    }
    
public:
    static double temperatureForLocation(double latitude, double longitude, const std::string& date) {
        if (!isValidLatitude(latitude) || !isValidLongitude(longitude) || !isValidDate(date)) {
            return -999.0; // Error value
        }
        
        sqlite3* db;
        sqlite3_stmt* stmt;
        double temperature = -999.0;
        
        if (sqlite3_open(DB_PATH.c_str(), &db) != SQLITE_OK) {
            std::cerr << "Database error occurred" << std::endl;
            return -999.0;
        }
        
        const char* query = "SELECT temperature FROM sensor_data WHERE latitude = ? AND longitude = ? AND date = ?";
        
        if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_double(stmt, 1, latitude);
            sqlite3_bind_double(stmt, 2, longitude);
            sqlite3_bind_text(stmt, 3, date.c_str(), -1, SQLITE_STATIC);
            
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                temperature = sqlite3_column_double(stmt, 0);
            }
            
            sqlite3_finalize(stmt);
        }
        
        sqlite3_close(db);
        return temperature;
    }
};

const std::string Lse85::DB_PATH = "Sensor.db";

int main() {
    std::cout << "Test 1: " << Lse85::temperatureForLocation(40.7128, -74.0060, "2024-01-15") << std::endl;
    std::cout << "Test 2: " << Lse85::temperatureForLocation(51.5074, -0.1278, "2024-02-20") << std::endl;
    std::cout << "Test 3: " << Lse85::temperatureForLocation(35.6762, 139.6503, "2024-03-10") << std::endl;
    std::cout << "Test 4 (Invalid lat): " << Lse85::temperatureForLocation(100.0, -74.0060, "2024-01-15") << std::endl;
    std::cout << "Test 5 (Invalid date): " << Lse85::temperatureForLocation(40.7128, -74.0060, "2024-13-45") << std::endl;
    
    return 0;
}
