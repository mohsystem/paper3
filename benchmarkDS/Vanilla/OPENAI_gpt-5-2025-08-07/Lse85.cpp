#include <sqlite3.h>
#include <iostream>
#include <string>
#include <limits>

static void exec_or_throw(sqlite3* db, const std::string& sql) {
    char* err = nullptr;
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err) != SQLITE_OK) {
        std::string msg = err ? err : "unknown error";
        sqlite3_free(err);
        throw std::runtime_error(msg);
    }
}

static void init_db() {
    sqlite3* db = nullptr;
    if (sqlite3_open("Sensor.db", &db) != SQLITE_OK) {
        std::cerr << "Unable to open DB\n";
        return;
    }
    try {
        exec_or_throw(db, "PRAGMA journal_mode=WAL;");
        exec_or_throw(db, "CREATE TABLE IF NOT EXISTS readings ("
                          "lat REAL NOT NULL, "
                          "lon REAL NOT NULL, "
                          "date TEXT NOT NULL, "
                          "temperature REAL NOT NULL, "
                          "PRIMARY KEY(lat, lon, date))");
        exec_or_throw(db, "DELETE FROM readings");

        const char* ins = "INSERT INTO readings(lat, lon, date, temperature) VALUES(?, ?, ?, ?)";
        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, ins, -1, &stmt, nullptr) == SQLITE_OK) {
            auto insert = [&](double lat, double lon, const char* date, double temp) {
                sqlite3_reset(stmt);
                sqlite3_bind_double(stmt, 1, lat);
                sqlite3_bind_double(stmt, 2, lon);
                sqlite3_bind_text(stmt, 3, date, -1, SQLITE_TRANSIENT);
                sqlite3_bind_double(stmt, 4, temp);
                sqlite3_step(stmt);
            };
            insert(37.7749, -122.4194, "2025-01-01", 15.5);
            insert(34.0522, -118.2437, "2025-01-01", 18.2);
            insert(40.7128, -74.0060, "2025-01-02", 5.1);
            insert(51.5074, -0.1278, "2025-01-03", 7.8);
            insert(-33.8688, 151.2093, "2025-01-04", 22.3);
            sqlite3_finalize(stmt);
        }
    } catch (...) {
        // ignore for simplicity
    }
    sqlite3_close(db);
}

double temperature_for_location(double latitude, double longitude, const std::string& date) {
    sqlite3* db = nullptr;
    double result = std::numeric_limits<double>::quiet_NaN();
    if (sqlite3_open("Sensor.db", &db) != SQLITE_OK) {
        return result;
    }
    const char* q = "SELECT temperature FROM readings WHERE lat = ? AND lon = ? AND date = ? LIMIT 1";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, q, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_double(stmt, 1, latitude);
        sqlite3_bind_double(stmt, 2, longitude);
        sqlite3_bind_text(stmt, 3, date.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            result = sqlite3_column_double(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    sqlite3_close(db);
    return result;
}

int main() {
    init_db();
    std::cout << temperature_for_location(37.7749, -122.4194, "2025-01-01") << "\n";
    std::cout << temperature_for_location(34.0522, -118.2437, "2025-01-01") << "\n";
    std::cout << temperature_for_location(40.7128, -74.0060, "2025-01-02") << "\n";
    std::cout << temperature_for_location(51.5074, -0.1278, "2025-01-03") << "\n";
    std::cout << temperature_for_location(-33.8688, 151.2093, "2025-01-04") << "\n";
    return 0;
}