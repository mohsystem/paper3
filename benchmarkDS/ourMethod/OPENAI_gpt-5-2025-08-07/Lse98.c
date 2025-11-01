/*
Build note: This program uses OpenSSL (for PBKDF2 and CSPRNG).
Example compile on Unix-like systems:
  cc -std=c11 -O2 -Wall -Wextra -pedantic reg.c -lssl -lcrypto
*/
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define ITERATIONS 210000
#define SALT_LEN 16
#define DK_LEN 32
#define MAX_USERS 100000
#define MAX_LINE 4096

static bool is_valid_username(const char* u) {
    size_t len = strlen(u);
    if (len < 3 || len > 32) return false;
    for (size_t i = 0; i < len; i++) {
        char c = u[i];
        if (!(isalnum((unsigned char)c) || c == '_' || c == '.' || c == '-')) return false;
    }
    return true;
}

static const char* check_password_policy(const char* p) {
    size_t len = strlen(p);
    if (len < 12) return "Password must be at least 12 characters long";
    bool hasU=false, hasL=false, hasD=false, hasS=false;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)p[i];
        if (isupper(c)) hasU = true;
        else if (islower(c)) hasL = true;
        else if (isdigit(c)) hasD = true;
        else hasS = true;
    }
    if (!(hasU && hasL && hasD && hasS)) {
        return "Password must include upper, lower, digit, and special character";
    }
    return NULL;
}

static void to_hex(const unsigned char* in, size_t len, char* out) {
    static const char* hex = "0123456789abcdef";
    for (size_t i = 0; i < len; i++) {
        out[2*i]   = hex[(in[i] >> 4) & 0xF];
        out[2*i+1] = hex[in[i] & 0xF];
    }
    out[2*len] = '\0';
}

static bool read_db(const char* path, char*** lines_out, size_t* n_out, bool* user_exists, const char* username) {
    *lines_out = NULL;
    *n_out = 0;
    *user_exists = false;
    FILE* f = fopen(path, "r");
    if (!f) return true; // missing is ok
    size_t cap = 16;
    char** lines = (char**)malloc(cap * sizeof(char*));
    if (!lines) { fclose(f); return false; }

    char buf[MAX_LINE];
    size_t count = 0;
    while (fgets(buf, sizeof(buf), f)) {
        size_t L = strnlen(buf, sizeof(buf));
        if (L == sizeof(buf)-1 && buf[L-1] != '\n') {
            // line too long; treat as error
            for (size_t i = 0; i < count; i++) free(lines[i]);
            free(lines);
            fclose(f);
            return false;
        }
        if (L > 0 && buf[L-1] == '\n') buf[L-1] = '\0';
        if (buf[0] == '\0' || buf[0] == '#') continue;
        // check username
        char* pipe = strchr(buf, '|');
        if (pipe) {
            *pipe = '\0';
            if (strcmp(buf, username) == 0) *user_exists = true;
            *pipe = '|';
        }
        if (count == cap) {
            size_t ncap = cap * 2;
            char** tmp = (char**)realloc(lines, ncap * sizeof(char*));
            if (!tmp) {
                for (size_t i = 0; i < count; i++) free(lines[i]);
                free(lines);
                fclose(f);
                return false;
            }
            cap = ncap;
            lines = tmp;
        }
        lines[count] = strdup(buf);
        if (!lines[count]) {
            for (size_t i = 0; i < count; i++) free(lines[i]);
            free(lines);
            fclose(f);
            return false;
        }
        count++;
        if (count > MAX_USERS) {
            for (size_t i = 0; i < count; i++) free(lines[i]);
            free(lines);
            fclose(f);
            return false;
        }
    }
    fclose(f);
    *lines_out = lines;
    *n_out = count;
    return true;
}

static bool atomic_write(const char* dir, const char* final_path, const char* content) {
    char tmpl[1024];
    int n = snprintf(tmpl, sizeof(tmpl), "%s/%s", dir, "users_tmp_XXXXXX");
    if (n < 0 || (size_t)n >= sizeof(tmpl)) return false;
    int fd = mkstemp(tmpl);
    if (fd == -1) return false;
    fchmod(fd, 0600);
    size_t len = strlen(content);
    size_t off = 0;
    while (off < len) {
        ssize_t w = write(fd, content + off, len - off);
        if (w < 0) {
            close(fd);
            unlink(tmpl);
            return false;
        }
        off += (size_t)w;
    }
    if (fsync(fd) != 0) {
        close(fd);
        unlink(tmpl);
        return false;
    }
    if (close(fd) != 0) {
        unlink(tmpl);
        return false;
    }
    if (rename(tmpl, final_path) != 0) {
        unlink(tmpl);
        return false;
    }
    return true;
}

