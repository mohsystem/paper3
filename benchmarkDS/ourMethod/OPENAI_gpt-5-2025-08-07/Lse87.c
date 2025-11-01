#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

static const char* DB_PATH = "weather_secure_c.db";

static bool is_valid_lat_lon(double lat, double lon) {
    if (isnan(lat) || isnan(lon)) return false;
    return (lat >= -90.0 && lat <= 90.0 && lon >= -180.0 && lon <= 180.0);
}

static bool is_valid_date(const char* date) {
    if (date == NULL) return false;
    if (strlen(date) != 10) return false;
    for (int i = 0; i < 10; i++) {
        char c = date[i];
        if (i == 4 || i == 7) {
            if (c != '-') return false;
        } else {
            if (c < '0' || c > '9') return false;
        }
    }
    int year = atoi((char[]){date[0],date[1],date[2],date[3],'\0'});
    int month = atoi((char[]){date[5],date[6],'\0'});
    int day = atoi((char[]){date[8],date[9],'\0'});
    if (year < 1 || year > 9999) return false;
    if (month < 1 || month > 12) return false;
    bool leap = ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0));
    int dim[12] = {31, (leap?29:28), 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (day < 1 || day > dim[month - 1]) return false;
    return true;
}

// Returns temperature for (lat, lon, date) or NAN if not found/invalid/error.
// Intended to be invoked when /api/temperature_for_location is requested.
double temperature_for_location(double latitude, double longitude, const char* date) {
    if (!is_valid_lat_lon(latitude, longitude) || !is_valid_date(date)) {
        return NAN;
    }

    sqlite3* db = NULL;
    if (sqlite3_open(DB_PATH, &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return NAN;
    }

    const char* sql = "SELECT temp FROM temperatures WHERE lat = ? AND lon = ? AND date = ? LIMIT 1";
    sqlite3_stmt* stmt = NULL;
    double result = NAN;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        if (stmt) sqlite3_finalize(stmt);
        sqlite3_close(db);
        return NAN;
    }

    if (sqlite3_bind_double(stmt, 1, latitude) != SQLITE_OK ||
        sqlite3_bind_double(stmt, 2, longitude) != SQLITE_OK ||
        sqlite3_bind_text(stmt, 3, date, -1, SQLITE_TRANSIENT) != SQLITE_OK) {
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return NAN;
    }

    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        result = sqlite3_column_double(stmt, 0);
    } else {
        result = NAN;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return result;
}

static void exec_no_result(sqlite3* db, const char* sql) {
    char* err = NULL;
    if (sqlite3_exec(db, sql, NULL, NULL, &err) != SQLITE_OK) {
        if (err) sqlite3_free(err);
    }
}

static void seed_database(void) {
    sqlite3* db = NULL;
    if (sqlite3_open(DB_PATH, &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return;
    }

    exec_no_result(db, "PRAGMA journal_mode=WAL;");
    exec_no_result(db, "CREATE TABLE IF NOT EXISTS temperatures ("
                        "lat REAL NOT NULL, "
                        "lon REAL NOT NULL, "
                        "date TEXT NOT NULL, "
                        "temp REAL NOT NULL, "
                        "PRIMARY KEY(lat, lon, date))");
    exec_no_result(db, "DELETE FROM temperatures;");

    const char* ins = "INSERT INTO temperatures(lat, lon, date, temp) VALUES(?,?,?,?)";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, ins, -1, &stmt, NULL) == SQLITE_OK) {
        struct Row { double lat; double lon; const char* date; double temp; } rows[] = {
            {37.7749, -122.4194, "2025-01-01", 12.3},
            {40.7128,  -74.0060, "2025-01-01", 5.7},
            {34.0522, -118.2437, "2025-01-02", 15.2},
            {51.5074,   -0.1278, "2025-01-01", 7.1},
            {-33.8688, 151.2093, "2025-01-03", 22.8}
        };
        for (size_t i = 0; i < sizeof(rows)/sizeof(rows[0]); i++) {
            sqlite3_reset(stmt);
            sqlite3_clear_bindings(stmt);
            sqlite3_bind_double(stmt, 1, rows[i].lat);
            sqlite3_bind_double(stmt, 2, rows[i].lon);
            sqlite3_bind_text(stmt, 3, rows[i].date, -1, SQLITE_TRANSIENT);
            sqlite3_bind_double(stmt, 4, rows[i].temp);
            sqlite3_step(stmt);
        }
    }
    if (stmt) sqlite3_finalize(stmt);
    sqlite3_close(db);
}

int main(void) {
    seed_database();

    struct Test { double lat; double lon; const char* date; } tests[5] = {
        {37.7749, -122.4194, "2025-01-01"},
        {40.7128,  -74.0060, "2025-01-01"},
        {34.0522, -118.2437, "2025-01-02"},
        {0.0, 0.0, "2025-01-01"},       /* not found */
        {100.0, 0.0, "2025-13-40"}      /* invalid */
    };

    for (int i = 0; i < 5; i++) {
        double res = temperature_for_location(tests[i].lat, tests[i].lon, tests[i].date);
        if (isnan(res)) {
            printf("Test%d: NaN\n", i+1);
        } else {
            printf("Test%d: %.2f\n", i+1, res);
        }
    }

    return 0;
}