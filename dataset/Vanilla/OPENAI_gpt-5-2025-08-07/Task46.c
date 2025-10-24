#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define DB_FILE "users_c.csv"
#define HEADER "name,email,password"
#define MAX_LINE 4096

static void init_db() {
    FILE* f = fopen(DB_FILE, "r");
    if (f) {
        fclose(f);
        return;
    }
    f = fopen(DB_FILE, "w");
    if (!f) return;
    fprintf(f, "%s\n", HEADER);
    fclose(f);
}

static void sanitize(char* s) {
    if (!s) return;
    size_t len = strlen(s);
    size_t i, j = 0;
    char* buf = (char*)malloc(len + 1);
    if (!buf) return;
    for (i = 0; i < len; ++i) {
        char c = s[i];
        if (c == '\n' || c == '\r' || c == ',') c = ' ';
        buf[j++] = c;
    }
    buf[j] = '\0';
    // trim
    size_t start = 0, end = j;
    while (start < end && (buf[start] == ' ' || buf[start] == '\t')) start++;
    while (end > start && (buf[end-1] == ' ' || buf[end-1] == '\t')) end--;
    size_t k = 0;
    for (i = start; i < end; ++i) s[k++] = buf[i];
    s[k] = '\0';
    free(buf);
}

static int is_valid_email(const char* email) {
    const char* at = strchr(email, '@');
    if (!at) return 0;
    if (at == email) return 0;
    const char* dot = strchr(at + 1, '.');
    if (!dot) return 0;
    if (*(dot + 1) == '\0') return 0;
    for (const char* p = email; *p; ++p) if (*p == ' ') return 0;
    return 1;
}

static int email_exists(const char* email) {
    init_db();
    FILE* f = fopen(DB_FILE, "r");
    if (!f) return 0;
    char line[MAX_LINE];
    int first = 1;
    while (fgets(line, sizeof(line), f)) {
        if (first) { first = 0; continue; }
        char* name = strtok(line, ",");
        char* em = strtok(NULL, ",");
        // remove newline from last token if present
        if (em) {
            // trim trailing newline/spaces
            char* p = em + strlen(em) - 1;
            while (p >= em && (*p == '\n' || *p == '\r' || *p == ' ' || *p == '\t')) { *p = '\0'; p--; }
            // case-insensitive compare
            const char* a = em;
            const char* b = email;
            int eq = 1;
            while (*a && *b) {
                if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) { eq = 0; break; }
                a++; b++;
            }
            if (eq && *a == '\0' && *b == '\0') { fclose(f); return 1; }
        }
    }
    fclose(f);
    return 0;
}

int register_user(const char* name_in, const char* email_in, const char* password_in) {
    init_db();

    if (!name_in || !email_in || !password_in) return 0;

    char name[1024], email[1024], password[1024];
    strncpy(name, name_in, sizeof(name)-1); name[sizeof(name)-1] = '\0';
    strncpy(email, email_in, sizeof(email)-1); email[sizeof(email)-1] = '\0';
    strncpy(password, password_in, sizeof(password)-1); password[sizeof(password)-1] = '\0';

    sanitize(name);
    sanitize(email);
    sanitize(password);

    if (strlen(name) == 0 || strlen(email) == 0 || strlen(password) == 0) return 0;
    if (!is_valid_email(email)) return 0;
    if (strlen(password) < 6) return 0;
    if (email_exists(email)) return 0;

    FILE* f = fopen(DB_FILE, "a");
    if (!f) return 0;
    fprintf(f, "%s,%s,%s\n", name, email, password);
    fclose(f);
    return 1;
}

typedef struct {
    char name[1024];
    char email[1024];
    char password[1024];
} User;

User* get_all_users(size_t* count) {
    init_db();
    *count = 0;
    FILE* f = fopen(DB_FILE, "r");
    if (!f) return NULL;

    // First pass: count lines
    char line[MAX_LINE];
    int first = 1;
    size_t n = 0;
    while (fgets(line, sizeof(line), f)) {
        if (first) { first = 0; continue; }
        n++;
    }
    rewind(f);

    User* users = (User*)malloc(n * sizeof(User));
    if (!users) { fclose(f); return NULL; }

    first = 1;
    size_t idx = 0;
    while (fgets(line, sizeof(line), f)) {
        if (first) { first = 0; continue; }
        char* name = strtok(line, ",");
        char* email = strtok(NULL, ",");
        char* password = strtok(NULL, ",");
        if (!name || !email || !password) continue;

        // trim newline from password
        char* p = password + strlen(password) - 1;
        while (p >= password && (*p == '\n' || *p == '\r')) { *p = '\0'; p--; }

        strncpy(users[idx].name, name, sizeof(users[idx].name)-1); users[idx].name[sizeof(users[idx].name)-1] = '\0';
        strncpy(users[idx].email, email, sizeof(users[idx].email)-1); users[idx].email[sizeof(users[idx].email)-1] = '\0';
        strncpy(users[idx].password, password, sizeof(users[idx].password)-1); users[idx].password[sizeof(users[idx].password)-1] = '\0';
        idx++;
    }
    fclose(f);
    *count = idx;
    return users;
}

int main() {
    printf("Test 1 (valid): %s\n", register_user("Alice", "alice@example.com", "pass123") ? "true" : "false");
    printf("Test 2 (duplicate email): %s\n", register_user("Alicia", "alice@example.com", "differentPass") ? "true" : "false");
    printf("Test 3 (invalid email): %s\n", register_user("Bob", "bob_at_example.com", "secret12") ? "true" : "false");
    printf("Test 4 (short password): %s\n", register_user("Charlie", "charlie@example.com", "123") ? "true" : "false");
    printf("Test 5 (valid): %s\n", register_user("Dana", "dana@example.org", "StrongPass9") ? "true" : "false");

    size_t count = 0;
    User* users = get_all_users(&count);
    printf("All users:\n");
    for (size_t i = 0; i < count; ++i) {
        printf("User{name='%s', email='%s', password='%s'}\n", users[i].name, users[i].email, users[i].password);
    }
    free(users);
    return 0;
}