#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <stdbool.h>

static long long to_micro(double deg) {
    double v = deg * 1000000.0;
    if (v > (double)LLONG_MAX) return LLONG_MAX;
    if (v < (double)LLONG_MIN) return LLONG_MIN;
    return (long long) llround(v);
}

static int is_valid_lat(double lat) {
    return !isnan(lat) && lat >= -90.0 && lat <= 90.0;
}

static int is_valid_lon(double lon) {
    return !isnan(lon) && lon >= -180.0 && lon <= 180.0;
}

static int is_leap(int y) {
    if (y % 400 == 0) return 1;
    if (y % 100 == 0) return 0;
    return (y % 4 == 0);
}

static int is_valid_date(const char* s) {
    if (!s) return 0;
    if (strlen(s) != 10) return 0;
    for (int i = 0; i < 10; i++) {
        if (i == 4 || i == 7) {
            if (s[i] != '-') return 0;
        } else {
            if (s[i] < '0' || s[i] > '9') return 0;
        }
    }
    int y = (s[0]-'0')*1000 + (s[1]-'0')*100 + (s[2]-'0')*10 + (s[3]-'0');
    int m = (s[5]-'0')*10 + (s[6]-'0');
    int d = (s[8]-'0')*10 + (s[9]-'0');
    if (y < 1900 || y > 2100) return 0;
    if (m < 1 || m > 12) return 0;
    int mdays[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if (is_leap(y)) mdays[1] = 29;
    if (d < 1 || d > mdays[m-1]) return 0;
    return 1;
}

static void init_db(void) {
    sqlite3* db = NULL;
    if (sqlite3_open("Sensor.db", &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        fprintf(stderr, "Database initialization error.\n");
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

    char* err = NULL;
    if (sqlite3_exec(db, create_sql, NULL, NULL, &err) != SQLITE_OK) {
        if (err) sqlite3_free(err);
        sqlite3_close(db);
        fprintf(stderr, "Database initialization error.\n");
        return;
    }

    const char* ins_sql = "INSERT OR IGNORE INTO temperatures(latitude_micro, longitude_micro, date, temperature) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, ins_sql, -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_close(db);
        fprintf(stderr, "Database initialization error.\n");
        return;
    }

    struct Sample { double lat; double lon; const char* date; double temp; } samples[] = {
        {37.7749, -122.4194, "2025-01-01", 15.2},
        {40.7128,  -74.0060, "2025-01-01", 5.4},
        {51.5074,   -0.1278, "2025-01-02", 7.1},
        {-33.8688, 151.2093, "2025-01-01", 26.3},
        {35.6895,  139.6917, "2025-01-03", 8.9}
    };

    for (size_t i = 0; i < sizeof(samples)/sizeof(samples[0]); i++) {
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
        sqlite3_bind_int64(stmt, 1, to_micro(samples[i].lat));
        sqlite3_bind_int64(stmt, 2, to_micro(samples[i].lon));
        sqlite3_bind_text(stmt, 3, samples[i].date, -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 4, samples[i].temp);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            // ignore seeding error
        }
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

// Returns temperature or NAN if not found/invalid
double temperature_for_location(double latitude, double longitude, const char* date) {
    if (!is_valid_lat(latitude) || !is_valid_lon(longitude) || !is_valid_date(date)) {
        return NAN;
    }

    sqlite3* db = NULL;
    if (sqlite3_open("Sensor.db", &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return NAN;
    }

    const char* sql = "SELECT temperature FROM temperatures WHERE latitude_micro=? AND longitude_micro=? AND date=? LIMIT 1;";
    sqlite3_stmt* stmt = NULL;
    double result = NAN;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
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

int main(void) {
    init_db();

    struct Test { double lat; double lon; const char* date; } tests[5] = {
        {37.7749, -122.4194, "2025-01-01"},
        {40.7128,  -74.0060, "2025-01-01"},
        {51.5074,   -0.1278, "2025-01-02"},
        {-33.8688, 151.2093, "2025-01-01"},
        {91.0, 0.0, "2025-01-01"} // invalid latitude
    };

    for (int i = 0; i < 5; i++) {
        double t = temperature_for_location(tests[i].lat, tests[i].lon, tests[i].date);
        if (isnan(t)) {
            printf("Test %d: temp=not found or invalid\n", i+1);
        } else {
            printf("Test %d: temp=%.2f\n", i+1, t);
        }
    }

    // extra: valid location but date not present
    double t2 = temperature_for_location(37.7749, -122.4194, "2025-01-03");
    if (isnan(t2)) {
        printf("Extra: temp=not found or invalid\n");
    } else {
        printf("Extra: temp=%.2f\n", t2);
    }

    return 0;
}