#include <sqlite3.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

double temperature_for_location(double latitude, double longitude, const char* date) {
    sqlite3* db = NULL;
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return NAN;
    }

    char* errMsg = NULL;
    const char* createSQL = "CREATE TABLE temperatures (lat REAL, lon REAL, date TEXT, temp REAL);";
    if (sqlite3_exec(db, createSQL, NULL, NULL, &errMsg) != SQLITE_OK) {
        if (errMsg) sqlite3_free(errMsg);
        sqlite3_close(db);
        return NAN;
    }
    if (errMsg) { sqlite3_free(errMsg); errMsg = NULL; }

    const char* insertSQL = "INSERT INTO temperatures(lat, lon, date, temp) VALUES (?,?,?,?);";
    sqlite3_stmt* ins = NULL;
    if (sqlite3_prepare_v2(db, insertSQL, -1, &ins, NULL) != SQLITE_OK) {
        sqlite3_close(db);
        return NAN;
    }

    // Helper macro to insert a row
    #define INSERT_ROW(LAT, LON, DATE, TEMP) do { \
        sqlite3_bind_double(ins, 1, (LAT)); \
        sqlite3_bind_double(ins, 2, (LON)); \
        sqlite3_bind_text(ins, 3, (DATE), -1, SQLITE_TRANSIENT); \
        sqlite3_bind_double(ins, 4, (TEMP)); \
        sqlite3_step(ins); \
        sqlite3_reset(ins); \
        sqlite3_clear_bindings(ins); \
    } while (0)

    INSERT_ROW(37.7749, -122.4194, "2025-01-01", 15.2);
    INSERT_ROW(40.7128,  -74.0060, "2025-01-01", 3.4);
    INSERT_ROW(34.0522, -118.2437, "2025-01-01", 20.1);
    INSERT_ROW(51.5074,   -0.1278, "2025-01-01", 7.8);
    INSERT_ROW(35.6895,  139.6917, "2025-01-01", 8.9);
    INSERT_ROW(37.7749, -122.4194, "2025-02-01", 16.0);

    sqlite3_finalize(ins);

    const char* selectSQL = "SELECT temp FROM temperatures WHERE lat=? AND lon=? AND date=? LIMIT 1;";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, selectSQL, -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_close(db);
        return NAN;
    }

    sqlite3_bind_double(stmt, 1, latitude);
    sqlite3_bind_double(stmt, 2, longitude);
    sqlite3_bind_text(stmt, 3, date, -1, SQLITE_TRANSIENT);

    double result = NAN;
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        result = sqlite3_column_double(stmt, 0);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return result;
}

int main(void) {
    printf("%f\n", temperature_for_location(37.7749, -122.4194, "2025-01-01"));
    printf("%f\n", temperature_for_location(40.7128, -74.0060, "2025-01-01"));
    printf("%f\n", temperature_for_location(34.0522, -118.2437, "2025-01-01"));
    printf("%f\n", temperature_for_location(51.5074, -0.1278, "2025-01-01"));
    printf("%f\n", temperature_for_location(37.7749, -122.4194, "2025-02-01"));
    return 0;
}