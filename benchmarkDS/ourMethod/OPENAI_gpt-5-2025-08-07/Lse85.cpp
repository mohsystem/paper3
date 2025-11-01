#include <sqlite3.h>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <limits>
#include <iostream>

static inline long long to_micro(double deg) {
    return llround(deg * 1000000.0);
}

static bool is_valid_lat(double lat) {
    return !std::isnan(lat) && lat >= -90.0 && lat <= 90.0;
}

static bool is_valid_lon(double lon) {
    return !std::isnan(lon) && lon >= -180.0 && lon <= 180.0;
}

static bool is_leap(int y) {
    return (y % 4 == 0 && y % 100 != 1) || (y % 400 == 0);
}

static bool is_valid_date(const std::string& s) {
    if (s.size() != 10) return false;
    if (s[4] != '-' || s[7] != '-') return false;
    for (size_t i = 0; i < s.size(); ++i) {
        if (i == 4 || i == 7) continue;
        if (s[i] < '0' || s[i] > '9') return false;
    }
    int y = std::stoi(s.substr(0,4));
    int m = std::stoi(s.substr(5,2));
    int d = std::stoi(s.substr(8,2));
    if (y < 1900 || y > 2100) return false;
    if (m < 1 || m > 12) return false;
    int mdays[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if (m == 2 && ((y%4==0 && y%100!=0) || (y%400==0))) mdays[1] = 29;
    return d >= 1 && d <= mdays[m-1];
}

static bool exec_sql(sqlite3* db, const char* sql) {
    char* err = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
        if (err) sqlite3_free(err);
        return false;
    }
    return true;
}

static void init_db() {
    sqlite3* db = nullptr;
    if (sqlite3_open("Sensor.db", &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        std::cerr << "Database initialization error.\n";
        return;
    }

    const char* create_sql =
        "CREATE TABLE IF NOT EXISTS temperatures ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "latitude_micro INTEGER NOT NULL,"
        "longitude_micro INTEGER NOT NULL,"
        "date TEXT NOT NULL,"
        "temperature REAL NOT NULL,"
        "UNIQUE(latitude_micro, longitude_micro, date)"
        ");";

    if (!exec_sql(db, create_sql)) {
        sqlite3_close(db);
        std::cerr << "Database initialization error.\n";
        return;
    }

    const char* ins_sql = "INSERT OR IGNORE INTO temperatures(latitude_micro, longitude_micro, date, temperature) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, ins_sql, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        std::cerr << "Database initialization error.\n";
        return;
    }

    struct Sample { double lat; double lon; const char* date; double t; };
    Sample samples[] = {
        {37.7749, -122.4194, "2025-01-01", 15.2},
        {40.7128,  -74.0060, "2025-01-01", 5.4},
        {51.5074,   -0.1278, "2025-01-02", 7.1},
        {-33.8688, 151.2093, "2025-01-01", 26.3},
        {35.6895,  139.6917, "2025-01-03", 8.9}
    };

    for (const auto& s : samples) {
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        sqlite3_bind_int64(stmt, 1, to_micro(s.lat));
        sqlite3_bind_int64(stmt, 2, to_micro(s.lon));
        sqlite3_bind_text(stmt, 3, s.date, -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 4, s.t);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            // ignore seeding errors
        }
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

// Returns temperature, or NAN if not found/invalid
double temperature_for_location(double latitude, double longitude, const char* date) {
    if (!date) return std::numeric_limits<double>::quiet_NaN();
    std::string d(date);
    if (!is_valid_lat(latitude) || !is_valid_lon(longitude) || !is_valid_date(d)) {
        return std::numeric_limits<double>::quiet_NaN();
    }

    sqlite3* db = nullptr;
    if (sqlite3_open("Sensor.db", &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return std::numeric_limits<double>::quiet_NaN();
    }

    const char* sql = "SELECT temperature FROM temperatures WHERE latitude_micro=? AND longitude_micro=? AND date=? LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;
    double result = std::numeric_limits<double>::quiet_NaN();

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int64(stmt, 1, to_micro(latitude));
        sqlite3_bind_int64(stmt, 2, to_micro(longitude));
        sqlite3_bind_text(stmt, 3, date, -1, SQLITE_TRANSIENT);

        int rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            result = sqlite3_column_double(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }
    sqlite3_close(db);
    return result;
}

int main() {
    init_db();
    struct Test { double lat; double lon; const char* date; };
    std::vector<Test> tests = {
        {37.7749, -122.4194, "2025-01-01"},
        {40.7128,  -74.0060, "2025-01-01"},
        {51.5074,   -0.1278, "2025-01-02"},
        {-33.8688, 151.2093, "2025-01-01"},
        {91.0, 0.0, "2025-01-01"} // invalid latitude
    };
    for (size_t i = 0; i < tests.size(); ++i) {
        double t = temperature_for_location(tests[i].lat, tests[i].lon, tests[i].date);
        if (std::isnan(t)) {
            std::cout << "Test " << (i+1) << ": temp=not found or invalid\n";
        } else {
            std::cout.setf(std::ios::fixed); std::cout.precision(2);
            std::cout << "Test " << (i+1) << ": temp=" << t << "\n";
        }
    }
    double t2 = temperature_for_location(37.7749, -122.4194, "2025-01-03"); // valid location, non-existent date
    if (std::isnan(t2)) {
        std::cout << "Extra: temp=not found or invalid\n";
    } else {
        std::cout.setf(std::ios::fixed); std::cout.precision(2);
        std::cout << "Extra: temp=" << t2 << "\n";
    }
    return 0;
}