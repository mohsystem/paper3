#include <iostream>
#include <string>
#include <regex>
#include <cmath>
#include <sqlite3.h>

static const char* DEFAULT_DB_PATH = "Sensor.db";

static bool is_leap(int y) {
    return (y%4==0 && y%100!=0) || (y%400==0);
}

static bool valid_date(const std::string& s) {
    static const std::regex re(R"(^\d{4}-\d{2}-\d{2}$)");
    if (!std::regex_match(s, re)) return false;
    int y = std::stoi(s.substr(0,4));
    int m = std::stoi(s.substr(5,2));
    int d = std::stoi(s.substr(8,2));
    if (m < 1 || m > 12) return false;
    int mdays[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    int dim = mdays[m] + ((m==2 && is_leap(y)) ? 1 : 0);
    return d >= 1 && d <= dim;
}

static void validate_inputs(double lat, double lon, const std::string& date) {
    if (!(lat >= -90.0 && lat <= 90.0)) throw std::invalid_argument("Invalid latitude");
    if (!(lon >= -180.0 && lon <= 180.0)) throw std::invalid_argument("Invalid longitude");
    if (!valid_date(date)) throw std::invalid_argument("Invalid date");
}

static int exec_sql(sqlite3* db, const char* sql) {
    char* err = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
        if (err) sqlite3_free(err);
    }
    return rc;
}

static bool init_database(const char* db_path) {
    sqlite3* db = nullptr;
    int rc = sqlite3_open_v2(db_path, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
    if (rc != SQLITE_OK) { if (db) sqlite3_close(db); return false; }
    sqlite3_busy_timeout(db, 3000);

    const char* create_sql =
        "PRAGMA foreign_keys = ON;"
        "CREATE TABLE IF NOT EXISTS SensorReadings("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  latitude REAL NOT NULL,"
        "  longitude REAL NOT NULL,"
        "  date TEXT NOT NULL,"
        "  temperature REAL NOT NULL,"
        "  UNIQUE(latitude, longitude, date)"
        ");";
    rc = exec_sql(db, create_sql);
    if (rc != SQLITE_OK) { sqlite3_close(db); return false; }

    const char* ins_sql = "INSERT OR IGNORE INTO SensorReadings(latitude, longitude, date, temperature) VALUES(?,?,?,?);";
    sqlite3_stmt* stmt = nullptr;
    rc = sqlite3_prepare_v2(db, ins_sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) { sqlite3_close(db); return false; }

    auto insert = [&](double lat, double lon, const char* date, double temp) {
        sqlite3_reset(stmt);
        sqlite3_bind_double(stmt, 1, lat);
        sqlite3_bind_double(stmt, 2, lon);
        sqlite3_bind_text(stmt, 3, date, -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, 4, temp);
        sqlite3_step(stmt);
    };

    insert(37.7749, -122.4194, "2023-08-15", 21.5);
    insert(37.7749, -122.4194, "2023-08-16", 20.0);
    insert(40.7128, -74.0060, "2023-08-15", 25.2);
    insert(34.0522, -118.2437, "2023-08-15", 27.1);

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return true;
}

// Function called for /api/temperature_for_location
static bool temperature_for_location(double latitude, double longitude, const std::string& date, const char* db_path, double& out_temperature) {
    try {
        validate_inputs(latitude, longitude, date);
    } catch (...) {
        return false;
    }
    if (db_path == nullptr || std::string(db_path).empty()) db_path = DEFAULT_DB_PATH;

    sqlite3* db = nullptr;
    int rc = sqlite3_open_v2(db_path, &db, SQLITE_OPEN_READONLY, nullptr);
    if (rc != SQLITE_OK) { if (db) sqlite3_close(db); return false; }
    sqlite3_busy_timeout(db, 3000);

    const char* sql = "SELECT temperature FROM SensorReadings WHERE latitude=? AND longitude=? AND date=?;";
    sqlite3_stmt* stmt = nullptr;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) { sqlite3_close(db); return false; }

    sqlite3_bind_double(stmt, 1, latitude);
    sqlite3_bind_double(stmt, 2, longitude);
    sqlite3_bind_text(stmt, 3, date.c_str(), -1, SQLITE_TRANSIENT);

    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        out_temperature = sqlite3_column_double(stmt, 0);
        found = true;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return found;
}

int main() {
    const char* db_path = DEFAULT_DB_PATH;
    if (!init_database(db_path)) {
        std::cerr << "DB init failed\n";
        return 1;
    }

    struct Test { double lat, lon; const char* date; };
    Test tests[5] = {
        {37.7749, -122.4194, "2023-08-15"},
        {37.7749, -122.4194, "2023-08-16"},
        {40.7128,  -74.0060, "2023-08-15"},
        {34.0522, -118.2437, "2023-08-15"},
        {0.0, 0.0, "2023-01-01"}
    };

    for (int i = 0; i < 5; ++i) {
        double temp = NAN;
        bool ok = temperature_for_location(tests[i].lat, tests[i].lon, tests[i].date, db_path, temp);
        if (ok) std::cout << "Test" << (i+1) << ": " << temp << "\n";
        else std::cout << "Test" << (i+1) << ": not found\n";
    }
    return 0;
}