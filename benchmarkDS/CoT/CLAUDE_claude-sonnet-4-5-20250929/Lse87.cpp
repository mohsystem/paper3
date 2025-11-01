
#include <iostream>
#include <string>
#include <sqlite3.h>
#include <cstring>
#include <regex>
#include <sstream>
#include <iomanip>

class Lse87 {
private:
    static const char* DB_PATH;
    
    static bool validateLatitude(double lat) {
        return lat >= -90.0 && lat <= 90.0;
    }
    
    static bool validateLongitude(double lon) {
        return lon >= -180.0 && lon <= 180.0;
    }
    
    static bool validateDate(const std::string& date) {
        std::regex datePattern("^\\\\d{4}-\\\\d{2}-\\\\d{2}$");
        return std::regex_match(date, datePattern);
    }
    
public:
    struct TemperatureResult {
        bool success;
        double temperature;
        std::string error;
        
        TemperatureResult(bool s, double t, const std::string& e) 
            : success(s), temperature(t), error(e) {}
    };
    
    static TemperatureResult temperatureForLocation(const char* latitudeStr, 
                                                     const char* longitudeStr, 
                                                     const char* dateStr) {
        // Null check
        if (!latitudeStr || !longitudeStr || !dateStr) {
            return TemperatureResult(false, 0.0, "Missing parameters");
        }
        
        // Parse and validate inputs
        double latitude, longitude;
        try {
            latitude = std::stod(latitudeStr);
            longitude = std::stod(longitudeStr);
        } catch (const std::exception& e) {
            return TemperatureResult(false, 0.0, "Invalid number format");
        }
        
        if (!validateLatitude(latitude)) {
            return TemperatureResult(false, 0.0, "Invalid latitude range");
        }
        if (!validateLongitude(longitude)) {
            return TemperatureResult(false, 0.0, "Invalid longitude range");
        }
        if (!validateDate(dateStr)) {
            return TemperatureResult(false, 0.0, "Invalid date format");
        }
        
        // Database operations
        sqlite3* db;
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_open(DB_PATH, &db);
        if (rc != SQLITE_OK) {
            return TemperatureResult(false, 0.0, "Database connection error");
        }
        
        const char* query = "SELECT temperature FROM weather_data WHERE latitude = ? AND longitude = ? AND date = ?";
        
        rc = sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            sqlite3_close(db);
            return TemperatureResult(false, 0.0, "Query preparation error");
        }
        
        // Bind parameters
        sqlite3_bind_double(stmt, 1, latitude);
        sqlite3_bind_double(stmt, 2, longitude);
        sqlite3_bind_text(stmt, 3, dateStr, -1, SQLITE_TRANSIENT);
        
        rc = sqlite3_step(stmt);
        
        TemperatureResult result(false, 0.0, "No data found");
        
        if (rc == SQLITE_ROW) {
            double temp = sqlite3_column_double(stmt, 0);
            result = TemperatureResult(true, temp, "");
        } else if (rc != SQLITE_DONE) {
            result = TemperatureResult(false, 0.0, "Database query error");
        }
        
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        return result;
    }
};

const char* Lse87::DB_PATH = "weather.db";

int main() {
    std::cout << "Test 1: Valid input\\n";
    auto result1 = Lse87::temperatureForLocation("40.7128", "-74.0060", "2024-01-15");
    std::cout << "Success: " << result1.success << ", Temp: " << result1.temperature 
              << ", Error: " << result1.error << "\\n\\n";
    
    std::cout << "Test 2: Invalid latitude\\n";
    auto result2 = Lse87::temperatureForLocation("100.0", "-74.0060", "2024-01-15");
    std::cout << "Success: " << result2.success << ", Error: " << result2.error << "\\n\\n";
    
    std::cout << "Test 3: Invalid date format\\n";
    auto result3 = Lse87::temperatureForLocation("40.7128", "-74.0060", "15-01-2024");
    std::cout << "Success: " << result3.success << ", Error: " << result3.error << "\\n\\n";
    
    std::cout << "Test 4: Null parameters\\n";
    auto result4 = Lse87::temperatureForLocation(nullptr, "-74.0060", "2024-01-15");
    std::cout << "Success: " << result4.success << ", Error: " << result4.error << "\\n\\n";
    
    std::cout << "Test 5: Invalid number format\\n";
    auto result5 = Lse87::temperatureForLocation("abc", "-74.0060", "2024-01-15");
    std::cout << "Success: " << result5.success << ", Error: " << result5.error << "\\n";
    
    return 0;
}