// Returns a heap-allocated message string ("OK" or "ERROR: ..."). Caller must free().
char* register_user(const char* db_path, const char* username, const char* password) {
    if (!db_path || !*db_path) return strdup("ERROR: Invalid database path");
    if (!username || !password) return strdup("ERROR: Null input");
    if (!is_valid_username(username)) {
        return strdup("ERROR: Invalid username (3-32 chars; allowed A-Z, a-z, 0-9, _ . -)");
    }
    const char* pol = check_password_policy(password);
    if (pol) {
        char* out = (char*)malloc(strlen("ERROR: ") + strlen(pol) + 1);
        if (!out) return strdup("ERROR: Memory");
        strcpy(out, "ERROR: ");
        strcat(out, pol);
        return out;
    }

    char** lines = NULL;
    size_t nlines = 0;
    bool user_exists = false;
    if (!read_db(db_path, &lines, &nlines, &user_exists, username)) {
        return strdup("ERROR: Database too large or unreadable");
    }
    if (user_exists) {
        for (size_t i = 0; i < nlines; i++) free(lines[i]);
        free(lines);
        return strdup("ERROR: Username already exists");
    }

    unsigned char salt[SALT_LEN];
    if (RAND_bytes(salt, SALT_LEN) != 1) {
        for (size_t i = 0; i < nlines; i++) free(lines[i]);
        free(lines);
        return strdup("ERROR: RNG failure");
    }
    unsigned char dk[DK_LEN];
    if (PKCS5_PBKDF2_HMAC(password, (int)strlen(password), salt, SALT_LEN, ITERATIONS, EVP_sha256(), DK_LEN, dk) != 1) {
        OPENSSL_cleanse(salt, SALT_LEN);
        for (size_t i = 0; i < nlines; i++) free(lines[i]);
        free(lines);
        return strdup("ERROR: Hashing failed");
    }

    char salt_hex[SALT_LEN*2 + 1];
    char dk_hex[DK_LEN*2 + 1];
    to_hex(salt, SALT_LEN, salt_hex);
    to_hex(dk, DK_LEN, dk_hex);
    OPENSSL_cleanse(salt, SALT_LEN);
    OPENSSL_cleanse(dk, DK_LEN);

    // Build content
    size_t total_len = 0;
    for (size_t i = 0; i < nlines; i++) total_len += strlen(lines[i]) + 1;
    const char* sep = "|";
    size_t rec_len = strlen(username) + 1 + 6 + 1 + strlen(salt_hex) + 1 + strlen(dk_hex) + 1; // approx: iterations up to 6 digits
    char* content = (char*)malloc(total_len + rec_len + 1);
    if (!content) {
        for (size_t i = 0; i < nlines; i++) free(lines[i]);
        free(lines);
        return strdup("ERROR: Memory");
    }
    content[0] = '\0';
    for (size_t i = 0; i < nlines; i++) {
        strcat(content, lines[i]);
        strcat(content, "\n");
        free(lines[i]);
    }
    free(lines);

    char iter_buf[16];
    snprintf(iter_buf, sizeof(iter_buf), "%d", ITERATIONS);
    strcat(content, username);
    strcat(content, sep);
    strcat(content, iter_buf);
    strcat(content, sep);
    strcat(content, salt_hex);
    strcat(content, sep);
    strcat(content, dk_hex);
    strcat(content, "\n");

    // Determine directory
    char dir[1024];
    const char* slash = strrchr(db_path, '/');
#ifdef _WIN32
    const char* bslash = strrchr(db_path, '\\');
    if (!slash || (bslash && bslash > slash)) slash = bslash;
#endif
    if (slash) {
        size_t dlen = (size_t)(slash - db_path);
        if (dlen >= sizeof(dir)) {
            free(content);
            return strdup("ERROR: Path too long");
        }
        memcpy(dir, db_path, dlen);
        dir[dlen] = '\0';
    } else {
        strcpy(dir, ".");
    }

    bool ok = atomic_write(dir, db_path, content);
    free(content);
    if (!ok) {
        return strdup("ERROR: Unable to write database");
    }
    return strdup("OK");
}

int main(void) {
    unsigned char rnd[8];
    RAND_bytes(rnd, sizeof(rnd));
    char hex[17];
    to_hex(rnd, sizeof(rnd), hex);
    char db_path[64];
    snprintf(db_path, sizeof(db_path), "users_c_%s.db", hex);

    const char* users[5] = {"alice","bob","invalid name","alice","charlie"};
    const char* pwds[5]  = {"Str0ngP@ssw0rd!","weakpass","Val1dP@ssw0rd!!","An0ther$tr0ng!","An0th3r$tr0ng!"};

    for (int i = 0; i < 5; i++) {
        char* res = register_user(db_path, users[i], pwds[i]);
        if (res) {
            printf("Test %d: %s\n", i+1, res);
            free(res);
        } else {
            printf("Test %d: ERROR: Unknown\n", i+1);
        }
    }
    return 0;
}