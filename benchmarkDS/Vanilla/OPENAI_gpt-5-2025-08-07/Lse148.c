#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char* username;
    char* content;
} Message;

static Message* DB = NULL;
static size_t DB_SIZE = 0;
static size_t DB_CAP = 0;

static void ensure_capacity(size_t extra) {
    if (DB_SIZE + extra > DB_CAP) {
        size_t newcap = DB_CAP ? DB_CAP * 2 : 8;
        while (newcap < DB_SIZE + extra) newcap *= 2;
        Message* nd = (Message*)realloc(DB, newcap * sizeof(Message));
        if (!nd) {
            fprintf(stderr, "Out of memory\n");
            exit(1);
        }
        DB = nd;
        DB_CAP = newcap;
    }
}

int insert_user_message_in_db(const char* username, const char* message) {
    if (!username) username = "";
    if (!message) message = "";
    ensure_capacity(1);
    DB[DB_SIZE].username = strdup(username);
    DB[DB_SIZE].content = strdup(message);
    if (!DB[DB_SIZE].username || !DB[DB_SIZE].content) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    DB_SIZE++;
    return 1;
}

const char* post(const char* username, const char* message) {
    insert_user_message_in_db(username, message);
    return "REDIRECT:/";
}

char* display_messages(void) {
    size_t total = 1;
    for (size_t i = 0; i < DB_SIZE; ++i) {
        total += 20 + 2 + strlen(DB[i].username) + 2 + strlen(DB[i].content) + 1;
    }
    char* buf = (char*)malloc(total);
    if (!buf) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    buf[0] = '\0';
    char line[1024];
    for (size_t i = 0; i < DB_SIZE; ++i) {
        snprintf(line, sizeof(line), "%zu. %s: %s", i + 1, DB[i].username, DB[i].content);
        strcat(buf, line);
        if (i + 1 < DB_SIZE) strcat(buf, "\n");
    }
    return buf;
}

static void free_db(void) {
    for (size_t i = 0; i < DB_SIZE; ++i) {
        free(DB[i].username);
        free(DB[i].content);
    }
    free(DB);
    DB = NULL;
    DB_SIZE = DB_CAP = 0;
}

int main(void) {
    printf("%s\n", post("alice", "Hello world"));
    printf("%s\n", post("bob", "Hi Alice"));
    printf("%s\n", post("carol", "Good morning"));
    printf("%s\n", post("dave", "How are you?"));
    printf("%s\n", post("eve", "All good!"));
    printf("Main page messages:\n");
    char* s = display_messages();
    printf("%s\n", s);
    free(s);
    free_db();
    return 0;
}