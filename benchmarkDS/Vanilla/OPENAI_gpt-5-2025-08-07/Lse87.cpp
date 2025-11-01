#include <sqlite3.h>
#include <iostream>
#include <string>
#include <cmath>

double temperature_for_location(double latitude, double longitude, const std::string& date) {
    sqlite3* db = nullptr;
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return std::nan("");
    }

    char* errMsg = nullptr;
    const char* createSQL = "CREATE TABLE temperatures (lat REAL, lon REAL, date TEXT, temp REAL);";
    if (sqlite3_exec(db, createSQL, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        if (errMsg) sqlite3_free(errMsg);
        sqlite3_close(db);
        return std::nan("");
    }
    if (errMsg) { sqlite3_free(errMsg); errMsg = nullptr; }

    const char* insertSQL = "INSERT INTO temperatures(lat, lon, date, temp) VALUES (?,?,?,?);";
    sqlite3_stmt* ins = nullptr;
    if (sqlite3_prepare_v2(db, insertSQL, -1, &ins, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        return std::nan("");
    }

    auto insertRow = [&](double lat, double lon, const std::string& d, double temp) {
        sqlite3_bind_double(ins, 1, lat);
        sqlite3_bind_double(ins, 2, lon);
        sqlite3_bind_text(ins, 3, d.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(ins, 4, temp);
        sqlite3_step(ins);
        sqlite3_reset(ins);
        sqlite3_clear_bindings(ins);
    };

    insertRow(37.7749, -122.4194, "2025-01-01", 15.2);
    insertRow(40.7128,  -74.0060, "2025-01-01", 3.4);
    insertRow(34.0522, -118.2437, "2025-01-01", 20.1);
    insertRow(51.5074,   -0.1278, "2025-01-01", 7.8);
    insertRow(35.6895,  139.6917, "2025-01-01", 8.9);
    insertRow(37.7749, -122.4194, "2025-02-01", 16.0);

    sqlite3_finalize(ins);

    const char* selectSQL = "SELECT temp FROM temperatures WHERE lat=? AND lon=? AND date=? LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, selectSQL, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        return std::nan("");
    }

    sqlite3_bind_double(stmt, 1, latitude);
    sqlite3_bind_double(stmt, 2, longitude);
    sqlite3_bind_text(stmt, 3, date.c_str(), -1, SQLITE_TRANSIENT);

    double result = std::nan("");
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        result = sqlite3_column_double(stmt, 0);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return result;
}

int main() {
    std::cout << temperature_for_location(37.7749, -122.4194, "2025-01-01") << std::endl;
    std::cout << temperature_for_location(40.7128, -74.0060, "2025-01-01") << std::endl;
    std::cout << temperature_for_location(34.0522, -118.2437, "2025-01-01") << std::endl;
    std::cout << temperature_for_location(51.5074, -0.1278, "2025-01-01") << std::endl;
    std::cout << temperature_for_location(37.7749, -122.4194, "2025-02-01") << std::endl;
    return 0;
}