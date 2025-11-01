#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <openssl/evp.h>
#include <openssl/rand.h>

#define DB_MAGIC "USRDB1"
#define PBKDF2_ITERATIONS 210000
#define SALT_LEN 16
#define DK_LEN 32

static int is_valid_username(const char* u) {
    if (!u) return 0;
    size_t len = strlen(u);
    if (len < 3 || len > 32) return 0;
    for (size_t i = 0; i < len; i++) {
        char c = u[i];
        if (!(isalnum((unsigned char)c) || c == '_' || c == '-')) return 0;
    }
    return 1;
}

static int is_strong_password(const char* p) {
    if (!p) return 0;
    size_t len = strlen(p);
    if (len < 12 || len > 128) return 0;
    int hasU=0, hasL=0, hasD=0, hasS=0;
    const char* specials = "!@#$%^&*()-_=+[]{}|;:'\",.<>/?`~";
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)p[i];
        if (isupper(c)) hasU=1;
        else if (islower(c)) hasL=1;
        else if (isdigit(c)) hasD=1;
        else if (strchr(specials, c)) hasS=1;
    }
    return hasU && hasL && hasD && hasS;
}

static void to_hex(const unsigned char* in, size_t len, char* out, size_t outlen) {
    static const char* hex = "0123456789abcdef";
    if (outlen < len*2+1) return;
    for (size_t i = 0; i < len; i++) {
        out[i*2]   = hex[(in[i] >> 4) & 0xF];
        out[i*2+1] = hex[in[i] & 0xF];
    }
    out[len*2] = '\0';
}

static int read_db(const char* path, char*** lines_out, size_t* count_out) {
    *lines_out = NULL;
    *count_out = 0;
    FILE* f = fopen(path, "r");
    if (!f) {
        return 1; // no file is OK
    }
    char buf[8192];
    if (!fgets(buf, sizeof(buf), f)) {
        fclose(f);
        return 0;
    }
    size_t len = strcspn(buf, "\r\n");
    buf[len] = '\0';
    if (strcmp(buf, DB_MAGIC) != 0) {
        fclose(f);
        return 0;
    }
    size_t cap = 16;
    size_t n = 0;
    char** lines = (char**)calloc(cap, sizeof(char*));
    if (!lines) { fclose(f); return 0; }
    lines[n] = strdup(DB_MAGIC);
    if (!lines[n]) { fclose(f); free(lines); return 0; }
    n++;

    while (fgets(buf, sizeof(buf), f)) {
        if (strlen(buf) > 4096) { // basic bound
            // skip overly long lines
            fclose(f);
            for (size_t i = 0; i < n; i++) free(lines[i]);
            free(lines);
            return 0;
        }
        size_t l = strcspn(buf, "\r\n");
        buf[l] = '\0';
        if (n >= cap) {
            cap *= 2;
            char** tmp = (char**)realloc(lines, cap * sizeof(char*));
            if (!tmp) {
                fclose(f);
                for (size_t i = 0; i < n; i++) free(lines[i]);
                free(lines);
                return 0;
            }
            lines = tmp;
        }
        lines[n] = strdup(buf);
        if (!lines[n]) {
            fclose(f);
            for (size_t i = 0; i < n; i++) free(lines[i]);
            free(lines);
            return 0;
        }
        n++;
    }
    fclose(f);
    *lines_out = lines;
    *count_out = n;
    return 1;
}

static int write_db_atomically(const char* path, char** lines, size_t count) {
    char dir[1024];
    const char* slash = strrchr(path, '/');
#ifdef _WIN32
    const char* bslash = strrchr(path, '\\');
    if (!slash || (bslash && bslash > slash)) slash = bslash;
#endif
    if (slash) {
        size_t l = (size_t)(slash - path);
        if (l >= sizeof(dir)) return 0;
        memcpy(dir, path, l);
        dir[l] = '\0';
    } else {
        strcpy(dir, ".");
    }

    char tmpl[1024];
    if (snprintf(tmpl, sizeof(tmpl), "%s/%s", dir, "usersdb_XXXXXX") >= (int)sizeof(tmpl)) return 0;
    int fd = mkstemp(tmpl);
    if (fd < 0) return 0;
    fchmod(fd, S_IRUSR | S_IWUSR);
    FILE* f = fdopen(fd, "w");
    if (!f) {
        close(fd);
        unlink(tmpl);
        return 0;
    }
    for (size_t i = 0; i < count; i++) {
        if (fputs(lines[i], f) == EOF || fputc('\n', f) == EOF) {
            fclose(f);
            unlink(tmpl);
            return 0;
        }
    }
    if (fflush(f) != 0) {
        fclose(f);
        unlink(tmpl);
        return 0;
    }
    if (fsync(fileno(f)) != 0) {
        fclose(f);
        unlink(tmpl);
        return 0;
    }
    if (fclose(f) != 0) {
        unlink(tmpl);
        return 0;
    }
    if (rename(tmpl, path) != 0) {
        unlink(tmpl);
        return 0;
    }
    return 1;
}

