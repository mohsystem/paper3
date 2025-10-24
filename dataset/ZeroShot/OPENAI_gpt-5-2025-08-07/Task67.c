#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sqlite3.h>

#define USERNAME_MAX_LEN 64

struct Customer {
    int id;
    char username[USERNAME_MAX_LEN + 1];
    char full_name[256];
    char email[256];
    char created_at[64];
};

static int is_valid_username(const char* u) {
    if (!u) return 0;
    size_t len = strlen(u);
    if (len == 0 || len > USERNAME_MAX_LEN) return 0;
    for (size_t i = 0; i < len; i++) {
        char c = u[i];
        if (!(isalnum((unsigned char)c) || c == '.' || c == '_' || c == '-')) {
            return 0;
        }
    }
    return 1;
}

static int exec_no_result(sqlite3* db, const char* sql) {
    char* errMsg = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        if (errMsg) sqlite3_free(errMsg);
    }
    return rc;
}

static void init_schema_and_sample_data(sqlite3* db) {
    const char* createTable =
        "CREATE TABLE IF NOT EXISTS customer ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT UNIQUE NOT NULL,"
        "full_name TEXT NOT NULL,"
        "email TEXT NOT NULL,"
        "created_at TEXT NOT NULL"
        ");";
    exec_no_result(db, createTable);

    const char* insertSQL = "INSERT OR IGNORE INTO customer(username, full_name, email, created_at) VALUES (?,?,?,?)";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, insertSQL, -1, &stmt, NULL) == SQLITE_OK) {
        const struct {
            const char* u; const char* f; const char* e; const char* c;
        } data[] = {
            {"alice", "Alice Johnson", "alice@example.com", "2023-01-10T09:15:00Z"},
            {"bob", "Bob Smith", "bob@example.com", "2023-02-12T10:20:00Z"},
            {"carol", "Carol White", "carol@example.com", "2023-03-15T11:25:00Z"},
            {"dave", "Dave Brown", "dave@example.com", "2023-04-18T12:30:00Z"},
            {"eve", "Eve Black", "eve@example.com", "2023-05-20T13:35:00Z"},
        };
        for (size_t i = 0; i < sizeof(data)/sizeof(data[0]); i++) {
            sqlite3_reset(stmt);
            sqlite3_bind_text(stmt, 1, data[i].u, -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, data[i].f, -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, data[i].e, -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 4, data[i].c, -1, SQLITE_TRANSIENT);
            sqlite3_step(stmt);
        }
        sqlite3_finalize(stmt);
    }
}

static sqlite3* get_connection_or_init_sample() {
    sqlite3* db = NULL;
    const char* db_path = getenv("DB_PATH");
    const char* open_path = (db_path && *db_path) ? db_path : ":memory:";
    if (sqlite3_open_v2(open_path, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return NULL;
    }
    if (!db_path || !*db_path) {
        init_schema_and_sample_data(db);
    }
    return db;
}

// Returns 1 and fills out if found; 0 otherwise.
static int get_customer_by_username(sqlite3* db, const char* username, struct Customer* out) {
    if (!db || !username || !out || !is_valid_username(username)) return 0;

    const char* sql = "SELECT id, username, full_name, email, created_at FROM customer WHERE username = ?";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return 0;
    }
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);

    int found = 0;
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        out->id = sqlite3_column_int(stmt, 0);
        const unsigned char* u = sqlite3_column_text(stmt, 1);
        const unsigned char* f = sqlite3_column_text(stmt, 2);
        const unsigned char* e = sqlite3_column_text(stmt, 3);
        const unsigned char* c = sqlite3_column_text(stmt, 4);

        if (u) { strncpy(out->username, (const char*)u, sizeof(out->username)-1); out->username[sizeof(out->username)-1] = '\0'; }
        if (f) { strncpy(out->full_name, (const char*)f, sizeof(out->full_name)-1); out->full_name[sizeof(out->full_name)-1] = '\0'; }
        if (e) { strncpy(out->email, (const char*)e, sizeof(out->email)-1); out->email[sizeof(out->email)-1] = '\0'; }
        if (c) { strncpy(out->created_at, (const char*)c, sizeof(out->created_at)-1); out->created_at[sizeof(out->created_at)-1] = '\0'; }
        found = 1;
    }
    sqlite3_finalize(stmt);
    return found;
}

int main(int argc, char** argv) {
    sqlite3* db = get_connection_or_init_sample();
    if (!db) {
        fprintf(stderr, "Failed to open database.\n");
        return 1;
    }

    if (argc > 1) {
        const char* username = argv[1];
        struct Customer c;
        if (get_customer_by_username(db, username, &c)) {
            printf("Customer{id=%d, username='%s', fullName='%s', email='%s', createdAt='%s'}\n",
                   c.id, c.username, c.full_name, c.email, c.created_at);
        } else {
            printf("No customer found for username: %s\n", username);
        }
    } else {
        const char* tests[5] = {"alice", "bob", "carol", "dave", "mallory"};
        for (int i = 0; i < 5; i++) {
            struct Customer c;
            if (get_customer_by_username(db, tests[i], &c)) {
                printf("Customer{id=%d, username='%s', fullName='%s', email='%s', createdAt='%s'}\n",
                       c.id, c.username, c.full_name, c.email, c.created_at);
            } else {
                printf("No customer found for username: %s\n", tests[i]);
            }
        }
    }

    sqlite3_close(db);
    return 0;
}