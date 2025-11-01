#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_USERS 128
#define MAX_EMAIL_LEN 256

static char* users_db[MAX_USERS];
static int users_db_count = 0;

static char* mailing_list[MAX_USERS];
static int mailing_list_count = 0;

static void add_user(const char* email) {
    if (users_db_count >= MAX_USERS || mailing_list_count >= MAX_USERS) return;
    char* e = strdup(email);
    char* m = strdup(email);
    users_db[users_db_count++] = e;
    mailing_list[mailing_list_count++] = m;
}

static void init_db() {
    add_user("alice@example.com");
    add_user("bob@example.com");
    add_user("carol@example.com");
}

static void normalize_email(const char* in, char* out, size_t outsz) {
    if (!in) { if (outsz) out[0] = '\0'; return; }
    // trim leading spaces
    const char* l = in;
    while (*l && isspace((unsigned char)*l)) l++;
    // find end, trim trailing
    const char* r = l + strlen(l);
    while (r > l && isspace((unsigned char)r[-1])) r--;
    size_t len = (size_t)(r - l);
    if (len >= outsz) len = outsz - 1;
    for (size_t i = 0; i < len; ++i) {
        out[i] = (char)tolower((unsigned char)l[i]);
    }
    out[len] = '\0';
}

static int find_in_array(char* arr[], int count, const char* email) {
    for (int i = 0; i < count; ++i) {
        if (arr[i] && strcmp(arr[i], email) == 0) return i;
    }
    return -1;
}

static void remove_at(char* arr[], int* count, int idx) {
    if (idx < 0 || idx >= *count) return;
    free(arr[idx]);
    for (int i = idx; i < *count - 1; ++i) {
        arr[i] = arr[i + 1];
    }
    (*count)--;
    arr[*count] = NULL;
}

// Simulates handling a GET /unsubscribe?email=<email>
const char* unsubscribe_get(const char* email) {
    static char result[256];
    if (email == NULL) {
        snprintf(result, sizeof(result), "Invalid email parameter.");
        return result;
    }
    // Check if email is only whitespace
    int only_space = 1;
    for (const char* p = email; *p; ++p) {
        if (!isspace((unsigned char)*p)) { only_space = 0; break; }
    }
    if (only_space) {
        snprintf(result, sizeof(result), "Invalid email parameter.");
        return result;
    }

    char norm[MAX_EMAIL_LEN];
    normalize_email(email, norm, sizeof(norm));

    int idx_db = find_in_array(users_db, users_db_count, norm);
    if (idx_db >= 0) {
        remove_at(users_db, &users_db_count, idx_db);      // delete user from the "database"
        int idx_ml = find_in_array(mailing_list, mailing_list_count, norm);
        if (idx_ml >= 0) {
            remove_at(mailing_list, &mailing_list_count, idx_ml); // unsubscribe from mailing list
        }
        snprintf(result, sizeof(result), "%s has been unsubscribed.", norm);
    } else {
        snprintf(result, sizeof(result), "%s is not subscribed.", norm);
    }
    return result;
}

int main(void) {
    init_db();

    const char* tests[5] = {
        "alice@example.com",   // subscribed -> unsubscribe
        "dave@example.com",    // not subscribed
        "bob@example.com",     // subscribed -> unsubscribe
        "bob@example.com",     // already unsubscribed
        "Carol@Example.com"    // case-insensitive match -> unsubscribe
    };

    for (int i = 0; i < 5; ++i) {
        printf("%s\n", unsubscribe_get(tests[i]));
    }

    // Cleanup
    for (int i = 0; i < users_db_count; ++i) free(users_db[i]);
    for (int i = 0; i < mailing_list_count; ++i) free(mailing_list[i]);

    return 0;
}