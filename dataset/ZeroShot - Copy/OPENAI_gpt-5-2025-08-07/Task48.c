#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_CLIENTS 32
#define MAX_NAME 64
#define MAX_MESSAGE 256
#define MAX_INBOX 256
#define MAX_BROADCAST_LEN (MAX_NAME + 2 + MAX_MESSAGE) /* "name: message" */
#define TRUE 1
#define FALSE 0

typedef struct {
    int active;
    int id;
    char name[MAX_NAME];
    char inbox[MAX_INBOX][MAX_BROADCAST_LEN + 1];
    int inbox_count;
} Client;

typedef struct {
    Client clients[MAX_CLIENTS];
    int next_id;
} ChatServer;

/* Helper: secure string copy with null-termination */
static void scpy(char *dst, size_t dstsz, const char *src) {
    if (dstsz == 0) return;
    if (src == NULL) {
        dst[0] = '\0';
        return;
    }
    strncpy(dst, src, dstsz - 1);
    dst[dstsz - 1] = '\0';
}

/* Sanitize message: keep printable ASCII, map 
\r\t to space, trim, limit length */
static void sanitize_message(const char *in, char *out, size_t outsz) {
    if (outsz == 0) return;
    if (in == NULL) {
        out[0] = '\0';
        return;
    }
    size_t maxLen = MAX_MESSAGE;
    size_t count = 0;
    for (size_t i = 0; in[i] != '\0' && count < maxLen && count + 1 < outsz; i++) {
        unsigned char u = (unsigned char)in[i];
        if (u >= 32 && u != 127) {
            out[count++] = (char)u;
        } else if (in[i] == '\n' || in[i] == '\r' || in[i] == '\t') {
            out[count++] = ' ';
        } /* else drop */
    }
    out[count] = '\0';
    /* trim spaces */
    size_t start = 0;
    while (out[start] == ' ') start++;
    size_t end = strlen(out);
    while (end > start && out[end - 1] == ' ') end--;
    size_t len = end - start;
    if (start > 0 && len > 0) {
        memmove(out, out + start, len);
    }
    out[len] = '\0';
}

/* Sanitize name: visible ASCII (33..126), exclude ':' */
static void sanitize_name(const char *in, char *out, size_t outsz) {
    if (outsz == 0) return;
    if (in == NULL) {
        out[0] = '\0';
        return;
    }
    size_t count = 0;
    for (size_t i = 0; in[i] != '\0' && count + 1 < outsz && count < 32; i++) {
        unsigned char u = (unsigned char)in[i];
        if (u >= 33 && u < 127 && in[i] != ':') {
            out[count++] = in[i];
        }
    }
    out[count] = '\0';
    /* trim spaces */
    size_t start = 0;
    while (out[start] == ' ') start++;
    size_t end = strlen(out);
    while (end > start && out[end - 1] == ' ') end--;
    size_t len = end - start;
    if (start > 0 && len > 0) {
        memmove(out, out + start, len);
    }
    out[len] = '\0';
}

void init_server(ChatServer *s) {
    if (!s) return;
    memset(s, 0, sizeof(*s));
    s->next_id = 1;
}

int register_client(ChatServer *s, const char *name) {
    if (!s) return -1;
    int slot = -1;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!s->clients[i].active) {
            slot = i;
            break;
        }
    }
    if (slot < 0) return -1;
    char safe_name[MAX_NAME];
    sanitize_name(name, safe_name, sizeof(safe_name));
    if (safe_name[0] == '\0') {
        snprintf(safe_name, sizeof(safe_name), "user%d", s->next_id);
    }
    Client *c = &s->clients[slot];
    c->active = TRUE;
    c->id = s->next_id++;
    scpy(c->name, sizeof(c->name), safe_name);
    c->inbox_count = 0;
    return c->id;
}

int disconnect_client(ChatServer *s, int client_id) {
    if (!s) return FALSE;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (s->clients[i].active && s->clients[i].id == client_id) {
            s->clients[i].active = FALSE;
            s->clients[i].id = 0;
            s->clients[i].name[0] = '\0';
            s->clients[i].inbox_count = 0;
            return TRUE;
        }
    }
    return FALSE;
}

static Client* find_client(ChatServer *s, int client_id) {
    if (!s) return NULL;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (s->clients[i].active && s->clients[i].id == client_id) {
            return &s->clients[i];
        }
    }
    return NULL;
}

int get_client_count(ChatServer *s) {
    if (!s) return 0;
    int count = 0;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (s->clients[i].active) count++;
    }
    return count;
}

