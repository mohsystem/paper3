
#include <iostream>
#include <string>
#include <map>
#include <sqlite3.h>
#include <cstring>

class Lse85 {
public:
    static std::map<std::string, std::string> temperatureForLocation(double latitude, double longitude, const std::string& date) {
        std::map<std::string, std::string> result;
        sqlite3* db = nullptr;
        sqlite3_stmt* stmt = nullptr;
        
        try {
            // Input validation
            if (date.empty()) {
                result["error"] = "Invalid date";
                return result;
            }
            
            if (latitude < -90 || latitude > 90 || longitude < -180 || longitude > 180) {
                result["error"] = "Invalid coordinates";
                return result;
            }
            
            // Open database
            int rc = sqlite3_open("Sensor.db", &db);
            if (rc != SQLITE_OK) {
                result["error"] = "Cannot open database: " + std::string(sqlite3_errmsg(db));
                sqlite3_close(db);
                return result;
            }
            
            // Prepare parameterized statement
            const char* sql = "SELECT temperature FROM sensor_data WHERE latitude = ? AND longitude = ? AND date = ?";
            rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
            
            if (rc != SQLITE_OK) {
                result["error"] = "Failed to prepare statement: " + std::string(sqlite3_errmsg(db));
                sqlite3_close(db);
                return result;
            }
            
            // Bind parameters
            sqlite3_bind_double(stmt, 1, latitude);
            sqlite3_bind_double(stmt, 2, longitude);
            sqlite3_bind_text(stmt, 3, date.c_str(), -1, SQLITE_TRANSIENT);
            
            // Execute query
            rc = sqlite3_step(stmt);
            
            if (rc == SQLITE_ROW) {
                double temperature = sqlite3_column_double(stmt, 0);
                result["temperature"] = std::to_string(temperature);
                result["success"] = "true";
            } else {
                result["error"] = "No data found";
                result["success"] = "false";
            }
            
        } catch (const std::exception& e) {
            result["error"] = "Exception: " + std::string(e.what());
            result["success"] = "false";
        }
        
        // Cleanup
        if (stmt) sqlite3_finalize(stmt);
        if (db) sqlite3_close(db);
        
        return result;
    }
};

int main() {
    // Test cases
    std::cout << "Test Case 1:" << std::endl;
    auto result1 = Lse85::temperatureForLocation(40.7128, -74.0060, "2023-01-15");
    for (const auto& pair : result1) {
        std::cout << pair.first << ": " << pair.second << std::endl;
    }
    
    std::cout << "\\nTest Case 2:" << std::endl;
    auto result2 = Lse85::temperatureForLocation(51.5074, -0.1278, "2023-02-20");
    for (const auto& pair : result2) {
        std::cout << pair.first << ": " << pair.second << std::endl;
    }
    
    std::cout << "\\nTest Case 3:" << std::endl;
    auto result3 = Lse85::temperatureForLocation(35.6762, 139.6503, "2023-03-10");
    for (const auto& pair : result3) {
        std::cout << pair.first << ": " << pair.second << std::endl;
    }
    
    std::cout << "\\nTest Case 4 (Invalid coordinates):" << std::endl;
    auto result4 = Lse85::temperatureForLocation(100.0, -200.0, "2023-04-05");
    for (const auto& pair : result4) {
        std::cout << pair.first << ": " << pair.second << std::endl;
    }
    
    std::cout << "\\nTest Case 5 (Empty date):" << std::endl;
    auto result5 = Lse85::temperatureForLocation(48.8566, 2.3522, "");
    for (const auto& pair : result5) {
        std::cout << pair.first << ": " << pair.second << std::endl;
    }
    
    return 0;
}
