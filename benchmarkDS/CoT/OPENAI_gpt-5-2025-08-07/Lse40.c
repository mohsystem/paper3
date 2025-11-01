/* Chain-of-Through process in code generation:
 * 1) Problem understanding:
 *    - A simple web-like application with a single route GET /info.
 *    - info() reads a "session" struct for username, email, ssn; returns HTML with username, email,
 *      and last 4 digits of SSN.
 * 2) Security requirements:
 *    - Escape HTML for username and email to avoid XSS.
 *    - Only show last 4 digits (digits-only) of SSN; mask if fewer than 4 digits.
 * 3) Secure coding generation:
 *    - Implement robust escaping and SSN processing.
 * 4) Code review:
 *    - No exposure of full SSN; careful memory allocation and bounds.
 * 5) Secure code output:
 *    - Provide info(session) and handle(method, path, session) with 5 test cases.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    char username[256];
    char email[256];
    char ssn[64];
    int has_username;
    int has_email;
    int has_ssn;
} Session;

// Escape HTML, returns newly allocated string that must be freed by caller
char* escape_html_dup(const char* in) {
    if (!in) {
        char* out = (char*)malloc(1);
        if (out) out[0] = '\0';
        return out;
    }
    size_t len = strlen(in);
    // Worst-case: every char becomes 6 chars (e.g., &quot;)
    size_t cap = len * 6 + 1;
    char* out = (char*)malloc(cap);
    if (!out) return NULL;
    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        char c = in[i];
        const char* rep = NULL;
        switch (c) {
            case '&': rep = "&amp;"; break;
            case '<': rep = "&lt;"; break;
            case '>': rep = "&gt;"; break;
            case '"': rep = "&quot;"; break;
            case '\'': rep = "&#x27;"; break;
            default: rep = NULL; break;
        }
        if (rep) {
            size_t rlen = strlen(rep);
            if (j + rlen >= cap) { // grow (shouldn't happen due to over-alloc), but safe guard
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

// Compute last 4 digits; returns malloc'ed 5-byte string (4 chars + NUL)
char* last4_safe_dup(const char* ssn) {
    if (!ssn) ssn = "";
    char digits_buf[128];
    size_t k = 0;
    for (size_t i = 0; ssn[i] != '\0' && k < sizeof(digits_buf) - 1; i++) {
        if (isdigit((unsigned char)ssn[i])) {
            digits_buf[k++] = ssn[i];
        }
    }
    digits_buf[k] = '\0';
    char* out = (char*)malloc(5);
    if (!out) return NULL;
    if (k >= 4) {
        out[0] = digits_buf[k - 4];
        out[1] = digits_buf[k - 3];
        out[2] = digits_buf[k - 2];
        out[3] = digits_buf[k - 1];
    } else {
        int pad = 4 - (int)k;
        for (int i = 0; i < pad; i++) out[i] = '*';
        for (size_t i = 0; i < k; i++) out[pad + i] = digits_buf[i];
        if (k == 0) {
            // all masked
        }
    }
    out[4] = '\0';
    return out;
}

// Ensure defaults present in session
static void ensure_defaults(Session* s) {
    if (!s->has_username) {
        strncpy(s->username, "Jane Doe", sizeof(s->username)-1);
        s->username[sizeof(s->username)-1] = '\0';
        s->has_username = 1;
    }
    if (!s->has_email) {
        strncpy(s->email, "jane.doe@example.com", sizeof(s->email)-1);
        s->email[sizeof(s->email)-1] = '\0';
        s->has_email = 1;
    }
    if (!s->has_ssn) {
        strncpy(s->ssn, "123-45-6789", sizeof(s->ssn)-1);
        s->ssn[sizeof(s->ssn)-1] = '\0';
        s->has_ssn = 1;
    }
}

// info(): returns newly allocated HTML string to be freed by caller
char* Lse40_info(Session* session) {
    if (!session) {
        session = (Session*)calloc(1, sizeof(Session));
        if (!session) return NULL;
        // This temporary session will be leaked intentionally if not freed by caller.
        // For testing purposes only; in real code, manage memory properly.
    }
    ensure_defaults(session);

    char* esc_user = escape_html_dup(session->username);
    char* esc_email = escape_html_dup(session->email);
    char* last4 = last4_safe_dup(session->ssn);
    if (!esc_user || !esc_email || !last4) {
        free(esc_user); free(esc_email); free(last4);
        return NULL;
    }

    const char* p1 = "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>User Info</title></head><body><h1>User Information</h1><ul><li>Username: ";
    const char* p2 = "</li><li>Email: ";
    const char* p3 = "</li><li>SSN (last 4): ";
    const char* p4 = "</li></ul></body></html>";

    size_t total = strlen(p1) + strlen(esc_user) + strlen(p2) + strlen(esc_email) + strlen(p3) + strlen(last4) + strlen(p4) + 1;
    char* html = (char*)malloc(total);
    if (!html) {
        free(esc_user); free(esc_email); free(last4);
        return NULL;
    }
    snprintf(html, total, "%s%s%s%s%s%s%s", p1, esc_user, p2, esc_email, p3, last4, p4);

    free(esc_user);
    free(esc_email);
    free(last4);
    return html;
}

// Router: returns newly allocated string to be freed by caller
char* Lse40_handle(const char* method, const char* path, Session* session) {
    if (method && path && (strcmp(method, "GET") == 0 || strcmp(method, "get") == 0 || strcmp(method, "Get") == 0) && strcmp(path, "/info") == 0) {
        return Lse40_info(session);
    }
    // return 404
    const char* notfound = "404 Not Found";
    char* out = (char*)malloc(strlen(notfound) + 1);
    if (!out) return NULL;
    strcpy(out, notfound);
    return out;
}

// Helper to init a session with provided values
void init_session(Session* s, const char* username, const char* email, const char* ssn) {
    memset(s, 0, sizeof(Session));
    if (username) {
        strncpy(s->username, username, sizeof(s->username)-1);
        s->username[sizeof(s->username)-1] = '\0';
        s->has_username = 1;
    }
    if (email) {
        strncpy(s->email, email, sizeof(s->email)-1);
        s->email[sizeof(s->email)-1] = '\0';
        s->has_email = 1;
    }
    if (ssn) {
        strncpy(s->ssn, ssn, sizeof(s->ssn)-1);
        s->ssn[sizeof(s->ssn)-1] = '\0';
        s->has_ssn = 1;
    }
}

int main(void) {
    // Test 1: Empty session -> defaults
    Session s1; memset(&s1, 0, sizeof(s1));
    char* r1 = Lse40_handle("GET", "/info", &s1);
    if (r1) { printf("%s\n-----\n", r1); free(r1); }

    // Test 2: Provided clean data
    Session s2; init_session(&s2, "Alice", "alice@example.com", "987654321");
    char* r2 = Lse40_handle("GET", "/info", &s2);
    if (r2) { printf("%s\n-----\n", r2); free(r2); }

    // Test 3: XSS attempt and short SSN
    Session s3; init_session(&s3, "<script>alert(1)</script>", "bob@example.com", "12");
    char* r3 = Lse40_handle("GET", "/info", &s3);
    if (r3) { printf("%s\n-----\n", r3); free(r3); }

    // Test 4: SSN with non-digits and spaces
    Session s4; init_session(&s4, "Charlie", "charlie+tag@example.co.uk", "  999-88  7777 ");
    char* r4 = Lse40_handle("GET", "/info", &s4);
    if (r4) { printf("%s\n-----\n", r4); free(r4); }

    // Test 5: Missing ssn; present username/email
    Session s5; init_session(&s5, "Dana", "dana@example.org", NULL);
    char* r5 = Lse40_handle("GET", "/info", &s5);
    if (r5) { printf("%s\n", r5); free(r5); }

    return 0;
}