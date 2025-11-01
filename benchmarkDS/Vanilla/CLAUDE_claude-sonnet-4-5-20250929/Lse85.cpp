
#include <iostream>
#include <string>
#include <map>
#include <sqlite3.h>

class Lse85 {
public:
    static std::map<std::string, std::string> temperatureForLocation(double latitude, double longitude, const std::string& date) {
        std::map<std::string, std::string> result;
        sqlite3* db;
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_open("Sensor.db", &db);
        
        if (rc) {
            result["success"] = "false";
            result["message"] = "Cannot open database";
            return result;
        }
        
        const char* query = "SELECT temperature FROM sensor_data WHERE latitude = ? AND longitude = ? AND date = ?";
        
        rc = sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);
        
        if (rc != SQLITE_OK) {
            result["success"] = "false";
            result["message"] = "Failed to prepare statement";
            sqlite3_close(db);
            return result;
        }
        
        sqlite3_bind_double(stmt, 1, latitude);
        sqlite3_bind_double(stmt, 2, longitude);
        sqlite3_bind_text(stmt, 3, date.c_str(), -1, SQLITE_STATIC);
        
        rc = sqlite3_step(stmt);
        
        if (rc == SQLITE_ROW) {
            double temperature = sqlite3_column_double(stmt, 0);
            result["temperature"] = std::to_string(temperature);
            result["success"] = "true";
        } else {
            result["temperature"] = "null";
            result["success"] = "false";
            result["message"] = "No data found";
        }
        
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        return result;
    }
};

int main() {
    // Test case 1
    auto result1 = Lse85::temperatureForLocation(40.7128, -74.0060, "2023-01-15");
    std::cout << "Test 1: temperature=" << result1["temperature"] << ", success=" << result1["success"] << std::endl;
    
    // Test case 2
    auto result2 = Lse85::temperatureForLocation(34.0522, -118.2437, "2023-02-20");
    std::cout << "Test 2: temperature=" << result2["temperature"] << ", success=" << result2["success"] << std::endl;
    
    // Test case 3
    auto result3 = Lse85::temperatureForLocation(51.5074, -0.1278, "2023-03-10");
    std::cout << "Test 3: temperature=" << result3["temperature"] << ", success=" << result3["success"] << std::endl;
    
    // Test case 4
    auto result4 = Lse85::temperatureForLocation(35.6762, 139.6503, "2023-04-05");
    std::cout << "Test 4: temperature=" << result4["temperature"] << ", success=" << result4["success"] << std::endl;
    
    // Test case 5
    auto result5 = Lse85::temperatureForLocation(48.8566, 2.3522, "2023-05-12");
    std::cout << "Test 5: temperature=" << result5["temperature"] << ", success=" << result5["success"] << std::endl;
    
    return 0;
}
