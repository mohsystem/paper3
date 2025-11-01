#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct Session {
    const char* username;
    const char* email;
    const char* ssn;
};

static char* safe_strdup(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, s, len);
    out[len] = '\0';
    return out;
}

static char* html_escape(const char* s) {
    if (s == NULL) {
        return safe_strdup("N/A");
    }
    // Worst-case: every char becomes 6 chars (e.g., &quot;)
    size_t len = strlen(s);
    size_t cap = len * 6 + 1;
    char* out = (char*)malloc(cap);
    if (!out) return NULL;
    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        char c = s[i];
        const char* rep = NULL;
        switch (c) {
            case '&': rep = "&amp;"; break;
            case '<': rep = "&lt;"; break;
            case '>': rep = "&gt;"; break;
            case '"': rep = "&quot;"; break;
            case '\'': rep = "&#x27;"; break;
            case '/': rep = "&#x2F;"; break;
            default: rep = NULL; break;
        }
        if (rep) {
            size_t rlen = strlen(rep);
            if (j + rlen >= cap) { // resize (shouldn't happen due to cap, but be safe)
                cap = cap + rlen + 64;
                char* tmp = (char*)realloc(out, cap);
                if (!tmp) { free(out); return NULL; }
                out = tmp;
            }
            memcpy(out + j, rep, rlen);
            j += rlen;
        } else {
            if (j + 1 >= cap) {
                cap = cap + 64;
                char* tmp = (char*)realloc(out, cap);
                if (!tmp) { free(out); return NULL; }
                out = tmp;
            }
            out[j++] = c;
        }
    }
    out[j] = '\0';
    return out;
}

static char* last4_of_ssn(const char* ssn) {
    if (ssn == NULL) {
        return safe_strdup("N/A");
    }
    // Collect digits up to a cap
    char digits[65];
    size_t dlen = 0;
    for (size_t i = 0; ssn[i] != '\0' && dlen < sizeof(digits) - 1; i++) {
        if (isdigit((unsigned char)ssn[i])) {
            digits[dlen++] = ssn[i];
        }
    }
    digits[dlen] = '\0';
    if (dlen < 4) {
        return safe_strdup("N/A");
    }
    char* out = (char*)malloc(5);
    if (!out) return NULL;
    memcpy(out, digits + (dlen - 4), 4);
    out[4] = '\0';
    return out;
}

static char* http_response(int code, const char* content_type, const char* body) {
    const char* reason = "OK";
    switch (code) {
        case 200: reason = "OK"; break;
        case 400: reason = "Bad Request"; break;
        case 404: reason = "Not Found"; break;
        case 405: reason = "Method Not Allowed"; break;
        default: reason = "OK"; break;
    }
    size_t body_len = body ? strlen(body) : 0;
    char header[256];
    int n = snprintf(header, sizeof(header),
                     "HTTP/1.1 %d %s\r\nContent-Type: %s\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n",
                     code, reason, content_type, body_len);
    if (n < 0) n = 0;
    size_t header_len = (size_t)n;
    size_t total = header_len + body_len;
    char* resp = (char*)malloc(total + 1);
    if (!resp) return NULL;
    memcpy(resp, header, header_len);
    if (body_len > 0) memcpy(resp + header_len, body, body_len);
    resp[total] = '\0';
    return resp;
}

