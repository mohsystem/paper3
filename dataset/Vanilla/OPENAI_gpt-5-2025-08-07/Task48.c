#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

// Simple dynamic message node
typedef struct MsgNode {
    char* text;
    struct MsgNode* next;
} MsgNode;

typedef struct {
    MsgNode* head;
    MsgNode* tail;
    pthread_mutex_t lock;
} ClientQueue;

typedef struct ClientEntry {
    char name[64];
    ClientQueue* inbox;
    struct ClientEntry* next;
} ClientEntry;

typedef struct {
    ClientEntry* head;
    pthread_mutex_t lock;
} ChatServer;

typedef struct {
    char name[64];
    ChatServer* server;
    ClientQueue inbox;
} ChatClient;

/* Queue utilities */
static void client_queue_init(ClientQueue* q) {
    q->head = q->tail = NULL;
    pthread_mutex_init(&q->lock, NULL);
}

static void client_queue_push(ClientQueue* q, const char* text) {
    MsgNode* n = (MsgNode*)malloc(sizeof(MsgNode));
    n->text = strdup(text);
    n->next = NULL;
    pthread_mutex_lock(&q->lock);
    if (q->tail) q->tail->next = n;
    else q->head = n;
    q->tail = n;
    pthread_mutex_unlock(&q->lock);
}

static char** client_queue_drain(ClientQueue* q, int* out_count) {
    pthread_mutex_lock(&q->lock);
    int count = 0;
    for (MsgNode* it = q->head; it; it = it->next) count++;
    char** arr = (char**)malloc(sizeof(char*) * count);
    int i = 0;
    MsgNode* it = q->head;
    while (it) {
        arr[i++] = it->text; // transfer ownership
        MsgNode* tmp = it;
        it = it->next;
        free(tmp);
    }
    q->head = q->tail = NULL;
    pthread_mutex_unlock(&q->lock);
    *out_count = count;
    return arr;
}

/* Server utilities */
static void chat_server_init(ChatServer* s) {
    s->head = NULL;
    pthread_mutex_init(&s->lock, NULL);
}

static int chat_server_register(ChatServer* s, const char* name, ClientQueue* inbox) {
    if (!name || !*name || !inbox) return 0;
    pthread_mutex_lock(&s->lock);
    for (ClientEntry* it = s->head; it; it = it->next) {
        if (strcmp(it->name, name) == 0) {
            pthread_mutex_unlock(&s->lock);
            return 0;
        }
    }
    ClientEntry* e = (ClientEntry*)malloc(sizeof(ClientEntry));
    strncpy(e->name, name, sizeof(e->name));
    e->name[sizeof(e->name)-1] = '\0';
    e->inbox = inbox;
    e->next = s->head;
    s->head = e;
    pthread_mutex_unlock(&s->lock);
    return 1;
}

static int chat_server_unregister(ChatServer* s, const char* name) {
    pthread_mutex_lock(&s->lock);
    ClientEntry* prev = NULL;
    ClientEntry* it = s->head;
    while (it) {
        if (strcmp(it->name, name) == 0) {
            if (prev) prev->next = it->next;
            else s->head = it->next;
            free(it);
            pthread_mutex_unlock(&s->lock);
            return 1;
        }
        prev = it;
        it = it->next;
    }
    pthread_mutex_unlock(&s->lock);
    return 0;
}

static void chat_server_broadcast(ChatServer* s, const char* from, const char* msg) {
    char buf[1024];
    snprintf(buf, sizeof(buf), "%s: %s", from, msg);
    pthread_mutex_lock(&s->lock);
    for (ClientEntry* it = s->head; it; it = it->next) {
        client_queue_push(it->inbox, buf);
    }
    pthread_mutex_unlock(&s->lock);
}

/* Client API */
static void chat_client_init(ChatClient* c, const char* name, ChatServer* server) {
    strncpy(c->name, name, sizeof(c->name));
    c->name[sizeof(c->name)-1] = '\0';
    c->server = server;
    client_queue_init(&c->inbox);
}

static int chat_client_connect(ChatClient* c) {
    return chat_server_register(c->server, c->name, &c->inbox);
}

