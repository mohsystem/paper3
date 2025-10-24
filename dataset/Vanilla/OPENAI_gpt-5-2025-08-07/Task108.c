// POSIX C single-file TCP server allowing remote clients to manipulate server-side objects.
// Build: cc -pthread task108.c -o task108
// Run: ./task108
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 5050
#define MAX_OBJECTS 100
#define MAX_PROPS 100
#define ID_LEN 64
#define KEY_LEN 64
#define VAL_LEN 512
#define LINE_LEN 2048

typedef struct {
    int used;
    char key[KEY_LEN];
    char value[VAL_LEN];
} Prop;

typedef struct {
    int used;
    char id[ID_LEN];
    Prop props[MAX_PROPS];
} Object;

static Object objects[MAX_OBJECTS];
static pthread_mutex_t store_mtx = PTHREAD_MUTEX_INITIALIZER;
static volatile int server_running = 1;
static int listen_fd_global = -1;

static void trim_newline(char* s) {
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r')) s[--n] = '\0';
}

static int find_object_index(const char* id) {
    for (int i = 0; i < MAX_OBJECTS; ++i) {
        if (objects[i].used && strcmp(objects[i].id, id) == 0) return i;
    }
    return -1;
}
static int create_object(const char* id) {
    int idx = find_object_index(id);
    if (idx >= 0) return idx;
    for (int i = 0; i < MAX_OBJECTS; ++i) {
        if (!objects[i].used) {
            objects[i].used = 1;
            strncpy(objects[i].id, id, ID_LEN - 1);
            objects[i].id[ID_LEN - 1] = '\0';
            for (int j = 0; j < MAX_PROPS; ++j) objects[i].props[j].used = 0;
            return i;
        }
    }
    return -1;
}
static int set_prop_idx(int objIdx, const char* key, const char* value) {
    for (int j = 0; j < MAX_PROPS; ++j) {
        if (objects[objIdx].props[j].used && strcmp(objects[objIdx].props[j].key, key) == 0) {
            strncpy(objects[objIdx].props[j].value, value, VAL_LEN - 1);
            objects[objIdx].props[j].value[VAL_LEN - 1] = '\0';
            return 0;
        }
    }
    for (int j = 0; j < MAX_PROPS; ++j) {
        if (!objects[objIdx].props[j].used) {
            objects[objIdx].props[j].used = 1;
            strncpy(objects[objIdx].props[j].key, key, KEY_LEN - 1);
            objects[objIdx].props[j].key[KEY_LEN - 1] = '\0';
            strncpy(objects[objIdx].props[j].value, value, VAL_LEN - 1);
            objects[objIdx].props[j].value[VAL_LEN - 1] = '\0';
            return 0;
        }
    }
    return -1;
}
static int get_prop_idx(int objIdx, const char* key, char* out, size_t outlen) {
    for (int j = 0; j < MAX_PROPS; ++j) {
        if (objects[objIdx].props[j].used && strcmp(objects[objIdx].props[j].key, key) == 0) {
            strncpy(out, objects[objIdx].props[j].value, outlen - 1);
            out[outlen - 1] = '\0';
            return 0;
        }
    }
    return -1;
}
static void delete_object(const char* id) {
    int idx = find_object_index(id);
    if (idx >= 0) {
        objects[idx].used = 0;
    }
}
static void list_objects(char* out, size_t outlen) {
    out[0] = '\0';
    int first = 1;
    for (int i = 0; i < MAX_OBJECTS; ++i) {
        if (objects[i].used) {
            if (!first) strncat(out, ",", outlen - strlen(out) - 1);
            strncat(out, objects[i].id, outlen - strlen(out) - 1);
            first = 0;
        }
    }
}
static void keys_object(int objIdx, char* out, size_t outlen) {
    out[0] = '\0';
    int first = 1;
    for (int j = 0; j < MAX_PROPS; ++j) {
        if (objects[objIdx].props[j].used) {
            if (!first) strncat(out, ",", outlen - strlen(out) - 1);
            strncat(out, objects[objIdx].props[j].key, outlen - strlen(out) - 1);
            first = 0;
        }
    }
}

static int read_line(int fd, char* buf, size_t buflen) {
    size_t pos = 0;
    while (pos + 1 < buflen) {
        char c;
        ssize_t n = recv(fd, &c, 1, 0);
        if (n == 0) return 0;
        if (n < 0) return -1;
        if (c == '\n') break;
        buf[pos++] = c;
    }
    buf[pos] = '\0';
    return 1;
}
static int write_line(int fd, const char* s) {
    size_t len = strlen(s);
    if (send(fd, s, len, 0) < 0) return -1;
    if (send(fd, "\n", 1, 0) < 0) return -1;
    return 0;
}

