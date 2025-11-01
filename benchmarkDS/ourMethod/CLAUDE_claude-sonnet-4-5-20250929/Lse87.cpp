
#include <iostream>
#include <string>
#include <cstring>
#include <sqlite3.h>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <limits>
#include <regex>

// Maximum allowed values to prevent resource exhaustion
const double MAX_LATITUDE = 90.0;
const double MIN_LATITUDE = -90.0;
const double MAX_LONGITUDE = 180.0;
const double MIN_LONGITUDE = -180.0;

/**
 * Validates and parses a double value from string
 * Returns true if valid, false otherwise
 */
bool parseDouble(const std::string& input, double& output) {
    if (input.empty()) {
        return false;
    }
    
    char* end = nullptr;
    // Reset errno before conversion
    errno = 0;
    output = std::strtod(input.c_str(), &end);
    
    // Check for conversion errors
    if (errno == ERANGE || end == input.c_str() || *end != '\\0') {
        return false;
    }
    
    return true;
}

/**
 * Validates date format (YYYY-MM-DD)
 */
bool validateDateFormat(const std::string& date) {
    // Use regex to validate ISO date format
    std::regex datePattern("^\\\\d{4}-\\\\d{2}-\\\\d{2}$");
    if (!std::regex_match(date, datePattern)) {
        return false;
    }
    
    // Additional validation for valid date ranges
    int year = 0, month = 0, day = 0;
    if (std::sscanf(date.c_str(), "%d-%d-%d", &year, &month, &day) != 3) {
        return false;
    }
    
    if (year < 1900 || year > 2100 || month < 1 || month > 12 || day < 1 || day > 31) {
        return false;
    }
    
    return true;
}

/**
 * Retrieves temperature for a given location and date.
 * Uses parameterized queries to prevent SQL injection.
 * Validates all inputs before processing.
 */
std::string temperatureForLocation(const std::string& latitude, 
                                   const std::string& longitude, 
                                   const std::string& date) {
    // Input validation: Check for empty inputs
    if (latitude.empty()) {
        return "Error: Invalid latitude parameter";
    }
    if (longitude.empty()) {
        return "Error: Invalid longitude parameter";
    }
    if (date.empty()) {
        return "Error: Invalid date parameter";
    }
    
    double lat = 0.0, lon = 0.0;
    
    // Input validation: Parse and validate latitude
    if (!parseDouble(latitude, lat)) {
        return "Error: Invalid coordinate format";
    }
    if (lat < MIN_LATITUDE || lat > MAX_LATITUDE) {
        return "Error: Latitude out of valid range";
    }
    
    // Input validation: Parse and validate longitude
    if (!parseDouble(longitude, lon)) {
        return "Error: Invalid coordinate format";
    }
    if (lon < MIN_LONGITUDE || lon > MAX_LONGITUDE) {
        return "Error: Longitude out of valid range";
    }
    
    // Input validation: Validate date format
    if (!validateDateFormat(date)) {
        return "Error: Invalid date format";
    }
    
    sqlite3* db = nullptr;
    sqlite3_stmt* stmt = nullptr;
    std::string result;
    
    // Open database connection
    int rc = sqlite3_open("weather.db", &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Database connection error" << std::endl;
        if (db) {
            sqlite3_close(db);
        }
        return "Error: Database operation failed";
    }
    
    // Use parameterized query to prevent SQL injection
    const char* query = "SELECT temperature FROM weather_data WHERE latitude = ? AND longitude = ? AND date = ? LIMIT 1";
    
    rc = sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Query preparation error" << std::endl;
        sqlite3_close(db);
        return "Error: Database operation failed";
    }
    
    // Bind parameters safely - database driver handles proper escaping
    sqlite3_bind_double(stmt, 1, lat);
    sqlite3_bind_double(stmt, 2, lon);
    sqlite3_bind_text(stmt, 3, date.c_str(), -1, SQLITE_TRANSIENT);
    
    // Execute query
    rc = sqlite3_step(stmt);
    
    if (rc == SQLITE_ROW) {
        double temperature = sqlite3_column_double(stmt, 0);
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2) << temperature;
        result = oss.str();
    } else if (rc == SQLITE_DONE) {
        result = "Error: No data found for given parameters";
    } else {
        std::cerr << "Query execution error" << std::endl;
        result = "Error: Database operation failed";
    }
    
    // Clean up resources
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return result;
}

int main() {
    // Test case 1: Valid input
    std::cout << "Test 1: " << temperatureForLocation("40.7128", "-74.0060", "2024-01-15") << std::endl;
    
    // Test case 2: Invalid latitude (out of range)
    std::cout << "Test 2: " << temperatureForLocation("95.0", "-74.0060", "2024-01-15") << std::endl;
    
    // Test case 3: Invalid date format
    std::cout << "Test 3: " << temperatureForLocation("40.7128", "-74.0060", "01/15/2024") << std::endl;
    
    // Test case 4: Empty input
    std::cout << "Test 4: " << temperatureForLocation("", "-74.0060", "2024-01-15") << std::endl;
    
    // Test case 5: SQL injection attempt (safely handled by parameterized query)
    std::cout << "Test 5: " << temperatureForLocation("40.7128' OR '1'='1", "-74.0060", "2024-01-15") << std::endl;
    
    return 0;
}
