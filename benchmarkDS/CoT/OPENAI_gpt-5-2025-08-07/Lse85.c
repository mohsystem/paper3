#include <stdio.h>
#include <sqlite3.h>
#include <math.h>
#include <string.h>
#include <limits.h>

static const char* DB_PATH = "Sensor.db";

static int valid_lat_lon(double lat, double lon) {
    return (lat >= -90.0 && lat <= 90.0 && lon >= -180.0 && lon <= 180.0);
}

static int valid_date(const char* d) {
    if (!d) return 0;
    if (strlen(d) != 10) return 0;
    for (int i = 0; i < 10; ++i) {
        if (i == 4 || i == 7) {
            if (d[i] != '-') return 0;
        } else {
            if (d[i] < '0' || d[i] > '9') return 0;
        }
    }
    return 1;
}

static void init_db(void) {
    sqlite3* db = NULL;
    if (sqlite3_open(DB_PATH, &db) != SQLITE_OK) {
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
    char* err = NULL;
    sqlite3_exec(db, sql, NULL, NULL, &err);
    if (err) sqlite3_free(err);
    sqlite3_close(db);
}

static void seed_data(void) {
    sqlite3* db = NULL;
    if (sqlite3_open(DB_PATH, &db) != SQLITE_OK) {
        return;
    }
    const char* sql = "INSERT OR REPLACE INTO temperatures (lat, lon, date, temp) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_close(db);
        return;
    }
    struct Row { double lat, lon, temp; const char* date; } rows[] = {
        {37.7749, -122.4194, 20.5, "2023-07-01"},
        {40.7128,  -74.0060, 27.3, "2023-07-01"},
        {51.5074,   -0.1278,  5.2, "2023-12-25"},
        {35.6895,  139.6917, 18.0, "2023-04-15"},
        {48.8566,    2.3522, 25.0, "2023-07-14"},
    };
    for (size_t i = 0; i < sizeof(rows)/sizeof(rows[0]); ++i) {
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        sqlite3_bind_double(stmt, 1, rows[i].lat);
        sqlite3_bind_double(stmt, 2, rows[i].lon);
        sqlite3_bind_text(stmt, 3, rows[i].date, -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, 4, rows[i].temp);
        sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

// Function to be called when /api/temperature_for_location is requested
// Returns NaN when not found or invalid input or error
double temperature_for_location(double lat, double lon, const char* date) {
    if (!valid_lat_lon(lat, lon) || !valid_date(date)) {
        return NAN;
    }
    sqlite3* db = NULL;
    if (sqlite3_open(DB_PATH, &db) != SQLITE_OK) {
        return NAN;
    }
    const char* sql = "SELECT temp FROM temperatures WHERE lat = ? AND lon = ? AND date = ?;";
    sqlite3_stmt* stmt = NULL;
    double result = NAN;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_double(stmt, 1, lat);
        sqlite3_bind_double(stmt, 2, lon);
        sqlite3_bind_text(stmt, 3, date, -1, SQLITE_TRANSIENT);

        int rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            result = sqlite3_column_double(stmt, 0);
        }
    }
    if (stmt) sqlite3_finalize(stmt);
    sqlite3_close(db);
    return result;
}

int main(void) {
    init_db();
    seed_data();

    double t1 = temperature_for_location(37.7749, -122.4194, "2023-07-01");
    double t2 = temperature_for_location(40.7128, -74.0060, "2023-07-01");
    double t3 = temperature_for_location(51.5074, -0.1278, "2023-01-01"); // not found
    double t4 = temperature_for_location(95.0, 10.0, "2023-07-01");       // invalid
    double t5 = temperature_for_location(48.8566, 2.3522, "2023-07-14");

    printf("Test1: %s\n", isnan(t1) ? "not found or invalid" : (printf("%f", t1), ""));
    if (!isnan(t1)) printf("\n");
    printf("Test2: %s\n", isnan(t2) ? "not found or invalid" : (printf("%f", t2), ""));
    if (!isnan(t2)) printf("\n");
    printf("Test3: %s\n", isnan(t3) ? "not found or invalid" : (printf("%f", t3), ""));
    if (!isnan(t3)) printf("\n");
    printf("Test4: %s\n", isnan(t4) ? "not found or invalid" : (printf("%f", t4), ""));
    if (!isnan(t4)) printf("\n");
    printf("Test5: %s\n", isnan(t5) ? "not found or invalid" : (printf("%f", t5), ""));
    if (!isnan(t5)) printf("\n");

    return 0;
}