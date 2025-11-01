#include <stdio.h>
#include <string.h>

#define MAX_USERS 100
#define MAX_MSG_LEN 256

static const char* initial_users[] = {
    "alice@example.com",
    "bob@example.com",
    "dana@example.com"
};

static const int initial_users_count = 3;

static const char* database[MAX_USERS];
static int db_count = 0;

static const char* mailing_list[MAX_USERS];
static int ml_count = 0;

void initializeDB() {
    db_count = 0;
    ml_count = 0;
    for (int i = 0; i < initial_users_count; ++i) {
        database[db_count++] = initial_users[i];
        mailing_list[ml_count++] = initial_users[i];
    }
}

int find_index(const char* arr[], int count, const char* email) {
    for (int i = 0; i < count; ++i) {
        if (arr[i] && strcmp(arr[i], email) == 0) {
            return i;
        }
    }
    return -1;
}

void remove_at(char* const_arr[], int* count, int idx) {
    // Not used for const, keep signature separate to avoid warnings
}

void remove_at_const(const char* arr[], int* count, int idx) {
    if (idx < 0 || idx >= *count) return;
    for (int i = idx; i < *count - 1; ++i) {
        arr[i] = arr[i + 1];
    }
    (*count)--;
}

const char* unsubscribe(const char* email) {
    static char msg[MAX_MSG_LEN];
    if (email == NULL || email[0] == '\0') {
        snprintf(msg, sizeof(msg), "Invalid email");
        return msg;
    }
    int idx = find_index(database, db_count, email);
    if (idx != -1) {
        remove_at_const(database, &db_count, idx);
        int ml_idx = find_index(mailing_list, ml_count, email);
        if (ml_idx != -1) {
            remove_at_const(mailing_list, &ml_count, ml_idx);
        }
        snprintf(msg, sizeof(msg), "Unsubscribed: %s", email);
        return msg;
    } else {
        snprintf(msg, sizeof(msg), "User not subscribed: %s", email);
        return msg;
    }
}

int main() {
    initializeDB();
    const char* tests[5] = {
        "alice@example.com",
        "bob@example.com",
        "charlie@example.com",
        "bob@example.com",
        "dana@example.com"
    };
    for (int i = 0; i < 5; ++i) {
        printf("%s\n", unsubscribe(tests[i]));
    }
    return 0;
}