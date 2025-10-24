#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>

#define DB_PATH_C "users_db_c.jsonl"

static void trim(const char* in, char* out, size_t out_sz) {
    if (out_sz == 0) return;
    size_t len = in ? strlen(in) : 0;
    size_t start = 0, end = 0;
    if (len == 0) { out[0] = '\0'; return; }
    while (start < len && (in[start] == ' ' || in[start] == '\t' || in[start] == '\n' || in[start] == '\r')) start++;
    if (start == len) { out[0] = '\0'; return; }
    end = len - 1;
    while (end > start && (in[end] == ' ' || in[end] == '\t' || in[end] == '\n' || in[end] == '\r')) end--;
    size_t copy_len = end - start + 1;
    if (copy_len >= out_sz) copy_len = out_sz - 1;
    memcpy(out, in + start, copy_len);
    out[copy_len] = '\0';
}

static int is_valid_name(const char* name_in) {
    if (!name_in) return 0;
    char name[256];
    trim(name_in, name, sizeof(name));
    size_t n = strlen(name);
    if (n < 2 || n > 100) return 0;
    if (!isalpha((unsigned char)name[0])) return 0;
    if (!isalpha((unsigned char)name[n-1])) return 0;
    for (size_t i = 0; i < n; i++) {
        char c = name[i];
        if (isalpha((unsigned char)c) || c==' ' || c=='-' || c=='\'') continue;
        return 0;
    }
    return 1;
}

static int is_valid_email(const char* email_in) {
    if (!email_in) return 0;
    char email[512];
    trim(email_in, email, sizeof(email));
    size_t n = strlen(email);
    if (n < 3 || n > 254) return 0;

    // to lower
    for (size_t i = 0; i < n; i++) {
        email[i] = (char)tolower((unsigned char)email[i]);
    }

    char* at = strchr(email, '@');
    if (!at) return 0;
    if (strchr(at + 1, '@')) return 0;

    size_t local_len = (size_t)(at - email);
    char* domain = at + 1;
    size_t domain_len = strlen(domain);

    if (local_len == 0 || domain_len < 3) return 0;
    if (domain[0] == '.' || domain[domain_len - 1] == '.') return 0;
    if (!strchr(domain, '.')) return 0;

    // local checks
    if (email[0]=='.' || email[local_len-1]=='.') return 0;
    for (size_t i = 0; i < local_len; i++) {
        char c = email[i];
        if (isalnum((unsigned char)c) || c=='.' || c=='_' || c=='%' || c=='+' || c=='-') continue;
        return 0;
    }
    for (size_t i = 0; i + 1 < local_len; i++) {
        if (email[i]=='.' && email[i+1]=='.') return 0;
    }

    // domain checks
    for (size_t i = 0; i < domain_len; i++) {
        char c = domain[i];
        if (isalnum((unsigned char)c) || c=='.' || c=='-') continue;
        return 0;
    }
    for (size_t i = 0; i + 1 < domain_len; i++) {
        if (domain[i]=='.' && domain[i+1]=='.') return 0;
    }
    char* last_dot = strrchr(domain, '.');
    if (!last_dot) return 0;
    if (strlen(last_dot + 1) < 2) return 0;

    return 1;
}

static int is_valid_age(int age) {
    return age >= 0 && age <= 150;
}

static void escape_json(const char* in, char* out, size_t out_sz) {
    size_t j = 0;
    for (size_t i = 0; in[i] != '\0'; i++) {
        unsigned char c = (unsigned char)in[i];
        const char* esc = NULL;
        char buf[7];
        if (c == '\\') esc = "\\\\";
        else if (c == '\"') esc = "\\\"";
        else if (c == '\b') esc = "\\b";
        else if (c == '\f') esc = "\\f";
        else if (c == '\n') esc = "\\n";
        else if (c == '\r') esc = "\\r";
        else if (c == '\t') esc = "\\t";
        else if (c < 0x20) {
            snprintf(buf, sizeof(buf), "\\u%04x", c);
            esc = buf;
        }

        if (esc) {
            size_t elen = strlen(esc);
            for (size_t k = 0; k < elen && j + 1 < out_sz; k++) {
                out[j++] = esc[k];
            }
        } else {
            if (j + 1 < out_sz) {
                out[j++] = (char)c;
            }
        }
    }
    if (j < out_sz) out[j] = '\0';
    else out[out_sz - 1] = '\0';
}

static void generate_id(char* out, size_t out_sz) {
    // 16 hex chars
    if (out_sz < 17) { if (out_sz > 0) out[0] = '\0'; return; }
    for (int i = 0; i < 8; i++) {
        unsigned int r = (unsigned int)(rand() & 0xFF);
        snprintf(out + i*2, out_sz - i*2, "%02x", r);
    }
    out[16] = '\0';
}

static int append_line_to_db(const char* line) {
    FILE* f = fopen(DB_PATH_C, "ab");
    if (!f) return 0;
    size_t len = strlen(line);
    size_t w = fwrite(line, 1, len, f);
    fflush(f);
    int ok = (w == len);
    fclose(f);
    return ok;
}

static int create_user(const char* name_in, const char* email_in, int age, char* out_message, size_t out_message_sz) {
    char n[256], e[512];
    trim(name_in, n, sizeof(n));
    trim(email_in, e, sizeof(e));

    if (!is_valid_name(n)) {
        snprintf(out_message, out_message_sz, "ERROR: invalid name");
        return 0;
    }
    if (!is_valid_email(e)) {
        snprintf(out_message, out_message_sz, "ERROR: invalid email");
        return 0;
    }
    if (!is_valid_age(age)) {
        snprintf(out_message, out_message_sz, "ERROR: invalid age");
        return 0;
    }

    char n_esc[512], e_esc[1024];
    escape_json(n, n_esc, sizeof(n_esc));
    escape_json(e, e_esc, sizeof(e_esc));

    time_t t = time(NULL);
    struct tm g;
#if defined(_WIN32)
    g = *gmtime(&t);
#else
    gmtime_r(&t, &g);
#endif
    char ts[32];
    strftime(ts, sizeof(ts), "%Y-%m-%dT%H:%M:%SZ", &g);

    char id[32];
    generate_id(id, sizeof(id));

    char line[2048];
    snprintf(line, sizeof(line), "{\"id\":\"%s\",\"name\":\"%s\",\"email\":\"%s\",\"age\":%d,\"ts\":\"%s\"}\n",
             id, n_esc, e_esc, age, ts);

    if (!append_line_to_db(line)) {
        snprintf(out_message, out_message_sz, "ERROR: storage failure");
        return 0;
    }

    snprintf(out_message, out_message_sz, "OK: %s", id);
    return 1;
}

int main(void) {
    srand((unsigned int)time(NULL));
    char msg[256];

    create_user("John Doe", "john.doe@example.com", 30, msg, sizeof(msg));
    printf("%s\n", msg);

    create_user("A", "not-an-email", 20, msg, sizeof(msg));
    printf("%s\n", msg);

    create_user("Mary-Jane O'Neil", "mary@example", -1, msg, sizeof(msg));
    printf("%s\n", msg);

    create_user("  Robert   ", "robert.smith@example.co.uk", 44, msg, sizeof(msg));
    printf("%s\n", msg);

    create_user("ThisNameIsWayTooLong_ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ", "long.name@example.com", 20, msg, sizeof(msg));
    printf("%s\n", msg);

    return 0;
}