// Returns "Registered", "Username already exists", "Invalid username format", "Password does not meet strength requirements", or "Internal error"
int register_user(const char* username, const char* password, const char* db_path, char* outbuf, size_t outbuf_len) {
    if (!username || !password || !db_path || !outbuf || outbuf_len == 0) return -1;
    const char* msg = NULL;

    if (!is_valid_username(username)) {
        msg = "Invalid username format";
        goto done;
    }
    if (!is_strong_password(password)) {
        msg = "Password does not meet strength requirements";
        goto done;
    }

    unsigned char salt[SALT_LEN];
    if (RAND_bytes(salt, SALT_LEN) != 1) {
        msg = "Internal error";
        goto done;
    }
    unsigned char dk[DK_LEN];
    if (PKCS5_PBKDF2_HMAC(password, (int)strlen(password), salt, SALT_LEN, PBKDF2_ITERATIONS, EVP_sha256(), DK_LEN, dk) != 1) {
        msg = "Internal error";
        goto done;
    }

    char** lines = NULL;
    size_t count = 0;
    if (access(db_path, F_OK) == 0) {
        if (!read_db(db_path, &lines, &count)) {
            msg = "Internal error";
            goto cleanse;
        }
    } else {
        lines = (char**)calloc(1, sizeof(char*));
        if (!lines) { msg = "Internal error"; goto cleanse; }
        lines[0] = strdup(DB_MAGIC);
        if (!lines[0]) { free(lines); msg = "Internal error"; goto cleanse; }
        count = 1;
    }

    for (size_t i = 1; i < count; i++) {
        char* line = lines[i];
        char* saveptr = NULL;
        char* u = strtok_r(line, "|", &saveptr);
        char* iters = strtok_r(NULL, "|", &saveptr);
        char* s = strtok_r(NULL, "|", &saveptr);
        char* h = strtok_r(NULL, "|", &saveptr);
        (void)iters; (void)s; (void)h;
        if (u && strcmp(u, username) == 0) {
            // Need to restore line? Not required after this point.
            msg = "Username already exists";
            goto free_lines;
        }
        // Reconstruct original line content for safety (not strictly necessary here since we only append)
    }

    {
        char salthex[SALT_LEN*2 + 1];
        char dkhex[DK_LEN*2 + 1];
        to_hex(salt, SALT_LEN, salthex, sizeof(salthex));
        to_hex(dk, DK_LEN, dkhex, sizeof(dkhex));
        size_t rec_len = strlen(username) + 1 + 10 + 1 + strlen(salthex) + 1 + strlen(dkhex) + 1;
        char* rec = (char*)malloc(rec_len);
        if (!rec) { msg = "Internal error"; goto free_lines; }
        snprintf(rec, rec_len, "%s|%d|%s|%s", username, PBKDF2_ITERATIONS, salthex, dkhex);

        char** new_lines = (char**)realloc(lines, (count + 1) * sizeof(char*));
        if (!new_lines) { free(rec); msg = "Internal error"; goto free_lines; }
        lines = new_lines;
        lines[count] = rec;
        count++;

        if (!write_db_atomically(db_path, lines, count)) {
            msg = "Internal error";
            goto free_lines;
        }
    }
    msg = "Registered";

free_lines:
    if (lines) {
        for (size_t i = 0; i < count; i++) free(lines[i]);
        free(lines);
    }

cleanse:
    OPENSSL_cleanse((void*)password, strlen(password)); // advisory; do not use password after this
    // dk already written; no sensitive need after
done:
    snprintf(outbuf, outbuf_len, "%s", msg ? msg : "Internal error");
    return (strcmp(outbuf, "Registered") == 0) ? 0 : -1;
}

int main(void) {
    const char* db_path = "users_c.db";
    if (access(db_path, F_OK) != 0) {
        char* init[1];
        init[0] = (char*)DB_MAGIC;
        if (!write_db_atomically(db_path, init, 1)) {
            fprintf(stderr, "Failed to init DB\n");
            return 1;
        }
    }

    struct Test { const char* u; const char* p; } tests[5] = {
        {"alice_01", "Str0ng!Password#1"},
        {"alice_01", "Anoth3r!Str0ngPass"}, // duplicate
        {"bob", "N0tStrong"}, // weak
        {"X", "Weak123!"}, // invalid username + weak
        {"charlie-user", "V3ry$trongPassw0rd!"}
    };

    for (int i = 0; i < 5; i++) {
        char out[128];
        int rc = register_user(tests[i].u, tests[i].p, db_path, out, sizeof(out));
        printf("Test %d register(%s): %s\n", i+1, tests[i].u, out);
        (void)rc;
    }
    return 0;
}