/* Returns number of recipients */
int send_from(ChatServer *s, int client_id, const char *message) {
    if (!s) return 0;
    Client *sender = find_client(s, client_id);
    if (!sender) return 0;

    char sanitized[MAX_MESSAGE + 4];
    sanitize_message(message, sanitized, sizeof(sanitized));
    if (sanitized[0] == '\0') return 0;

    char payload[MAX_BROADCAST_LEN + 1];
    payload[0] = '\0';
    /* Build "name: message" safely */
    size_t used = 0;
    int n = snprintf(payload + used, sizeof(payload) - used, "%s: ", sender->name);
    if (n < 0) return 0;
    used += (size_t)n;
    if (used >= sizeof(payload)) used = sizeof(payload) - 1;
    n = snprintf(payload + used, sizeof(payload) - used, "%s", sanitized);
    if (n < 0) return 0;
    used += (size_t)n;
    if (used >= sizeof(payload)) used = sizeof(payload) - 1;
    payload[used] = '\0';

    int count = 0;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (s->clients[i].active) {
            Client *c = &s->clients[i];
            if (c->inbox_count >= MAX_INBOX) {
                /* drop oldest to make room */
                for (int k = 1; k < MAX_INBOX; k++) {
                    scpy(c->inbox[k - 1], sizeof(c->inbox[k - 1]), c->inbox[k]);
                }
                c->inbox_count = MAX_INBOX - 1;
            }
            scpy(c->inbox[c->inbox_count], sizeof(c->inbox[c->inbox_count]), payload);
            c->inbox_count++;
            count++;
        }
    }
    return count;
}

/* Fetch messages into out buffer; returns number of messages copied (up to max_out) */
int fetch_all(ChatServer *s, int client_id, char out[][MAX_BROADCAST_LEN + 1], int max_out) {
    if (!s || !out || max_out <= 0) return 0;
    Client *c = find_client(s, client_id);
    if (!c) return 0;
    int n = c->inbox_count;
    if (n > max_out) n = max_out;
    for (int i = 0; i < n; i++) {
        scpy(out[i], MAX_BROADCAST_LEN + 1, c->inbox[i]);
    }
    /* shift remaining */
    int remaining = c->inbox_count - n;
    for (int i = 0; i < remaining; i++) {
        scpy(c->inbox[i], sizeof(c->inbox[i]), c->inbox[i + n]);
    }
    c->inbox_count = remaining;
    return n;
}

static void print_inbox(const char *label, char msgs[][MAX_BROADCAST_LEN + 1], int count) {
    printf("%s (%d):\n", label, count);
    for (int i = 0; i < count; i++) {
        printf("  %s\n", msgs[i]);
    }
}

int main(void) {
    ChatServer server;
    init_server(&server);

    int alice = register_client(&server, "Alice");
    int bob = register_client(&server, "Bob");
    int charlie = register_client(&server, "Charlie");

    char buf[512][MAX_BROADCAST_LEN + 1];

    printf("Test 1: Single client receives own message\n");
    send_from(&server, alice, "Hello everyone!");
    int n1 = fetch_all(&server, alice, buf, 512);
    print_inbox("Alice inbox after send", buf, n1);
    int n2 = fetch_all(&server, bob, buf, 512);
    print_inbox("Bob inbox", buf, n2);
    int n3 = fetch_all(&server, charlie, buf, 512);
    print_inbox("Charlie inbox", buf, n3);
    printf("----\n");

    printf("Test 2: Control chars sanitized\n");
    send_from(&server, bob, "Hi\nthere\tall!");
    n1 = fetch_all(&server, alice, buf, 512);
    print_inbox("Alice inbox", buf, n1);
    n2 = fetch_all(&server, bob, buf, 512);
    print_inbox("Bob inbox", buf, n2);
    n3 = fetch_all(&server, charlie, buf, 512);
    print_inbox("Charlie inbox", buf, n3);
    printf("----\n");

    printf("Test 3: Long message truncated\n");
    char longMsg[401];
    for (int i = 0; i < 400; i++) longMsg[i] = 'X';
    longMsg[400] = '\0';
    send_from(&server, charlie, longMsg);
    n1 = fetch_all(&server, alice, buf, 512);
    print_inbox("Alice inbox", buf, n1);
    printf("----\n");

    printf("Test 4: Empty/invalid message rejected\n");
    int count = send_from(&server, alice, "\x01\x02\x03");
    printf("Broadcast count (should be 0): %d\n", count);
    n2 = fetch_all(&server, bob, buf, 512);
    print_inbox("Bob inbox", buf, n2);
    printf("----\n");

    printf("Test 5: Disconnect behavior\n");
    disconnect_client(&server, charlie);
    send_from(&server, bob, "After disconnect");
    n1 = fetch_all(&server, alice, buf, 512);
    print_inbox("Alice inbox", buf, n1);
    n3 = fetch_all(&server, charlie, buf, 512);
    print_inbox("Charlie inbox (should be empty)", buf, n3);
    printf("Active clients: %d\n", get_client_count(&server));

    return 0;
}