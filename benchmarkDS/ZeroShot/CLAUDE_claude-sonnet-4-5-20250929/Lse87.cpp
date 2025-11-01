
#include <iostream>
#include <sqlite3.h>
#include <string>
#include <stdexcept>
#include <memory>

class Lse87 {
public:
    static double* temperatureForLocation(double latitude, double longitude, const std::string& date) {
        sqlite3* db = nullptr;
        sqlite3_stmt* stmt = nullptr;
        double* temperature = nullptr;
        
        try {
            // Input validation
            if (date.empty()) {
                throw std::invalid_argument("Date cannot be empty");
            }
            
            if (latitude < -90 || latitude > 90) {
                throw std::invalid_argument("Invalid latitude");
            }
            
            if (longitude < -180 || longitude > 180) {
                throw std::invalid_argument("Invalid longitude");
            }
            
            // Open database connection
            int rc = sqlite3_open("weather.db", &db);
            if (rc != SQLITE_OK) {
                std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
                sqlite3_close(db);
                return nullptr;
            }
            
            // Prepare parameterized query to prevent SQL injection
            const char* query = "SELECT temperature FROM weather_data WHERE latitude = ? AND longitude = ? AND date = ?";
            rc = sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);
            
            if (rc != SQLITE_OK) {
                std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
                sqlite3_close(db);
                return nullptr;
            }
            
            // Bind parameters
            sqlite3_bind_double(stmt, 1, latitude);
            sqlite3_bind_double(stmt, 2, longitude);
            sqlite3_bind_text(stmt, 3, date.c_str(), -1, SQLITE_TRANSIENT);
            
            // Execute query
            rc = sqlite3_step(stmt);
            if (rc == SQLITE_ROW) {
                temperature = new double(sqlite3_column_double(stmt, 0));
            }
            
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
        
        // Clean up
        if (stmt) {
            sqlite3_finalize(stmt);
        }
        if (db) {
            sqlite3_close(db);
        }
        
        return temperature;
    }
};

int main() {
    // Test cases
    std::cout << "Test Case 1:" << std::endl;
    double* temp1 = Lse87::temperatureForLocation(40.7128, -74.0060, "2023-01-15");
    if (temp1) {
        std::cout << "Temperature: " << *temp1 << std::endl;
        delete temp1;
    } else {
        std::cout << "Temperature: Not found" << std::endl;
    }
    
    std::cout << "\\nTest Case 2:" << std::endl;
    double* temp2 = Lse87::temperatureForLocation(51.5074, -0.1278, "2023-06-20");
    if (temp2) {
        std::cout << "Temperature: " << *temp2 << std::endl;
        delete temp2;
    } else {
        std::cout << "Temperature: Not found" << std::endl;
    }
    
    std::cout << "\\nTest Case 3:" << std::endl;
    double* temp3 = Lse87::temperatureForLocation(35.6762, 139.6503, "2023-12-25");
    if (temp3) {
        std::cout << "Temperature: " << *temp3 << std::endl;
        delete temp3;
    } else {
        std::cout << "Temperature: Not found" << std::endl;
    }
    
    std::cout << "\\nTest Case 4:" << std::endl;
    double* temp4 = Lse87::temperatureForLocation(-33.8688, 151.2093, "2023-03-10");
    if (temp4) {
        std::cout << "Temperature: " << *temp4 << std::endl;
        delete temp4;
    } else {
        std::cout << "Temperature: Not found" << std::endl;
    }
    
    std::cout << "\\nTest Case 5:" << std::endl;
    double* temp5 = Lse87::temperatureForLocation(48.8566, 2.3522, "2023-09-05");
    if (temp5) {
        std::cout << "Temperature: " << *temp5 << std::endl;
        delete temp5;
    } else {
        std::cout << "Temperature: Not found" << std::endl;
    }
    
    return 0;
}
