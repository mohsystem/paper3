#include <stdio.h>
#include <sqlite3.h>
#include <math.h>
#include <string.h>

static void init_db() {
    sqlite3* db = NULL;
    if (sqlite3_open("Sensor.db", &db) != SQLITE_OK) {
        return;
    }

    char* err = NULL;
    sqlite3_exec(db, "PRAGMA journal_mode=WAL;", NULL, NULL, &err);
    if (err) { sqlite3_free(err); err = NULL; }

    const char* create_sql =
        "CREATE TABLE IF NOT EXISTS readings ("
        "lat REAL NOT NULL, "
        "lon REAL NOT NULL, "
        "date TEXT NOT NULL, "
        "temperature REAL NOT NULL, "
        "PRIMARY KEY(lat, lon, date))";
    sqlite3_exec(db, create_sql, NULL, NULL, &err);
    if (err) { sqlite3_free(err); err = NULL; }

    sqlite3_exec(db, "DELETE FROM readings", NULL, NULL, &err);
    if (err) { sqlite3_free(err); err = NULL; }

    const char* ins = "INSERT INTO readings(lat, lon, date, temperature) VALUES(?, ?, ?, ?)";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, ins, -1, &stmt, NULL) == SQLITE_OK) {
        struct Row { double lat; double lon; const char* date; double temp; } rows[] = {
            {37.7749, -122.4194, "2025-01-01", 15.5},
            {34.0522, -118.2437, "2025-01-01", 18.2},
            {40.7128, -74.0060, "2025-01-02", 5.1},
            {51.5074, -0.1278, "2025-01-03", 7.8},
            {-33.8688, 151.2093, "2025-01-04", 22.3}
        };
        for (int i = 0; i < 5; ++i) {
            sqlite3_reset(stmt);
            sqlite3_bind_double(stmt, 1, rows[i].lat);
            sqlite3_bind_double(stmt, 2, rows[i].lon);
            sqlite3_bind_text(stmt, 3, rows[i].date, -1, SQLITE_TRANSIENT);
            sqlite3_bind_double(stmt, 4, rows[i].temp);
            sqlite3_step(stmt);
        }
        sqlite3_finalize(stmt);
    }
    sqlite3_close(db);
}

double temperature_for_location(double latitude, double longitude, const char* date) {
    sqlite3* db = NULL;
    double result = NAN;
    if (sqlite3_open("Sensor.db", &db) != SQLITE_OK) {
        return result;
    }
    const char* q = "SELECT temperature FROM readings WHERE lat = ? AND lon = ? AND date = ? LIMIT 1";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, q, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_double(stmt, 1, latitude);
        sqlite3_bind_double(stmt, 2, longitude);
        sqlite3_bind_text(stmt, 3, date, -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            result = sqlite3_column_double(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    sqlite3_close(db);
    return result;
}

int main(void) {
    init_db();
    printf("%f\n", temperature_for_location(37.7749, -122.4194, "2025-01-01"));
    printf("%f\n", temperature_for_location(34.0522, -118.2437, "2025-01-01"));
    printf("%f\n", temperature_for_location(40.7128, -74.0060, "2025-01-02"));
    printf("%f\n", temperature_for_location(51.5074, -0.1278, "2025-01-03"));
    printf("%f\n", temperature_for_location(-33.8688, 151.2093, "2025-01-04"));
    return 0;
}