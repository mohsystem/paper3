#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

typedef struct {
    char username[64];
    char email[128];
    char ssn[32];
} Session;

static char* escape_html(const char* s) {
    size_t len = strlen(s);
    // Worst case every char becomes 6 chars; allocate generously
    size_t cap = len * 6 + 1;
    char* out = (char*)malloc(cap);
    if (!out) return NULL;
    size_t j = 0;
    for (size_t i = 0; i < len; ++i) {
        char c = s[i];
        const char* rep = NULL;
        switch (c) {
            case '&': rep="&amp;"; break;
            case '<': rep="&lt;"; break;
            case '>': rep="&gt;"; break;
            case '"': rep="&quot;"; break;
            case '\'': rep="&#39;"; break;
            default: break;
        }
        if (rep) {
            size_t rl = strlen(rep);
            memcpy(out + j, rep, rl);
            j += rl;
        } else {
            out[j++] = c;
        }
    }
    out[j] = '\0';
    return out;
}

static char* last4_of_ssn(const char* ssn) {
    // Extract digits
    char digits[128];
    size_t d = 0;
    for (size_t i = 0; ssn[i] != '\0' && d < sizeof(digits)-1; ++i) {
        if (isdigit((unsigned char)ssn[i])) {
            digits[d++] = ssn[i];
        }
    }
    digits[d] = '\0';
    if (d == 0) {
        char* res = (char*)malloc(4);
        strcpy(res, "N/A");
        return res;
    }
    size_t start = d > 4 ? d - 4 : 0;
    size_t len = d - start;
    char* res = (char*)malloc(len + 1);
    memcpy(res, digits + start, len);
    res[len] = '\0';
    return res;
}

// info() function that accepts a session and returns an allocated HTML string
char* info(const Session* session) {
    const char* username = session->username[0] ? session->username : "guest";
    const char* email = session->email[0] ? session->email : "guest@example.com";
    const char* ssn = session->ssn;

    char* last4 = last4_of_ssn(ssn);
    char* u_esc = escape_html(username);
    char* e_esc = escape_html(email);
    char* l4_esc = escape_html(last4);

    const char* tmpl_prefix = "<!DOCTYPE html><html><head><title>Info</title></head><body><h1>User Info</h1>";
    const char* tmpl_mid1 = "<p>Username: ";
    const char* tmpl_mid2 = "</p><p>Email: ";
    const char* tmpl_mid3 = "</p><p>SSN (last 4): ";
    const char* tmpl_suffix = "</p></body></html>";

    size_t total = strlen(tmpl_prefix) + strlen(tmpl_mid1) + strlen(u_esc) +
                   strlen(tmpl_mid2) + strlen(e_esc) +
                   strlen(tmpl_mid3) + strlen(l4_esc) +
                   strlen(tmpl_suffix) + 1;

    char* html = (char*)malloc(total);
    if (!html) {
        free(last4); free(u_esc); free(e_esc); free(l4_esc);
        return NULL;
    }
    html[0] = '\0';
    strcat(html, tmpl_prefix);
    strcat(html, tmpl_mid1); strcat(html, u_esc);
    strcat(html, tmpl_mid2); strcat(html, e_esc);
    strcat(html, tmpl_mid3); strcat(html, l4_esc);
    strcat(html, tmpl_suffix);

    free(last4); free(u_esc); free(e_esc); free(l4_esc);
    return html;
}

// Minimal single-threaded HTTP server for /info using a default in-memory session
static void run_server(unsigned short port) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) return;
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(server_fd);
        return;
    }
    if (listen(server_fd, 16) < 0) {
        close(server_fd);
        return;
    }

    // Single session placeholder for demo
    Session session;
    memset(&session, 0, sizeof(session));
    strcpy(session.username, "guest");
    strcpy(session.email, "guest@example.com");
    strcpy(session.ssn, "0000");

    for (;;) {
        int client = accept(server_fd, NULL, NULL);
        if (client < 0) continue;
        char req[8192];
        ssize_t n = recv(client, req, sizeof(req)-1, 0);
        if (n <= 0) { close(client); continue; }
        req[n] = '\0';

        // Parse request line
        char method[16], path[256];
        method[0] = path[0] = '\0';
        sscanf(req, "%15s %255s", method, path);

        if (strcmp(method, "GET") != 0) {
            const char* body = "Method Not Allowed";
            char header[256];
            snprintf(header, sizeof(header),
                     "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: %zu\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\n",
                     strlen(body));
            send(client, header, strlen(header), 0);
            send(client, body, strlen(body), 0);
            close(client);
            continue;
        }
        if (strcmp(path, "/info") != 0) {
            const char* body = "Not Found";
            char header[256];
            snprintf(header, sizeof(header),
                     "HTTP/1.1 404 Not Found\r\nContent-Length: %zu\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\n",
                     strlen(body));
            send(client, header, strlen(header), 0);
            send(client, body, strlen(body), 0);
            close(client);
            continue;
        }

        char* body = info(&session);
        if (!body) { close(client); continue; }
        char header[512];
        snprintf(header, sizeof(header),
                 "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n",
                 strlen(body));
        send(client, header, strlen(header), 0);
        send(client, body, strlen(body), 0);
        free(body);
        close(client);
    }
    // close(server_fd); // unreachable
}

int main(void) {
    // 5 test cases calling info() directly
    Session tests[5];
    memset(tests, 0, sizeof(tests));

    strcpy(tests[0].username, "alice");
    strcpy(tests[0].email, "alice@example.com");
    strcpy(tests[0].ssn, "123-45-6789");

    strcpy(tests[1].username, "bob");
    strcpy(tests[1].email, "bob@x.com");
    strcpy(tests[1].ssn, "0000");

    strcpy(tests[2].username, "carol");
    strcpy(tests[2].email, "carol@x.com");
    strcpy(tests[2].ssn, "987654321");

    strcpy(tests[3].username, "dave");
    strcpy(tests[3].email, "dave@x.com");
    strcpy(tests[3].ssn, "");

    strcpy(tests[4].username, "eve");
    strcpy(tests[4].email, "eve@x.com");
    strcpy(tests[4].ssn, "12-34");

    for (int i = 0; i < 5; ++i) {
        char* out = info(&tests[i]);
        printf("TEST %d OUTPUT:\n%s\n-----\n", i + 1, out);
        free(out);
    }

    // To run the web server (optional), uncomment:
    // run_server(8083);

    return 0;
}