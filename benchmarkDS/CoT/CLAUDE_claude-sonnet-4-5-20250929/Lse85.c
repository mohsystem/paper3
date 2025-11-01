
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <stdbool.h>

#define DB_PATH "Sensor.db"
#define ERROR_TEMP -999.0

bool is_valid_latitude(double lat) {
    return lat >= -90.0 && lat <= 90.0;
}

bool is_valid_longitude(double lon) {
    return lon >= -180.0 && lon <= 180.0;
}

bool is_valid_date(const char* date) {
    int year, month, day;
    if (sscanf(date, "%d-%d-%d", &year, &month, &day) != 3) {
        return false;
    }
    
    if (strlen(date) != 10 || date[4] != '-' || date[7] != '-') {
        return false;
    }
    
    if (month < 1 || month > 12 || day < 1 || day > 31) {
        return false;
    }
    
    return true;
}

double temperature_for_location(double latitude, double longitude, const char* date) {
    if (!is_valid_latitude(latitude) || !is_valid_longitude(longitude) || !is_valid_date(date)) {
        return ERROR_TEMP;
    }
    
    sqlite3* db;
    sqlite3_stmt* stmt;
    double temperature = ERROR_TEMP;
    
    if (sqlite3_open(DB_PATH, &db) != SQLITE_OK) {
        fprintf(stderr, "Database error occurred\\n");
        return ERROR_TEMP;
    }
    
    const char* query = "SELECT temperature FROM sensor_data WHERE latitude = ? AND longitude = ? AND date = ?";
    
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_double(stmt, 1, latitude);
        sqlite3_bind_double(stmt, 2, longitude);
        sqlite3_bind_text(stmt, 3, date, -1, SQLITE_STATIC);
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            temperature = sqlite3_column_double(stmt, 0);
        }
        
        sqlite3_finalize(stmt);
    }
    
    sqlite3_close(db);
    return temperature;
}

int main() {
    printf("Test 1: %.2f\\n", temperature_for_location(40.7128, -74.0060, "2024-01-15"));
    printf("Test 2: %.2f\\n", temperature_for_location(51.5074, -0.1278, "2024-02-20"));
    printf("Test 3: %.2f\\n", temperature_for_location(35.6762, 139.6503, "2024-03-10"));
    printf("Test 4 (Invalid lat): %.2f\\n", temperature_for_location(100.0, -74.0060, "2024-01-15"));
    printf("Test 5 (Invalid date): %.2f\\n", temperature_for_location(40.7128, -74.0060, "2024-13-45"));
    
    return 0;
}
