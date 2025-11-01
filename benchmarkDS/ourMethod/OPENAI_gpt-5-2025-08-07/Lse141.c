#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
  Minimal "web app" placeholder: In constrained C environments without external
  frameworks, we represent a web app by an initialization function. Networking
  could be added with BSD sockets if needed, but is omitted here to keep the
  single-file, portable example straightforward.
*/

typedef struct {
    char* host;
    unsigned short port;
    int running;
} WebApp;

static void free_str(char** p) {
    if (p && *p) {
        // avoid printing secrets; clear then free
#if defined(__STDC_LIB_EXT1__)
        memset_s(*p, strlen(*p), 0, strlen(*p));
#else
        volatile unsigned char *vp = (volatile unsigned char*)*p;
        size_t n = strlen(*p);
        while (n--) { *vp++ = 0; }
#endif
        free(*p);
        *p = NULL;
    }
}

static WebApp create_web_app(unsigned short port) {
    WebApp app;
    app.host = (char*)malloc(10);
    if (app.host) {
        strcpy(app.host, "127.0.0.1");
    }
    app.port = port == 0 ? 8080 : port; // placeholder port
    app.running = 1;
    return app;
}

static void stop_web_app(WebApp* app) {
    if (!app) return;
    app->running = 0;
    free_str(&app->host);
}

// Securely read env (no hardcoded secrets)
static char* get_server_name(void) {
    const char* v = getenv("DB_HOST");
    const char* def = "127.0.0.1";
    const char* src = (v && v[0] != '\0') ? v : def;
    size_t len = strlen(src);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, src, len);
    out[len] = '\0';
    return out;
}

static char* get_database_name(void) {
    const char* v = getenv("DB_NAME");
    const char* def = "appdb";
    const char* src = (v && v[0] != '\0') ? v : def;
    size_t len = strlen(src);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, src, len);
    out[len] = '\0';
    return out;
}

static char* get_database_user(void) {
    const char* v = getenv("DB_USER");
    const char* src = (v ? v : "");
    size_t len = strlen(src);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, src, len);
    out[len] = '\0';
    return out;
}

// Do not print or log this value
static char* get_database_password(void) {
    const char* v = getenv("DB_PASSWORD");
    const char* src = (v ? v : "");
    size_t len = strlen(src);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, src, len);
    out[len] = '\0';
    return out;
}

typedef struct {
    char* host;
    char* db;
    char* user;
} DBConnection;

static void free_db_connection(DBConnection** pc) {
    if (!pc || !*pc) return;
    DBConnection* c = *pc;
    free_str(&c->host);
    free_str(&c->db);
    free_str(&c->user);
    free(c);
    *pc = NULL;
}

// Returns a "connection object" only if explicitly enabled. No real DB I/O here.
static DBConnection* get_database_connection(void) {
    const char* flag = getenv("DB_CONNECT");
    if (!(flag && strcmp(flag, "true") == 0)) {
        return NULL;
    }
    DBConnection* c = (DBConnection*)calloc(1, sizeof(DBConnection));
    if (!c) return NULL;

    c->host = get_server_name();
    c->db = get_database_name();
    c->user = get_database_user();
    if (!c->host || !c->db || !c->user) {
        free_db_connection(&c);
        return NULL;
    }
    return c;
}

int main(void) {
    // Start "web app" placeholder
    WebApp app = create_web_app(0);
    printf("[Test1] Web app initialized on %s:%u\n", app.host ? app.host : "(null)", app.port);

    // Test2: Server name
    char* host = get_server_name();
    printf("[Test2] DB Server Name: %s\n", host ? host : "(null)");

    // Test3: Database name
    char* db = get_database_name();
    printf("[Test3] DB Name: %s\n", db ? db : "(null)");

    // Test4: Database user
    char* user = get_database_user();
    if (user && user[0] != '\0') {
        printf("[Test4] DB User: %s\n", user);
    } else {
        printf("[Test4] DB User: (not set)\n");
    }

    // Test5: Password presence (do not print secret)
    char* pwd = get_database_password();
    printf("[Test5] DB Password set: %s\n", (pwd && pwd[0] != '\0') ? "true" : "false");

    // Optional: connection object (no real DB connection)
    DBConnection* c = get_database_connection();
    if (c) {
        printf("[Extra] Connection object prepared for %s@%s/%s\n",
               c->user ? c->user : "(null)",
               c->host ? c->host : "(null)",
               c->db ? c->db : "(null)");
        free_db_connection(&c);
    } else {
        printf("[Extra] Connection not prepared (expected unless DB_CONNECT=true)\n");
    }

    // Cleanup
    free_str(&host);
    free_str(&db);
    free_str(&user);
    free_str(&pwd);
    stop_web_app(&app);
    printf("[Done] Web app stopped.\n");
    return 0;
}