static void* client_handler(void* arg) {
    int cfd = *(int*)arg;
    free(arg);
    char line[LINE_LEN];
    while (1) {
        int r = read_line(cfd, line, sizeof(line));
        if (r <= 0) break;
        trim_newline(line);

        char resp[LINE_LEN];
        resp[0] = '\0';

        // Parse command
        char cmd[16] = {0};
        sscanf(line, "%15s", cmd);
        for (char* p = cmd; *p; ++p) if (*p >= 'a' && *p <= 'z') *p = (char)(*p - 32);

        if (strcmp(cmd, "CREATE") == 0) {
            char id[ID_LEN] = {0};
            if (sscanf(line, "CREATE %63s", id) != 1) {
                snprintf(resp, sizeof(resp), "ERROR Usage: CREATE <id>");
            } else {
                pthread_mutex_lock(&store_mtx);
                create_object(id);
                pthread_mutex_unlock(&store_mtx);
                snprintf(resp, sizeof(resp), "OK");
            }
        } else if (strcmp(cmd, "SET") == 0) {
            // Extract id, key, and the rest as value
            char id[ID_LEN] = {0}, key[KEY_LEN] = {0};
            const char* p = strchr(line, ' ');
            if (!p) { snprintf(resp, sizeof(resp), "ERROR Usage: SET <id> <key> <value>"); goto writeout; }
            p++;
            const char* p2 = strchr(p, ' ');
            if (!p2) { snprintf(resp, sizeof(resp), "ERROR Usage: SET <id> <key> <value>"); goto writeout; }
            size_t len_id = (size_t)(p2 - p);
            if (len_id >= sizeof(id)) len_id = sizeof(id) - 1;
            strncpy(id, p, len_id); id[len_id] = '\0';

            const char* p3 = strchr(p2 + 1, ' ');
            if (!p3) { snprintf(resp, sizeof(resp), "ERROR Usage: SET <id> <key> <value>"); goto writeout; }
            size_t len_key = (size_t)(p3 - (p2 + 1));
            if (len_key >= sizeof(key)) len_key = sizeof(key) - 1;
            strncpy(key, p2 + 1, len_key); key[len_key] = '\0';

            const char* value = p3 + 1;
            pthread_mutex_lock(&store_mtx);
            int idx = find_object_index(id);
            if (idx < 0) {
                pthread_mutex_unlock(&store_mtx);
                snprintf(resp, sizeof(resp), "ERROR NotFound");
            } else {
                set_prop_idx(idx, key, value);
                pthread_mutex_unlock(&store_mtx);
                snprintf(resp, sizeof(resp), "OK");
            }
        } else if (strcmp(cmd, "GET") == 0) {
            char id[ID_LEN] = {0}, key[KEY_LEN] = {0};
            if (sscanf(line, "GET %63s %63s", id, key) != 2) {
                snprintf(resp, sizeof(resp), "ERROR Usage: GET <id> <key>");
            } else {
                pthread_mutex_lock(&store_mtx);
                int idx = find_object_index(id);
                if (idx < 0) {
                    pthread_mutex_unlock(&store_mtx);
                    snprintf(resp, sizeof(resp), "ERROR NotFound");
                } else {
                    char val[VAL_LEN];
                    if (get_prop_idx(idx, key, val, sizeof(val)) == 0) {
                        pthread_mutex_unlock(&store_mtx);
                        snprintf(resp, sizeof(resp), "VALUE %s", val);
                    } else {
                        pthread_mutex_unlock(&store_mtx);
                        snprintf(resp, sizeof(resp), "ERROR NotFound");
                    }
                }
            }
        } else if (strcmp(cmd, "DELETE") == 0) {
            char id[ID_LEN] = {0};
            if (sscanf(line, "DELETE %63s", id) != 1) {
                snprintf(resp, sizeof(resp), "ERROR Usage: DELETE <id>");
            } else {
                pthread_mutex_lock(&store_mtx);
                delete_object(id);
                pthread_mutex_unlock(&store_mtx);
                snprintf(resp, sizeof(resp), "OK");
            }
        } else if (strcmp(cmd, "KEYS") == 0) {
            char id[ID_LEN] = {0};
            if (sscanf(line, "KEYS %63s", id) != 1) {
                snprintf(resp, sizeof(resp), "ERROR Usage: KEYS <id>");
            } else {
                pthread_mutex_lock(&store_mtx);
                int idx = find_object_index(id);
                if (idx < 0) {
                    pthread_mutex_unlock(&store_mtx);
                    snprintf(resp, sizeof(resp), "ERROR NotFound");
                } else {
                    char ks[LINE_LEN];
                    keys_object(idx, ks, sizeof(ks));
                    pthread_mutex_unlock(&store_mtx);
                    snprintf(resp, sizeof(resp), "KEYS %s", ks);
                }
            }
        } else if (strcmp(cmd, "LIST") == 0) {
            pthread_mutex_lock(&store_mtx);
            char ls[LINE_LEN];
            list_objects(ls, sizeof(ls));
            pthread_mutex_unlock(&store_mtx);
            snprintf(resp, sizeof(resp), "LIST %s", ls);
        } else if (strcmp(cmd, "EXIT") == 0) {
            snprintf(resp, sizeof(resp), "OK BYE");
            write_line(cfd, resp);
            break;
        } else if (strcmp(cmd, "SHUTDOWN") == 0) {
            server_running = 0;
            if (listen_fd_global >= 0) close(listen_fd_global);
            snprintf(resp, sizeof(resp), "OK SHUTDOWN");
            write_line(cfd, resp);
            break;
        } else {
            snprintf(resp, sizeof(resp), "ERROR UnknownCommand");
        }
    writeout:
        if (resp[0] != '\0') {
            if (write_line(cfd, resp) < 0) break;
        }
    }
    close(cfd);
    return NULL;
}

