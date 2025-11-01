
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <regex.h>

#define DB_PATH "weather.db"

typedef struct {
    int success;
    double temperature;
    char error[256];
} TemperatureResult;

int validate_latitude(double lat) {
    return lat >= -90.0 && lat <= 90.0;
}

int validate_longitude(double lon) {
    return lon >= -180.0 && lon <= 180.0;
}

int validate_date(const char* date) {
    regex_t regex;
    int ret;
    
    ret = regcomp(&regex, "^[0-9]{4}-[0-9]{2}-[0-9]{2}$", REG_EXTENDED);
    if (ret) return 0;
    
    ret = regexec(&regex, date, 0, NULL, 0);
    regfree(&regex);
    
    return ret == 0;
}

TemperatureResult temperature_for_location(const char* latitude_str, 
                                           const char* longitude_str, 
                                           const char* date_str) {
    TemperatureResult result = {0, 0.0, ""};
    
    /* Input validation */
    if (!latitude_str || !longitude_str || !date_str) {
        result.success = 0;
        strcpy(result.error, "Missing parameters");
        return result;
    }
    
    char* endptr;
    double latitude = strtod(latitude_str, &endptr);
    if (*endptr != '\\0') {
        result.success = 0;
        strcpy(result.error, "Invalid latitude format");
        return result;
    }
    
    double longitude = strtod(longitude_str, &endptr);
    if (*endptr != '\\0') {
        result.success = 0;
        strcpy(result.error, "Invalid longitude format");
        return result;
    }
    
    if (!validate_latitude(latitude)) {
        result.success = 0;
        strcpy(result.error, "Invalid latitude range");
        return result;
    }
    
    if (!validate_longitude(longitude)) {
        result.success = 0;
        strcpy(result.error, "Invalid longitude range");
        return result;
    }
    
    if (!validate_date(date_str)) {
        result.success = 0;
        strcpy(result.error, "Invalid date format");
        return result;
    }
    
    /* Database operations */
    sqlite3* db;
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_open(DB_PATH, &db);
    if (rc != SQLITE_OK) {
        result.success = 0;
        strcpy(result.error, "Database connection error");
        return result;
    }
    
    const char* query = "SELECT temperature FROM weather_data WHERE latitude = ? AND longitude = ? AND date = ?";
    
    rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        sqlite3_close(db);
        result.success = 0;
        strcpy(result.error, "Query preparation error");
        return result;
    }
    
    sqlite3_bind_double(stmt, 1, latitude);
    sqlite3_bind_double(stmt, 2, longitude);
    sqlite3_bind_text(stmt, 3, date_str, -1, SQLITE_TRANSIENT);
    
    rc = sqlite3_step(stmt);
    
    if (rc == SQLITE_ROW) {
        result.success = 1;
        result.temperature = sqlite3_column_double(stmt, 0);
        result.error[0] = '\\0';
    } else if (rc == SQLITE_DONE) {
        result.success = 0;
        strcpy(result.error, "No data found");
    } else {
        result.success = 0;
        strcpy(result.error, "Database query error");
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return result;
}

int main() {
    printf("Test 1: Valid input\\n");
    TemperatureResult result1 = temperature_for_location("40.7128", "-74.0060", "2024-01-15");
    printf("Success: %d, Temp: %.2f, Error: %s\\n\\n", result1.success, result1.temperature, result1.error);
    
    printf("Test 2: Invalid latitude\\n");
    TemperatureResult result2 = temperature_for_location("100.0", "-74.0060", "2024-01-15");
    printf("Success: %d, Error: %s\\n\\n", result2.success, result2.error);
    
    printf("Test 3: Invalid date format\\n");
    TemperatureResult result3 = temperature_for_location("40.7128", "-74.0060", "15-01-2024");
    printf("Success: %d, Error: %s\\n\\n", result3.success, result3.error);
    
    printf("Test 4: Null parameters\\n");
    TemperatureResult result4 = temperature_for_location(NULL, "-74.0060", "2024-01-15");
    printf("Success: %d, Error: %s\\n\\n", result4.success, result4.error);
    
    printf("Test 5: Invalid number format\\n");
    TemperatureResult result5 = temperature_for_location("abc", "-74.0060", "2024-01-15");
    printf("Success: %d, Error: %s\\n", result5.success, result5.error);
    
    return 0;
}
