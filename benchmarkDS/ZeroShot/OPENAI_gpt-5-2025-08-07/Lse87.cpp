#include <sqlite3.h>
#include <iostream>
#include <cmath>
#include <string>
#include <limits>

static const char* DB_PATH = "weather.db";

static bool isValidLatitude(double lat) {
    return std::isfinite(lat) && lat >= -90.0 && lat <= 90.0;
}

static bool isValidLongitude(double lon) {
    return std::isfinite(lon) && lon >= -180.0 && lon <= 180.0;
}

static bool isLeap(int y) {
    return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

static bool isValidDate(const std::string& s) {
    if (s.size() != 10 || s[4] != '-' || s[7] != '-') return false;
    int y = 0, m = 0, d = 0;
    try {
        y = std::stoi(s.substr(0,4));
        m = std::stoi(s.substr(5,2));
        d = std::stoi(s.substr(8,2));
    } catch (...) {
        return false;
    }
    if (m < 1 || m > 12) return false;
    int mdays[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    int days = mdays[m];
    if (m == 2 && isLeap(y)) days = 29;
    if (d < 1 || d > days) return false;
    return true;
}

double temperature_for_location(double latitude, double longitude, const std::string& date) {
    if (!isValidLatitude(latitude) || !isValidLongitude(longitude) || !isValidDate(date)) {
        return std::numeric_limits<double>::quiet_NaN();
    }

    sqlite3* db = nullptr;
    sqlite3_stmt* stmt = nullptr;
    double result = std::numeric_limits<double>::quiet_NaN();

    if (sqlite3_open(DB_PATH, &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return result;
    }

    const char* sql = "SELECT temperature FROM temperatures WHERE latitude = ? AND longitude = ? AND date = ? LIMIT 1";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        return result;
    }

    sqlite3_bind_double(stmt, 1, latitude);
    sqlite3_bind_double(stmt, 2, longitude);
    sqlite3_bind_text(stmt, 3, date.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        result = sqlite3_column_double(stmt, 0);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return result;
}

static void execNoResult(sqlite3* db, const char* sql) {
    char* err = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &err) != SQLITE_OK) {
        if (err) sqlite3_free(err);
    }
}

static void setupDatabase() {
    sqlite3* db = nullptr;
    if (sqlite3_open(DB_PATH, &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return;
    }

    const char* createSql =
        "CREATE TABLE IF NOT EXISTS temperatures ("
        " latitude REAL NOT NULL,"
        " longitude REAL NOT NULL,"
        " date TEXT NOT NULL,"
        " temperature REAL NOT NULL,"
        " PRIMARY KEY (latitude, longitude, date)"
        ");";
    execNoResult(db, createSql);
    execNoResult(db, "DELETE FROM temperatures;");

    const char* ins = "INSERT OR REPLACE INTO temperatures (latitude, longitude, date, temperature) VALUES (?, ?, ?, ?)";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, ins, -1, &stmt, nullptr) == SQLITE_OK) {
        struct Row { double lat, lon, temp; const char* date; };
        Row rows[] = {
            {10.0, 20.0, 23.5, "2025-01-01"},
            {10.0, 20.0, 24.0, "2025-01-02"},
            {40.7128, -74.0060, -1.0, "2023-12-25"},
            {-33.8688, 151.2093, 12.3, "2024-07-04"},
            {51.5074, -0.1278, 8.5, "2022-11-11"}
        };
        for (const auto& r : rows) {
            sqlite3_reset(stmt);
            sqlite3_clear_bindings(stmt);
            sqlite3_bind_double(stmt, 1, r.lat);
            sqlite3_bind_double(stmt, 2, r.lon);
            sqlite3_bind_text(stmt, 3, r.date, -1, SQLITE_TRANSIENT);
            sqlite3_bind_double(stmt, 4, r.temp);
            sqlite3_step(stmt);
        }
    }
    if (stmt) sqlite3_finalize(stmt);
    sqlite3_close(db);
}

int main() {
    setupDatabase();
    double r1 = temperature_for_location(10.0, 20.0, "2025-01-01");
    double r2 = temperature_for_location(40.7128, -74.0060, "2023-12-25");
    double r3 = temperature_for_location(10.0, 20.0, "2025-01-03"); // not found
    double r4 = temperature_for_location(100.0, 20.0, "2025-01-01"); // invalid lat
    double r5 = temperature_for_location(10.0, 20.0, "2025/01/01");  // invalid date

    auto printRes = [](int i, double v){
        if (std::isnan(v)) std::cout << "Test " << i << ": NaN\n";
        else std::cout << "Test " << i << ": " << v << "\n";
    };
    printRes(1, r1);
    printRes(2, r2);
    printRes(3, r3);
    printRes(4, r4);
    printRes(5, r5);
    return 0;
}