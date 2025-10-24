#include <stdio.h>
#include <string.h>
#include <sqlite3.h>

typedef struct {
    int id;
    char customerusername[128];
    char name[256];
    char email[256];
    char phone[64];
    char address[256];
} Customer;

static int execOrFail(sqlite3* db, const char* sql) {
    char* errMsg = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        if (errMsg) {
            fprintf(stderr, "SQLite error: %s\n", errMsg);
            sqlite3_free(errMsg);
        }
        return 1;
    }
    return 0;
}

sqlite3* initDatabase() {
    sqlite3* db = NULL;
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
        fprintf(stderr, "Failed to open DB\n");
        return NULL;
    }
    if (execOrFail(db, "CREATE TABLE customer ("
                       "id INTEGER PRIMARY KEY,"
                       "customerusername TEXT UNIQUE,"
                       "name TEXT,"
                       "email TEXT,"
                       "phone TEXT,"
                       "address TEXT)")) {
        sqlite3_close(db);
        return NULL;
    }
    if (execOrFail(db, "INSERT INTO customer VALUES "
                       "(1, 'alice', 'Alice Johnson', 'alice@example.com', '111-222-3333', '123 Maple St'),"
                       "(2, 'bob', 'Bob Smith', 'bob@example.com', '222-333-4444', '456 Oak Ave'),"
                       "(3, 'charlie', 'Charlie Lee', 'charlie@example.com', '333-444-5555', '789 Pine Rd'),"
                       "(4, 'diana', 'Diana Prince', 'diana@example.com', '444-555-6666', '101 Cedar Blvd'),"
                       "(5, 'eric', 'Eric Yang', 'eric@example.com', '555-666-7777', '202 Birch Ln')")) {
        sqlite3_close(db);
        return NULL;
    }
    return db;
}

int getCustomerByUsername(sqlite3* db, const char* customerusername, Customer* out) {
    const char* sql = "SELECT id, customerusername, name, email, phone, address FROM customer WHERE customerusername = ?";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) return 0;
    sqlite3_bind_text(stmt, 1, customerusername, -1, SQLITE_TRANSIENT);
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        out->id = sqlite3_column_int(stmt, 0);
        const unsigned char* u0 = sqlite3_column_text(stmt, 1);
        const unsigned char* u1 = sqlite3_column_text(stmt, 2);
        const unsigned char* u2 = sqlite3_column_text(stmt, 3);
        const unsigned char* u3 = sqlite3_column_text(stmt, 4);
        const unsigned char* u4 = sqlite3_column_text(stmt, 5);
        snprintf(out->customerusername, sizeof(out->customerusername), "%s", u0 ? (const char*)u0 : "");
        snprintf(out->name, sizeof(out->name), "%s", u1 ? (const char*)u1 : "");
        snprintf(out->email, sizeof(out->email), "%s", u2 ? (const char*)u2 : "");
        snprintf(out->phone, sizeof(out->phone), "%s", u3 ? (const char*)u3 : "");
        snprintf(out->address, sizeof(out->address), "%s", u4 ? (const char*)u4 : "");
        sqlite3_finalize(stmt);
        return 1;
    }
    sqlite3_finalize(stmt);
    return 0;
}

static void printJson(const Customer* c) {
    // minimal escaping for quotes and backslashes
    // For simplicity, assume data is clean or does not include control characters
    printf("{\"id\":%d,\"customerusername\":\"%s\",\"name\":\"%s\",\"email\":\"%s\",\"phone\":\"%s\",\"address\":\"%s\"}\n",
           c->id, c->customerusername, c->name, c->email, c->phone, c->address);
}

int main(int argc, char** argv) {
    sqlite3* db = initDatabase();
    if (!db) return 1;

    if (argc > 1) {
        Customer c;
        if (getCustomerByUsername(db, argv[1], &c)) {
            printJson(&c);
        } else {
            printf("null\n");
        }
        sqlite3_close(db);
        return 0;
    }

    const char* tests[5] = {"alice", "bob", "charlie", "diana", "unknown"};
    for (int i = 0; i < 5; ++i) {
        Customer c;
        if (getCustomerByUsername(db, tests[i], &c)) {
            printJson(&c);
        } else {
            printf("null\n");
        }
    }

    sqlite3_close(db);
    return 0;
}