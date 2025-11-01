#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char* username;
    char* message;
} Message;

static Message DB[1024];
static int DB_COUNT = 0;

void insert_user_message_in_db(const char* username, const char* message) {
    if (DB_COUNT >= 1024) return;
    const char* u = (username ? username : "");
    const char* m = (message ? message : "");
    DB[DB_COUNT].username = (char*)malloc(strlen(u) + 1);
    DB[DB_COUNT].message = (char*)malloc(strlen(m) + 1);
    if (DB[DB_COUNT].username) strcpy(DB[DB_COUNT].username, u);
    if (DB[DB_COUNT].message) strcpy(DB[DB_COUNT].message, m);
    DB_COUNT++;
}

const char* post(const char* username, const char* message) {
    insert_user_message_in_db(username, message);
    return "REDIRECT:/";
}

const char* main_page() {
    return "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>Main</title></head><body>"
           "<h1>Message Board</h1>"
           "<form method=\"post\" action=\"/post\">"
           "Username: <input type=\"text\" name=\"username\"/><br/>"
           "Message: <textarea name=\"message\"></textarea><br/>"
           "<button type=\"submit\">Submit</button>"
           "</form>"
           "</body></html>";
}

int main() {
    printf("%s\n", main_page());
    printf("%s\n", post("alice", "Hello!"));
    printf("%s\n", post("bob", "Hi there"));
    printf("%s\n", post("carol", "This is a test"));
    printf("%s\n", post("dave", "Another message"));
    printf("%s\n", post("eve", "Last one"));
    printf("%s\n", main_page());

    // Optional cleanup (not strictly necessary on short-lived process)
    for (int i = 0; i < DB_COUNT; ++i) {
        free(DB[i].username);
        free(DB[i].message);
    }
    return 0;
}