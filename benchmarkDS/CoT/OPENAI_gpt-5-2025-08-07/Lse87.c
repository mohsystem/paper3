#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#ifndef NAN
#define NAN (0.0/0.0)
#endif

typedef struct {
    char key[128];
    double value;
} Entry;

typedef struct {
    Entry entries[128];
    int count;
} SecureDB;

static SecureDB* get_db() {
    static SecureDB db = { .entries = {{0}}, .count = 0 };
    return &db;
}

static void db_insert(double lat, double lon, const char* dateISO, double temperature) {
    SecureDB* db = get_db();
    if (db->count >= 128) return;
    char key[128];
    snprintf(key, sizeof(key), "%.6f|%.6f|%s", lat, lon, dateISO);
    strncpy(db->entries[db->count].key, key, sizeof(db->entries[db->count].key) - 1);
    db->entries[db->count].key[sizeof(db->entries[db->count].key) - 1] = '\0';
    db->entries[db->count].value = temperature;
    db->count++;
}

static bool db_get(const char* key, double* outValue) {
    SecureDB* db = get_db();
    for (int i = 0; i < db->count; ++i) {
        if (strcmp(db->entries[i].key, key) == 0) {
            if (outValue) *outValue = db->entries[i].value;
            return true;
        }
    }
    return false;
}

static int is_valid_lat(double lat) {
    return isfinite(lat) && lat >= -90.0 && lat <= 90.0;
}

static int is_valid_lon(double lon) {
    return isfinite(lon) && lon >= -180.0 && lon <= 180.0;
}

static int is_valid_iso_date(const char* s) {
    if (!s) return 0;
    if (strlen(s) != 10) return 0;
    if (s[4] != '-' || s[7] != '-') return 0;
    for (int i = 0; i < 10; ++i) {
        if (i == 4 || i == 7) continue;
        if (s[i] < '0' || s[i] > '9') return 0;
    }
    int y = (s[0]-'0')*1000 + (s[1]-'0')*100 + (s[2]-'0')*10 + (s[3]-'0');
    int m = (s[5]-'0')*10 + (s[6]-'0');
    int d = (s[8]-'0')*10 + (s[9]-'0');
    if (m < 1 || m > 12) return 0;
    if (d < 1 || d > 31) return 0;
    (void)y;
    return 1;
}

// Function: accepts inputs as parameters and returns the temperature; returns NAN if not found or invalid input
double temperature_for_location(double latitude, double longitude, const char* dateISO) {
    if (!is_valid_lat(latitude) || !is_valid_lon(longitude)) return NAN;
    if (!is_valid_iso_date(dateISO)) return NAN;

    char key[128];
    snprintf(key, sizeof(key), "%.6f|%.6f|%s", latitude, longitude, dateISO);

    double temp = 0.0;
    if (db_get(key, &temp)) {
        return temp;
    }
    return NAN;
}

static void seed_sample_data() {
    db_insert(37.7749, -122.4194, "2023-10-01", 20.5);
    db_insert(40.7128, -74.0060, "2023-10-01", 18.1);
    db_insert(51.5074, -0.1278, "2023-12-25", 5.2);
    db_insert(-33.8688, 151.2093, "2024-01-15", 27.3);
    db_insert(35.6895, 139.6917, "2023-07-20", 30.4);
}

int main(void) {
    seed_sample_data();

    // 5 test cases
    double t1 = temperature_for_location(37.7749, -122.4194, "2023-10-01");
    double t2 = temperature_for_location(35.6895, 139.6917, "2023-07-20");
    double t3 = temperature_for_location(37.7749, -122.4194, "2023-10-02");
    double t4 = temperature_for_location(100.0, -122.4194, "2023-10-01");
    double t5 = temperature_for_location(40.7128, -74.0060, "2023-13-01");

    printf("Test1: %f\n", t1);
    printf("Test2: %f\n", t2);
    printf("Test3: %f\n", t3);
    printf("Test4: %f\n", t4);
    printf("Test5: %f\n", t5);

    return 0;
}