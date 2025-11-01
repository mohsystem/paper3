#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sqlite3.h>

static const char* DEFAULT_DB_PATH = "Sensor.db";

static bool is_leap(int y) {
    return (y%4==0 && y%100!=0) || (y%400==0);
}

static bool valid_date(const char* s) {
    if (!s) return false;
    if (strlen(s) != 10) return false;
    if (!(s[4]=='-' && s[7]=='-')) return false;
    for (int i=0;i<10;i++){
        if (i==4 || i==7) continue;
        if (s[i]<'0' || s[i]>'9') return false;
    }
    int y = (s[0]-'0')*1000 + (s[1]-'0')*100 + (s[2]-'0')*10 + (s[3]-'0');
    int m = (s[5]-'0')*10 + (s[6]-'0');
    int d = (s[8]-'0')*10 + (s[9]-'0');
    if (m < 1 || m > 12) return false;
    int mdays[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    int dim = mdays[m] + ((m==2 && is_leap(y)) ? 1 : 0);
    return d >= 1 && d <= dim;
}

static bool validate_inputs(double lat, double lon, const char* date) {
    if (!(lat >= -90.0 && lat <= 90.0)) return false;
    if (!(lon >= -180.0 && lon <= 180.0)) return false;
    if (!valid_date(date)) return false;
    return true;
}

static int exec_sql(sqlite3* db, const char* sql) {
    char* err = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &err);
    if (rc != SQLITE_OK) {
        if (err) sqlite3_free(err);
    }
    return rc;
}

static bool init_database(const char* db_path) {
    sqlite3* db = NULL;
    int rc = sqlite3_open_v2(db_path, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
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
    sqlite3_stmt* stmt = NULL;
    rc = sqlite3_prepare_v2(db, ins_sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) { sqlite3_close(db); return false; }

    double lats[] = {37.7749, 37.7749, 40.7128, 34.0522};
    double lons[] = {-122.4194, -122.4194, -74.0060, -118.2437};
    const char* dates[] = {"2023-08-15","2023-08-16","2023-08-15","2023-08-15"};
    double temps[] = {21.5, 20.0, 25.2, 27.1};

    for (int i = 0; i < 4; ++i) {
        sqlite3_reset(stmt);
        sqlite3_bind_double(stmt, 1, lats[i]);
        sqlite3_bind_double(stmt, 2, lons[i]);
        sqlite3_bind_text(stmt, 3, dates[i], -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, 4, temps[i]);
        sqlite3_step(stmt);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return true;
}

// Function called for /api/temperature_for_location
static bool temperature_for_location(double latitude, double longitude, const char* date, const char* db_path, double* out_temperature) {
    if (!date || !out_temperature) return false;
    if (!validate_inputs(latitude, longitude, date)) return false;
    if (db_path == NULL || db_path[0] == '\0') db_path = DEFAULT_DB_PATH;

    sqlite3* db = NULL;
    int rc = sqlite3_open_v2(db_path, &db, SQLITE_OPEN_READONLY, NULL);
    if (rc != SQLITE_OK) { if (db) sqlite3_close(db); return false; }
    sqlite3_busy_timeout(db, 3000);

    const char* sql = "SELECT temperature FROM SensorReadings WHERE latitude=? AND longitude=? AND date=?;";
    sqlite3_stmt* stmt = NULL;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) { sqlite3_close(db); return false; }

    sqlite3_bind_double(stmt, 1, latitude);
    sqlite3_bind_double(stmt, 2, longitude);
    sqlite3_bind_text(stmt, 3, date, -1, SQLITE_TRANSIENT);

    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        *out_temperature = sqlite3_column_double(stmt, 0);
        found = true;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return found;
}

int main(void) {
    const char* db_path = DEFAULT_DB_PATH;
    if (!init_database(db_path)) {
        fprintf(stderr, "DB init failed\n");
        return 1;
    }

    struct Test { double lat, lon; const char* date; } tests[5] = {
        {37.7749, -122.4194, "2023-08-15"},
        {37.7749, -122.4194, "2023-08-16"},
        {40.7128,  -74.0060, "2023-08-15"},
        {34.0522, -118.2437, "2023-08-15"},
        {0.0, 0.0, "2023-01-01"}
    };

    for (int i = 0; i < 5; ++i) {
        double temp = 0.0;
        bool ok = temperature_for_location(tests[i].lat, tests[i].lon, tests[i].date, db_path, &temp);
        if (ok) printf("Test%d: %.2f\n", i+1, temp);
        else printf("Test%d: not found\n", i+1);
    }
    return 0;
}