static void* server_thread(void* arg) {
    int port = *(int*)arg;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return NULL;
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(port);
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(fd);
        return NULL;
    }
    if (listen(fd, 16) < 0) {
        close(fd);
        return NULL;
    }
    listen_fd_global = fd;
    while (server_running) {
        int* cfd = (int*)malloc(sizeof(int));
        if (!cfd) break;
        *cfd = accept(fd, NULL, NULL);
        if (*cfd < 0) {
            free(cfd);
            if (!server_running) break;
            continue;
        }
        pthread_t t;
        pthread_create(&t, NULL, client_handler, cfd);
        pthread_detach(t);
    }
    if (listen_fd_global >= 0) close(listen_fd_global);
    listen_fd_global = -1;
    return NULL;
}

// Client helpers (functions take parameters and return output as malloc'd string)
static char* send_command(const char* cmd) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        return strdup("ERROR socket");
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(fd);
        return strdup("ERROR connect");
    }
    char sendbuf[LINE_LEN];
    snprintf(sendbuf, sizeof(sendbuf), "%s\n", cmd);
    if (send(fd, sendbuf, strlen(sendbuf), 0) < 0) {
        close(fd);
        return strdup("ERROR send");
    }
    char c, resp[LINE_LEN];
    size_t pos = 0;
    while (pos + 1 < sizeof(resp)) {
        ssize_t n = recv(fd, &c, 1, 0);
        if (n <= 0) break;
        if (c == '\n') break;
        resp[pos++] = c;
    }
    resp[pos] = '\0';
    close(fd);
    return strdup(resp);
}

static char* create_obj(const char* id) {
    char buf[LINE_LEN];
    snprintf(buf, sizeof(buf), "CREATE %s", id);
    return send_command(buf);
}
static char* set_prop(const char* id, const char* key, const char* value) {
    char buf[LINE_LEN];
    snprintf(buf, sizeof(buf), "SET %s %s %s", id, key, value);
    return send_command(buf);
}
static char* get_prop(const char* id, const char* key) {
    char buf[LINE_LEN];
    snprintf(buf, sizeof(buf), "GET %s %s", id, key);
    return send_command(buf);
}
static char* delete_obj(const char* id) {
    char buf[LINE_LEN];
    snprintf(buf, sizeof(buf), "DELETE %s", id);
    return send_command(buf);
}
static char* keys_obj(const char* id) {
    char buf[LINE_LEN];
    snprintf(buf, sizeof(buf), "KEYS %s", id);
    return send_command(buf);
}
static char* list_objs(void) {
    return send_command("LIST");
}
static char* shutdown_server(void) {
    return send_command("SHUTDOWN");
}

int main(void) {
    pthread_t th;
    int port = PORT;
    pthread_create(&th, NULL, server_thread, &port);
    usleep(200 * 1000);

    // Test case 1
    char* r1 = create_obj("foo"); printf("%s\n", r1); free(r1);
    char* r2 = set_prop("foo", "a", "1"); printf("%s\n", r2); free(r2);
    char* r3 = get_prop("foo", "a"); printf("%s\n", r3); free(r3);

    // Test case 2
    char* r4 = create_obj("bar"); printf("%s\n", r4); free(r4);
    char* r5 = set_prop("bar", "x", "hello world"); printf("%s\n", r5); free(r5);
    char* r6 = keys_obj("bar"); printf("%s\n", r6); free(r6);

    // Test case 3
    char* r7 = list_objs(); printf("%s\n", r7); free(r7);

    // Test case 4
    char* r8 = delete_obj("foo"); printf("%s\n", r8); free(r8);
    char* r9 = list_objs(); printf("%s\n", r9); free(r9);

    // Test case 5
    char* r10 = get_prop("foo", "a"); printf("%s\n", r10); free(r10);
    char* r11 = get_prop("bar", "missing"); printf("%s\n", r11); free(r11);

    char* r12 = shutdown_server(); printf("%s\n", r12); free(r12);
    pthread_join(th, NULL);
    return 0;
}