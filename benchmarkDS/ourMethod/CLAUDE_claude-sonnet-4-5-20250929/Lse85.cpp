
#include <iostream>
#include <string>
#include <regex>
#include <memory>
#include <sqlite3.h>
#include <cstring>
#include <limits>

// Validation constants
constexpr double MIN_LAT = -90.0;
constexpr double MAX_LAT = 90.0;
constexpr double MIN_LON = -180.0;
constexpr double MAX_LON = 180.0;
constexpr size_t MAX_DATE_LEN = 10;

// Result structure for API response
struct TemperatureResponse {
    bool success;
    double temperature;
    std::string error;
    
    TemperatureResponse(bool s, double t, const std::string& e) 
        : success(s), temperature(t), error(e) {}
};

/**
 * Validates latitude value
 * Prevents injection and ensures valid geographic coordinate
 */
bool isValidLatitude(double lat) {
    return lat >= MIN_LAT && lat <= MAX_LAT && !std::isnan(lat) && !std::isinf(lat);
}

/**
 * Validates longitude value
 * Prevents injection and ensures valid geographic coordinate
 */
bool isValidLongitude(double lon) {
    return lon >= MIN_LON && lon <= MAX_LON && !std::isnan(lon) && !std::isinf(lon);
}

/**
 * Validates date format (YYYY-MM-DD)
 * Prevents SQL injection via date parameter
 */
bool isValidDate(const std::string& date) {
    if (date.length() > MAX_DATE_LEN) {
        return false;
    }
    
    // Regex pattern for YYYY-MM-DD format
    std::regex datePattern("^\\\\d{4}-\\\\d{2}-\\\\d{2}$");
    return std::regex_match(date, datePattern);
}

/**
 * Main API function to get temperature for a location
 * Uses parameterized queries to prevent SQL injection
 * Validates all inputs before processing
 * Fails closed on any validation error
 */
TemperatureResponse temperatureForLocation(double latitude, double longitude, const std::string& date) {
    // Input validation - reject invalid data early
    if (!isValidLatitude(latitude)) {
        return TemperatureResponse(false, 0.0, "Invalid request");
    }
    
    if (!isValidLongitude(longitude)) {
        return TemperatureResponse(false, 0.0, "Invalid request");
    }
    
    if (!isValidDate(date)) {
        return TemperatureResponse(false, 0.0, "Invalid request");
    }
    
    sqlite3* db = nullptr;
    sqlite3_stmt* stmt = nullptr;
    
    // Open database connection
    int rc = sqlite3_open("Sensor.db", &db);
    if (rc != SQLITE_OK) {
        // Generic error message - do not leak internal details
        if (db) {
            sqlite3_close(db);
        }
        return TemperatureResponse(false, 0.0, "Service error");
    }
    
    // Use parameterized query to prevent SQL injection
    // All user inputs are bound as parameters, never concatenated
    const char* query = "SELECT temperature FROM sensor_data WHERE latitude = ? AND longitude = ? AND date = ? LIMIT 1";
    
    rc = sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        // Generic error message - do not leak database structure
        sqlite3_close(db);
        return TemperatureResponse(false, 0.0, "Service error");
    }
    
    // Bind parameters with type safety
    sqlite3_bind_double(stmt, 1, latitude);
    sqlite3_bind_double(stmt, 2, longitude);
    sqlite3_bind_text(stmt, 3, date.c_str(), -1, SQLITE_TRANSIENT);
    
    // Execute query
    rc = sqlite3_step(stmt);
    
    TemperatureResponse result(false, 0.0, "Data not found");
    
    if (rc == SQLITE_ROW) {
        // Check if value is NULL
        if (sqlite3_column_type(stmt, 0) != SQLITE_NULL) {
            double temperature = sqlite3_column_double(stmt, 0);
            result = TemperatureResponse(true, temperature, "");
        } else {
            result = TemperatureResponse(false, 0.0, "Data not available");
        }
    } else if (rc != SQLITE_DONE) {
        // Generic error message - do not leak database details
        result = TemperatureResponse(false, 0.0, "Service error");
    }
    
    // Cleanup resources - freed exactly once
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return result;
}

int main() {
    // Test case 1: Valid request
    TemperatureResponse result1 = temperatureForLocation(40.7128, -74.0060, "2024-01-15");
    std::cout << "Test 1 - Success: " << result1.success << ", Temp: " << result1.temperature 
              << ", Error: " << result1.error << std::endl;
    
    // Test case 2: Invalid latitude
    TemperatureResponse result2 = temperatureForLocation(95.0, -74.0060, "2024-01-15");
    std::cout << "Test 2 - Success: " << result2.success << ", Temp: " << result2.temperature 
              << ", Error: " << result2.error << std::endl;
    
    // Test case 3: Invalid date format (SQL injection attempt)
    TemperatureResponse result3 = temperatureForLocation(40.7128, -74.0060, "2024-01-15' OR '1'='1");
    std::cout << "Test 3 - Success: " << result3.success << ", Temp: " << result3.temperature 
              << ", Error: " << result3.error << std::endl;
    
    // Test case 4: Invalid longitude
    TemperatureResponse result4 = temperatureForLocation(40.7128, -200.0, "2024-01-15");
    std::cout << "Test 4 - Success: " << result4.success << ", Temp: " << result4.temperature 
              << ", Error: " << result4.error << std::endl;
    
    // Test case 5: Valid coordinates, different date
    TemperatureResponse result5 = temperatureForLocation(51.5074, -0.1278, "2024-02-20");
    std::cout << "Test 5 - Success: " << result5.success << ", Temp: " << result5.temperature 
              << ", Error: " << result5.error << std::endl;
    
    return 0;
}
