#include <stdio.h>
#include <string.h>

#define MAX_EMAILS 100
#define MAX_LEN 256

typedef struct {
    char emails[MAX_EMAILS][MAX_LEN];
    int count;
} Lse142;

int index_of(Lse142* db, const char* email) {
    for (int i = 0; i < db->count; ++i) {
        if (strcmp(db->emails[i], email) == 0) {
            return i;
        }
    }
    return -1;
}

const char* unsubscribe(Lse142* db, const char* email) {
    int idx = index_of(db, email);
    if (idx >= 0) {
        // Replace the removed element with the last one to keep O(1) deletion
        db->count--;
        if (idx < db->count) {
            strcpy(db->emails[idx], db->emails[db->count]);
        }
        return "Email removed from database.";
    } else {
        return "Email not found in database.";
    }
}

int main() {
    Lse142 service;
    service.count = 3;
    strcpy(service.emails[0], "alice@example.com");
    strcpy(service.emails[1], "bob@example.com");
    strcpy(service.emails[2], "carol@example.com");

    const char* tests[5] = {
        "alice@example.com",
        "dave@example.com",
        "bob@example.com",
        "alice@example.com",
        "carol@example.com"
    };

    for (int i = 0; i < 5; ++i) {
        const char* email = tests[i];
        printf("%s -> %s\n", email, unsubscribe(&service, email));
    }

    return 0;
}