static int chat_client_disconnect(ChatClient* c) {
    return chat_server_unregister(c->server, c->name);
}

static void chat_client_send(ChatClient* c, const char* message) {
    chat_server_broadcast(c->server, c->name, message);
}

static char** chat_client_receive_all(ChatClient* c, int* count) {
    return client_queue_drain(&c->inbox, count);
}

/* Helpers to print and free received arrays */
static void print_and_free_msgs(const char* who, char** msgs, int count) {
    printf("%s inbox:", who);
    for (int i = 0; i < count; ++i) {
        printf(" [%s]", msgs[i]);
        free(msgs[i]);
    }
    free(msgs);
    printf("\n");
}

/* Tests */
static void test1() {
    printf("Test1: Single client\n");
    ChatServer s; chat_server_init(&s);
    ChatClient a; chat_client_init(&a, "Alice", &s);
    chat_client_connect(&a);
    chat_client_send(&a, "Hello");
    int n; char** msgs = chat_client_receive_all(&a, &n);
    print_and_free_msgs("Alice", msgs, n);
}

static void test2() {
    printf("Test2: Two clients\n");
    ChatServer s; chat_server_init(&s);
    ChatClient a; chat_client_init(&a, "Alice", &s);
    ChatClient b; chat_client_init(&b, "Bob", &s);
    chat_client_connect(&a); chat_client_connect(&b);
    chat_client_send(&a, "Hi Bob");
    chat_client_send(&b, "Hi Alice");
    int na, nb; char** ma = chat_client_receive_all(&a, &na);
    char** mb = chat_client_receive_all(&b, &nb);
    print_and_free_msgs("Alice", ma, na);
    print_and_free_msgs("Bob", mb, nb);
}

static void test3() {
    printf("Test3: Three clients\n");
    ChatServer s; chat_server_init(&s);
    ChatClient a; chat_client_init(&a, "A", &s);
    ChatClient b; chat_client_init(&b, "B", &s);
    ChatClient c; chat_client_init(&c, "C", &s);
    chat_client_connect(&a); chat_client_connect(&b); chat_client_connect(&c);
    chat_client_send(&a, "One");
    chat_client_send(&b, "Two");
    chat_client_send(&c, "Three");
    int na, nb, nc;
    char** ma = chat_client_receive_all(&a, &na);
    char** mb = chat_client_receive_all(&b, &nb);
    char** mc = chat_client_receive_all(&c, &nc);
    print_and_free_msgs("A", ma, na);
    print_and_free_msgs("B", mb, nb);
    print_and_free_msgs("C", mc, nc);
}

static void test4() {
    printf("Test4: Disconnect\n");
    ChatServer s; chat_server_init(&s);
    ChatClient a; chat_client_init(&a, "A", &s);
    ChatClient b; chat_client_init(&b, "B", &s);
    chat_client_connect(&a); chat_client_connect(&b);
    chat_client_send(&a, "Before leave");
    chat_client_disconnect(&b);
    chat_client_send(&a, "After leave");
    int na, nb;
    char** ma = chat_client_receive_all(&a, &na);
    char** mb = chat_client_receive_all(&b, &nb);
    print_and_free_msgs("A", ma, na);
    print_and_free_msgs("B (only first expected)", mb, nb);
}

static void test5() {
    printf("Test5: Five clients\n");
    ChatServer s; chat_server_init(&s);
    ChatClient cs[5];
    char names[5][8];
    for (int i = 0; i < 5; ++i) {
        snprintf(names[i], sizeof(names[i]), "C%d", i);
        chat_client_init(&cs[i], names[i], &s);
        chat_client_connect(&cs[i]);
    }
    for (int i = 0; i < 5; ++i) {
        char msg[32];
        snprintf(msg, sizeof(msg), "Msg%d", i);
        chat_client_send(&cs[i], msg);
    }
    for (int i = 0; i < 5; ++i) {
        int n; char** m = chat_client_receive_all(&cs[i], &n);
        print_and_free_msgs(names[i], m, n);
    }
}

int main() {
    test1();
    test2();
    test3();
    test4();
    test5();
    return 0;
}