static char* info(const struct Session* session) {
    if (session == NULL) {
        return http_response(400, "text/plain; charset=UTF-8", "Bad Request");
    }
    char* esc_user = html_escape(session->username);
    char* esc_email = html_escape(session->email);
    char* last4 = last4_of_ssn(session->ssn);
    char* esc_last4 = html_escape(last4);

    const char* prefix = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>Info</title></head><body>"
                         "<h1>User Info</h1><ul>";
    const char* suffix = "</ul></body></html>";

    // Build body safely
    const char* li_user_prefix = "<li>Username: ";
    const char* li_email_prefix = "<li>Email: ";
    const char* li_ssn_prefix = "<li>SSN Last 4: ";
    const char* li_end = "</li>";

    size_t body_len = strlen(prefix) + strlen(suffix)
                    + strlen(li_user_prefix) + strlen(esc_user) + strlen(li_end)
                    + strlen(li_email_prefix) + strlen(esc_email) + strlen(li_end)
                    + strlen(li_ssn_prefix) + strlen(esc_last4) + strlen(li_end);

    char* body = (char*)malloc(body_len + 1);
    if (!body) {
        free(esc_user);
        free(esc_email);
        free(last4);
        free(esc_last4);
        return http_response(500, "text/plain; charset=UTF-8", "Internal Server Error");
    }

    int m = snprintf(body, body_len + 1, "%s%s%s%s%s%s%s%s%s%s",
                     prefix,
                     li_user_prefix, esc_user, li_end,
                     li_email_prefix, esc_email, li_end,
                     li_ssn_prefix, esc_last4, li_end);
    // Append suffix
    if (m < 0) {
        free(body);
        free(esc_user);
        free(esc_email);
        free(last4);
        free(esc_last4);
        return http_response(500, "text/plain; charset=UTF-8", "Internal Server Error");
    }
    // Ensure room and append suffix
    size_t used = (size_t)m;
    size_t suf_len = strlen(suffix);
    char* tmp = (char*)realloc(body, used + suf_len + 1);
    if (!tmp) {
        free(body);
        free(esc_user);
        free(esc_email);
        free(last4);
        free(esc_last4);
        return http_response(500, "text/plain; charset=UTF-8", "Internal Server Error");
    }
    body = tmp;
    memcpy(body + used, suffix, suf_len);
    body[used + suf_len] = '\0';

    free(esc_user);
    free(esc_email);
    free(last4);
    free(esc_last4);

    char* resp = http_response(200, "text/html; charset=UTF-8", body);
    free(body);
    return resp;
}

static char* handle_request(const char* method, const char* path, const struct Session* session) {
    if (!method || !path) {
        return http_response(400, "text/plain; charset=UTF-8", "Bad Request");
    }
    if (strcmp(method, "GET") != 0) {
        return http_response(405, "text/plain; charset=UTF-8", "Method Not Allowed");
    }
    if (strcmp(path, "/info") == 0) {
        return info(session);
    }
    return http_response(404, "text/plain; charset=UTF-8", "Not Found");
}

int main(void) {
    // Test 1: Valid session
    struct Session s1 = { "alice", "alice@example.com", "123-45-6789" };
    char* r1 = handle_request("GET", "/info", &s1);
    if (r1) { puts(r1); free(r1); }

    // Test 2: Long spaced SSN (ensure last 4)
    struct Session s2 = { "bob", "bob@example.com", "0000 1111 2222 3333 4444 5555 6666 7777 8888 9999" };
    char* r2 = handle_request("GET", "/info", &s2);
    if (r2) { puts(r2); free(r2); }

    // Test 3: Malicious username (escaped)
    struct Session s3 = { "<script>alert(1)</script>", "evil@example.com", "111223333" };
    char* r3 = handle_request("GET", "/info", &s3);
    if (r3) { puts(r3); free(r3); }

    // Test 4: Missing SSN -> N/A
    struct Session s4 = { "charlie", "charlie@example.com", NULL };
    char* r4 = handle_request("GET", "/info", &s4);
    if (r4) { puts(r4); free(r4); }

    // Test 5: Wrong path and wrong method
    struct Session s5 = { "dana", "dana@example.com", "444-55-1234" };
    char* r5a = handle_request("GET", "/unknown", &s5);
    if (r5a) { puts(r5a); free(r5a); }
    char* r5b = handle_request("POST", "/info", &s5);
    if (r5b) { puts(r5b); free(r5b); }

    return 0;
}