// 1) Problem understanding:
// Minimal HTTP server exposing /info, returning HTML with username, email, and last 4 digits of SSN.
// Core logic in render_info(username, email, ssn) returning dynamically allocated HTML string.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

// 2) Security requirements:
// - Escape HTML.
// - Only last 4 of SSN.
// - Basic email validation.
// - Security headers; do not process sensitive data from URL.

static char* escape_html(const char* in) {
    if (!in) {
        char* z = (char*)malloc(1);
        if (z) z[0] = '\0';
        return z;
    }
    size_t len = 0;
    for (const char* p = in; *p; ++p) {
        switch (*p) {
            case '&': len += 5; break; // &amp;
            case '<': len += 4; break; // &lt;
            case '>': len += 4; break; // &gt;
            case '"': len += 6; break; // &quot;
            case '\'': len += 6; break; // &#x27;
            case '/': len += 6; break; // &#x2F;
            default: len += 1; break;
        }
    }
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    char* o = out;
    for (const char* p = in; *p; ++p) {
        switch (*p) {
            case '&': memcpy(o, "&amp;", 5); o += 5; break;
            case '<': memcpy(o, "&lt;", 4); o += 4; break;
            case '>': memcpy(o, "&gt;", 4); o += 4; break;
            case '"': memcpy(o, "&quot;", 6); o += 6; break;
            case '\'': memcpy(o, "&#x27;", 6); o += 6; break;
            case '/': memcpy(o, "&#x2F;", 6); o += 6; break;
            default: *o++ = *p; break;
        }
    }
    *o = '\0';
    return out;
}

static char* extract_last4(const char* ssn) {
    if (!ssn) {
        char* z = (char*)malloc(4);
        strcpy(z, "N/A");
        return z;
    }
    char digits[64];
    size_t d = 0;
    for (const char* p = ssn; *p && d < sizeof(digits)-1; ++p) {
        if (isdigit((unsigned char)*p)) digits[d++] = *p;
    }
    digits[d] = '\0';
    if (d < 4) {
        char* z = (char*)malloc(4);
        strcpy(z, "N/A");
        return z;
    }
    char* out = (char*)malloc(5);
    memcpy(out, digits + d - 4, 4);
    out[4] = '\0';
    return out;
}

static int is_likely_valid_email(const char* email) {
    if (!email) return 0;
    size_t n = strlen(email);
    if (n == 0 || n > 254) return 0;
    // Very simple validation: must contain one '@' and at least one '.' after it
    const char* at = strchr(email, '@');
    if (!at || at == email) return 0;
    const char* dot = strrchr(email, '.');
    if (!dot || dot < at + 2) return 0;
    return 1;
}

// 3) Secure coding generation: function taking inputs, returns HTML (caller must free)
char* render_info(const char* username, const char* email, const char* ssn) {
    char* safe_user = escape_html(username ? username : "");
    char* safe_email = escape_html(email ? email : "");
    char* last4 = escape_html(extract_last4(ssn));
    const char* note = is_likely_valid_email(email) ? "" : " (unverified format)";
    // CSP
    const char* csp = "default-src 'none'; style-src 'unsafe-inline'; img-src 'none'; frame-ancestors 'none'; base-uri 'none'; form-action 'none'";

    // Estimate size and build
    const char* head = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\">"
                       "<meta http-equiv=\"Content-Security-Policy\" content=\"";
    const char* mid1 = "\"><meta http-equiv=\"X-Content-Type-Options\" content=\"nosniff\"><title>User Info</title>"
                       "<style>body{font-family:sans-serif;margin:2rem} .box{border:1px solid #ccc;padding:1rem;border-radius:8px;max-width:500px}</style>"
                       "</head><body><div class=\"box\"><h1>User Info</h1><p><strong>Username:</strong> ";
    const char* mid2 = "</p><p><strong>Email:</strong> ";
    const char* mid3 = "</p><p><strong>SSN (last 4):</strong> ";
    const char* tail = "</p></div></body></html>";

    size_t total = strlen(head) + strlen(csp) + strlen(mid1) + strlen(safe_user)
                 + strlen(mid2) + strlen(safe_email) + strlen(note)
                 + strlen(mid3) + strlen(last4) + strlen(tail) + 1;

    char* html = (char*)malloc(total);
    if (!html) { free(safe_user); free(safe_email); free(last4); return NULL; }

    snprintf(html, total, "%s%s%s%s%s%s%s%s%s", head, csp, mid1, safe_user, mid2, safe_email, note, mid3, last4);
    // Append tail
    size_t cur = strlen(html);
    snprintf(html + cur, total - cur, "%s", tail);

    free(safe_user);
    free(safe_email);
    free(last4);
    return html;
}

static void run_server(int port) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return;
    }
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons((unsigned short)port);
    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        return;
    }
    if (listen(server_fd, 10) < 0) {
        perror("listen");
        close(server_fd);
        return;
    }
    printf("C server started on http://localhost:%d/info\n", port);
    while (1) {
        int client = accept(server_fd, NULL, NULL);
        if (client < 0) continue;
        char buffer[2048];
        ssize_t n = read(client, buffer, sizeof(buffer) - 1);
        if (n <= 0) { close(client); continue; }
        buffer[n] = '\0';

        int ok = 0;
        char* line_end = strstr(buffer, "\r\n");
        if (line_end) {
            *line_end = '\0';
            if (strncmp(buffer, "GET ", 4) == 0) {
                char* path_start = buffer + 4;
                char* sp = strchr(path_start, ' ');
                if (sp) *sp = '\0';
                if (strcmp(path_start, "/info") == 0) ok = 1;
            }
        }

        if (!ok) {
            const char* notFound = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: 9\r\n\r\nNot Found";
            write(client, notFound, strlen(notFound));
            close(client);
            continue;
        }

        char* body = render_info("alice", "alice@example.com", "123-45-6789");
        if (!body) {
            const char* err = "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n";
            write(client, err, strlen(err));
            close(client);
            continue;
        }

        char header[512];
        int hdr_len = snprintf(header, sizeof(header),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html; charset=utf-8\r\n"
            "Content-Security-Policy: default-src 'none'; style-src 'unsafe-inline'; img-src 'none'; frame-ancestors 'none'; base-uri 'none'; form-action 'none'\r\n"
            "X-Content-Type-Options: nosniff\r\n"
            "Content-Length: %zu\r\n"
            "Connection: close\r\n\r\n", strlen(body));
        write(client, header, (size_t)hdr_len);
        write(client, body, strlen(body));
        free(body);
        close(client);
    }
}

// 4) Code review: ensured escaping; only last 4 digits; safe minimal HTTP.

int main(void) {
    // 5 test cases
    const char* tests[5][3] = {
        {"alice", "alice@example.com", "123-45-6789"},
        {"bob", "bob.smith@corp.co.uk", "000123456"},
        {"<script>alert(1)</script>", "user@ex<ample>.com", "12-34"},
        {"charlie", "invalid-email@", "xxx-yy-7788"},
        {"\xCE\x94\xCE\xB7\xCE\xBC\xCE\xAE\xCF\x84\xCF\x81\xCE\xB7\xCF\x82", "dimitris@example.gr", "9999"}
    };
    for (int i = 0; i < 5; i++) {
        char* html = render_info(tests[i][0], tests[i][1], tests[i][2]);
        printf("=== C Test Case %d ===\n", i + 1);
        if (html) {
            printf("%s\n\n", html);
            free(html);
        } else {
            printf("Error generating HTML\n\n");
        }
    }
    run_server(8083);
    return 0;
}