#include <sqlite3.h>
#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include <limits>

static const char* DB_PATH = "weather_secure_cpp.db";

static bool isValidLatLon(double lat, double lon) {
    if (std::isnan(lat) || std::isnan(lon)) return false;
    return lat >= -90.0 && lat <= 90.0 && lon >= -180.0 && lon <= 180.0;
}

static bool isValidDate(const std::string& date) {
    if (date.size() != 10) return false;
    for (size_t i = 0; i < 10; ++i) {
        char c = date[i];
        if (i == 4 || i == 7) {
            if (c != '-') return false;
        } else if (c < '0' || c > '9') {
            return false;
        }
    }
    int year = std::stoi(date.substr(0,4));
    int month = std::stoi(date.substr(5,2));
    int day = std::stoi(date.substr(8,2));
    if (year < 1 || year > 9999) return false;
    if (month < 1 || month > 12) return false;
    auto isLeap = [](int y)->bool { return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0); };
    int dim[12] = {31, (isLeap(year)?29:28), 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (day < 1 || day > dim[month - 1]) return false;
    return true;
}

// Returns the temperature for (lat, lon, date) or NaN if not found/invalid/error.
// Intended to be invoked for /api/temperature_for_location
double temperature_for_location(double latitude, double longitude, const std::string& date) {
    if (!isValidLatLon(latitude, longitude) || !isValidDate(date)) {
        return std::numeric_limits<double>::quiet_NaN();
    }

    sqlite3* db = nullptr;
    if (sqlite3_open(DB_PATH, &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return std::numeric_limits<double>::quiet_NaN();
    }

    const char* sql = "SELECT temp FROM temperatures WHERE lat = ? AND lon = ? AND date = ? LIMIT 1";
    sqlite3_stmt* stmt = nullptr;
    double result = std::numeric_limits<double>::quiet_NaN();

    do {
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) break;
        if (sqlite3_bind_double(stmt, 1, latitude) != SQLITE_OK) break;
        if (sqlite3_bind_double(stmt, 2, longitude) != SQLITE_OK) break;
        if (sqlite3_bind_text(stmt, 3, date.c_str(), (int)date.size(), SQLITE_TRANSIENT) != SQLITE_OK) break;

        int rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            result = sqlite3_column_double(stmt, 0);
        } else {
            // not found or error
        }
    } while (false);

    if (stmt) sqlite3_finalize(stmt);
    sqlite3_close(db);
    return result;
}

static void execNoResult(sqlite3* db, const char* sql) {
    char* err = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &err) != SQLITE_OK) {
        if (err) sqlite3_free(err);
    }
}

static void seedDatabase() {
    sqlite3* db = nullptr;
    if (sqlite3_open(DB_PATH, &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return;
    }
    execNoResult(db, "PRAGMA journal_mode=WAL;");
    execNoResult(db, "CREATE TABLE IF NOT EXISTS temperatures ("
                     "lat REAL NOT NULL, "
                     "lon REAL NOT NULL, "
                     "date TEXT NOT NULL, "
                     "temp REAL NOT NULL, "
                     "PRIMARY KEY(lat, lon, date))");
    execNoResult(db, "DELETE FROM temperatures;");

    const char* ins = "INSERT INTO temperatures(lat, lon, date, temp) VALUES(?,?,?,?)";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, ins, -1, &stmt, nullptr) == SQLITE_OK) {
        struct Row { double lat; double lon; const char* date; double temp; };
        Row rows[] = {
            {37.7749, -122.4194, "2025-01-01", 12.3},
            {40.7128,  -74.0060, "2025-01-01", 5.7},
            {34.0522, -118.2437, "2025-01-02", 15.2},
            {51.5074,   -0.1278, "2025-01-01", 7.1},
            {-33.8688, 151.2093, "2025-01-03", 22.8}
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
    seedDatabase();

    // 5 test cases
    struct Test { double lat; double lon; std::string date; };
    std::vector<Test> tests = {
        {37.7749, -122.4194, "2025-01-01"},
        {40.7128,  -74.0060, "2025-01-01"},
        {34.0522, -118.2437, "2025-01-02"},
        {0.0, 0.0, "2025-01-01"},       // not found
        {100.0, 0.0, "2025-13-40"}      // invalid
    };

    int i = 1;
    for (const auto& t : tests) {
        double res = temperature_for_location(t.lat, t.lon, t.date);
        if (std::isnan(res)) {
            std::cout << "Test" << i++ << ": NaN\n";
        } else {
            std::cout << "Test" << i++ << ": " << res << "\n";
        }
    }
    return 0;
}