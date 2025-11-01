#include <sqlite3.h>
#include <iostream>
#include <string>
#include <limits>
#include <cmath>

static const char* DB_PATH = "Sensor.db";

static bool valid_lat_lon(double lat, double lon) {
    return lat >= -90.0 && lat <= 90.0 && lon >= -180.0 && lon <= 180.0;
}

static bool valid_date(const std::string& d) {
    if (d.size() != 10) return false;
    for (size_t i = 0; i < d.size(); ++i) {
        if (i == 4 || i == 7) {
            if (d[i] != '-') return false;
        } else {
            if (d[i] < '0' || d[i] > '9') return false;
        }
    }
    return true;
}

static void init_db() {
    sqlite3* db = nullptr;
    if (sqlite3_open(DB_PATH, &db) != SQLITE_OK) {
        std::cerr << "DB open error\n";
        return;
    }
    const char* sql =
        "CREATE TABLE IF NOT EXISTS temperatures ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "lat REAL NOT NULL,"
        "lon REAL NOT NULL,"
        "date TEXT NOT NULL,"
        "temp REAL NOT NULL,"
        "UNIQUE(lat, lon, date));";
    char* err = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &err) != SQLITE_OK) {
        if (err) sqlite3_free(err);
    }
    sqlite3_close(db);
}

static void seed_data() {
    sqlite3* db = nullptr;
    if (sqlite3_open(DB_PATH, &db) != SQLITE_OK) {
        return;
    }
    const char* sql = "INSERT OR REPLACE INTO temperatures (lat, lon, date, temp) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        return;
    }
    auto add = [&](double lat, double lon, const char* date, double temp) {
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        sqlite3_bind_double(stmt, 1, lat);
        sqlite3_bind_double(stmt, 2, lon);
        sqlite3_bind_text(stmt, 3, date, -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, 4, temp);
        sqlite3_step(stmt);
    };
    add(37.7749, -122.4194, "2023-07-01", 20.5);
    add(40.7128, -74.0060, "2023-07-01", 27.3);
    add(51.5074, -0.1278, "2023-12-25", 5.2);
    add(35.6895, 139.6917, "2023-04-15", 18.0);
    add(48.8566, 2.3522, "2023-07-14", 25.0);

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

// Function to be called when /api/temperature_for_location is requested
// Returns NaN if not found or invalid input / error
double temperature_for_location(double lat, double lon, const std::string& date) {
    if (!valid_lat_lon(lat, lon) || !valid_date(date)) {
        return std::numeric_limits<double>::quiet_NaN();
    }
    sqlite3* db = nullptr;
    if (sqlite3_open(DB_PATH, &db) != SQLITE_OK) {
        return std::numeric_limits<double>::quiet_NaN();
    }
    const char* sql = "SELECT temp FROM temperatures WHERE lat = ? AND lon = ? AND date = ?;";
    sqlite3_stmt* stmt = nullptr;
    double result = std::numeric_limits<double>::quiet_NaN();

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_double(stmt, 1, lat);
        sqlite3_bind_double(stmt, 2, lon);
        sqlite3_bind_text(stmt, 3, date.c_str(), -1, SQLITE_TRANSIENT);

        int rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            result = sqlite3_column_double(stmt, 0);
        }
    }
    if (stmt) sqlite3_finalize(stmt);
    sqlite3_close(db);
    return result;
}

int main() {
    init_db();
    seed_data();

    double t1 = temperature_for_location(37.7749, -122.4194, "2023-07-01");
    double t2 = temperature_for_location(40.7128, -74.0060, "2023-07-01");
    double t3 = temperature_for_location(51.5074, -0.1278, "2023-01-01"); // not found
    double t4 = temperature_for_location(95.0, 10.0, "2023-07-01");       // invalid
    double t5 = temperature_for_location(48.8566, 2.3522, "2023-07-14");

    auto print = [](const char* label, double v) {
        if (std::isnan(v)) std::cout << label << ": not found or invalid\n";
        else std::cout << label << ": " << v << "\n";
    };
    print("Test1", t1);
    print("Test2", t2);
    print("Test3", t3);
    print("Test4", t4);
    print("Test5", t5);
    return 0;
}