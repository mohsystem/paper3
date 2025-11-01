#include <sqlite3.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

static const char* DB_PATH = "weather.db";

static bool is_valid_lat(double lat) {
    return isfinite(lat) && lat >= -90.0 && lat <= 90.0;
}

static bool is_valid_lon(double lon) {
    return isfinite(lon) && lon >= -180.0 && lon <= 180.0;
}

static bool is_leap(int y) {
    return ((y % 4 == 0) && (y % 100 != 0)) || (y % 400 == 0);
}

static bool is_valid_date(const char* s) {
    if (!s) return false;
    if (strlen(s) != 10) return false;
    if (s[4] != '-' || s[7] != '-') return false;
    int y = 0, m = 0, d = 0;
    if (sscanf(s, "%4d-%2d-%2d", &y, &m, &d) != 3) return false;
    if (m < 1 || m > 12) return false;
    int mdays[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    int days = mdays[m];
    if (m == 2 && is_leap(y)) days = 29;
    if (d < 1 || d > days) return false;
    return true;
}

double temperature_for_location(double latitude, double longitude, const char* date) {
    if (!is_valid_lat(latitude) || !is_valid_lon(longitude) || !is_valid_date(date)) {
        return NAN;
    }

    sqlite3* db = NULL;
    sqlite3_stmt* stmt = NULL;
    double result = NAN;

    if (sqlite3_open(DB_PATH, &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return result;
    }

    const char* sql = "SELECT temperature FROM temperatures WHERE latitude = ? AND longitude = ? AND date = ? LIMIT 1";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_close(db);
        return result;
    }

    sqlite3_bind_double(stmt, 1, latitude);
    sqlite3_bind_double(stmt, 2, longitude);
    sqlite3_bind_text(stmt, 3, date, -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        result = sqlite3_column_double(stmt, 0);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return result;
}

static void exec_noresult(sqlite3* db, const char* sql) {
    char* err = NULL;
    if (sqlite3_exec(db, sql, NULL, NULL, &err) != SQLITE_OK) {
        if (err) sqlite3_free(err);
    }
}

static void setup_database() {
    sqlite3* db = NULL;
    if (sqlite3_open(DB_PATH, &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return;
    }

    const char* create_sql =
        "CREATE TABLE IF NOT EXISTS temperatures ("
        " latitude REAL NOT NULL,"
        " longitude REAL NOT NULL,"
        " date TEXT NOT NULL,"
        " temperature REAL NOT NULL,"
        " PRIMARY KEY (latitude, longitude, date)"
        ");";
    exec_noresult(db, create_sql);
    exec_noresult(db, "DELETE FROM temperatures;");

    const char* ins = "INSERT OR REPLACE INTO temperatures (latitude, longitude, date, temperature) VALUES (?, ?, ?, ?)";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, ins, -1, &stmt, NULL) == SQLITE_OK) {
        struct Row { double lat, lon, temp; const char* date; };
        struct Row rows[] = {
            {10.0, 20.0, 23.5, "2025-01-01"},
            {10.0, 20.0, 24.0, "2025-01-02"},
            {40.7128, -74.0060, -1.0, "2023-12-25"},
            {-33.8688, 151.2093, 12.3, "2024-07-04"},
            {51.5074, -0.1278, 8.5, "2022-11-11"}
        };
        size_t i;
        for (i = 0; i < sizeof(rows)/sizeof(rows[0]); ++i) {
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
    setup_database();
    double r1 = temperature_for_location(10.0, 20.0, "2025-01-01");
    double r2 = temperature_for_location(40.7128, -74.0060, "2023-12-25");
    double r3 = temperature_for_location(10.0, 20.0, "2025-01-03"); /* not found */
    double r4 = temperature_for_location(100.0, 20.0, "2025-01-01"); /* invalid lat */
    double r5 = temperature_for_location(10.0, 20.0, "2025/01/01");  /* invalid date */

    printf("Test 1: %s\n", isnan(r1) ? "NaN" : (sprintf((char[64]){0}, ""), printf("%f", r1), "")); /* compact print */
    printf("Test 2: %s\n", isnan(r2) ? "NaN" : (sprintf((char[64]){0}, ""), printf("%f", r2), ""));
    printf("Test 3: %s\n", isnan(r3) ? "NaN" : (sprintf((char[64]){0}, ""), printf("%f", r3), ""));
    printf("Test 4: %s\n", isnan(r4) ? "NaN" : (sprintf((char[64]){0}, ""), printf("%f", r4), ""));
    printf("Test 5: %s\n", isnan(r5) ? "NaN" : (sprintf((char[64]){0}, ""), printf("%f", r5), ""));
    return 0;
}