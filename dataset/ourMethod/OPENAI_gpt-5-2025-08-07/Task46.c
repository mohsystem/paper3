#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

#define DB_PATH "users_c.db"
#define MAGIC 0x55535231u /* "USR1" */
#define VERSION 1u

#define MAX_NAME 100u
#define MAX_EMAIL 254u
#define SALT_LEN 16u
#define HASH_LEN 32u
#define PBKDF2_ITERS 210000

static int write_u16_be(FILE* f, uint16_t v) {
    unsigned char b[2];
    b[0] = (unsigned char)((v >> 8) & 0xFFu);
    b[1] = (unsigned char)(v & 0xFFu);
    if (fwrite(b, 1, 2, f) != 2) return 0;
    return 1;
}

static int read_u16_be(FILE* f, uint16_t* v) {
    unsigned char b[2];
    if (fread(b, 1, 2, f) != 2) return 0;
    *v = (uint16_t)(((uint16_t)b[0] << 8) | (uint16_t)b[1]);
    return 1;
}

static int db_init(const char* path) {
    FILE* f = fopen(path, "rb");
    if (f) {
        uint32_t magic = 0;
        unsigned char version = 0;
        if (fread(&magic, 1, sizeof(magic), f) != sizeof(magic)) { fclose(f); return 0; }
        if (fread(&version, 1, sizeof(version), f) != sizeof(version)) { fclose(f); return 0; }
        fclose(f);
        if (magic != MAGIC || version != VERSION) return 0;
        return 1;
    }
    f = fopen(path, "wb");
    if (!f) return 0;
    uint32_t magic = MAGIC;
    unsigned char version = VERSION;
    if (fwrite(&magic, 1, sizeof(magic), f) != sizeof(magic)) { fclose(f); return 0; }
    if (fwrite(&version, 1, sizeof(version), f) != sizeof(version)) { fclose(f); return 0; }
    fclose(f);
    return 1;
}

static int email_exists(const char* path, const char* email) {
    FILE* f = fopen(path, "rb");
    if (!f) return 0;

    uint32_t magic = 0;
    unsigned char version = 0;
    if (fread(&magic, 1, sizeof(magic), f) != sizeof(magic)) { fclose(f); return 0; }
    if (fread(&version, 1, sizeof(version), f) != sizeof(version)) { fclose(f); return 0; }
    if (magic != MAGIC || version != VERSION) { fclose(f); return 0; }

    while (1) {
        uint16_t nameLen = 0, emailLen = 0, saltLen = 0, hashLen = 0;
        size_t r;
        long pos = ftell(f);
        (void)pos;

        if (!read_u16_be(f, &nameLen)) break; /* EOF */
        if (!read_u16_be(f, &emailLen)) { fclose(f); return 0; }
        if (!read_u16_be(f, &saltLen)) { fclose(f); return 0; }
        if (!read_u16_be(f, &hashLen)) { fclose(f); return 0; }

        if (nameLen > MAX_NAME || emailLen > MAX_EMAIL || saltLen > 1024u || hashLen > 1024u) {
            fclose(f);
            return 0;
        }

        if (nameLen > 0) {
            if (fseek(f, (long)nameLen, SEEK_CUR) != 0) { fclose(f); return 0; }
        }

        char* emailBuf = NULL;
        if (emailLen > 0) {
            emailBuf = (char*)malloc(emailLen + 1u);
            if (!emailBuf) { fclose(f); return 0; }
            r = fread(emailBuf, 1, emailLen, f);
            if (r != emailLen) { free(emailBuf); fclose(f); return 0; }
            emailBuf[emailLen] = '\0';
        }

        int match = 0;
        if (emailBuf) {
            if (strcmp(emailBuf, email) == 0) match = 1;
        }

        free(emailBuf);

        if (saltLen > 0) {
            if (fseek(f, (long)saltLen, SEEK_CUR) != 0) { fclose(f); return 0; }
        }
        if (hashLen > 0) {
            if (fseek(f, (long)hashLen, SEEK_CUR) != 0) { fclose(f); return 0; }
        }

        if (match) { fclose(f); return 1; }
    }

    fclose(f);
    return 0;
}

static int add_user(const char* path,
                    const char* name,
                    const char* email,
                    const unsigned char* salt, uint16_t saltLen,
                    const unsigned char* hash, uint16_t hashLen) {
    size_t nameLen = strlen(name);
    size_t emailLen = strlen(email);
    if (nameLen > MAX_NAME || emailLen > MAX_EMAIL) return 0;
    if (saltLen > 0xFFFFu || hashLen > 0xFFFFu) return 0;

    if (!db_init(path)) return 0;

    FILE* f = fopen(path, "ab");
    if (!f) return 0;

    int ok = 1;
    ok &= write_u16_be(f, (uint16_t)nameLen);
    ok &= write_u16_be(f, (uint16_t)emailLen);
    ok &= write_u16_be(f, saltLen);
    ok &= write_u16_be(f, hashLen);

    if (ok && nameLen > 0) ok &= (fwrite(name, 1, nameLen, f) == nameLen);
    if (ok && emailLen > 0) ok &= (fwrite(email, 1, emailLen, f) == emailLen);
    if (ok && saltLen > 0) ok &= (fwrite(salt, 1, saltLen, f) == saltLen);
    if (ok && hashLen > 0) ok &= (fwrite(hash, 1, hashLen, f) == hashLen);

    fclose(f);
    return ok ? 1 : 0;
}

static int validate_name(const char* name) {
    size_t len = strlen(name);
    if (len == 0 || len > MAX_NAME) return 0;
    if (!isalpha((unsigned char)name[0])) return 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)name[i];
        if (isalpha(c) || c == ' ' || c == '-' || c == '.' || c == '\'' || c == '`') {
            continue;
        }
        return 0;
    }
    return 1;
}

static int validate_email(const char* email) {
    size_t len = strlen(email);
    if (len == 0 || len > MAX_EMAIL) return 0;
    const char* at = strchr(email, '@');
    if (!at) return 0;
    const char* last_at = strrchr(email, '@');
    if (at != last_at) return 0; /* multiple '@' */

    size_t local_len = (size_t)(at - email);
    const char* domain = at + 1;
    size_t domain_len = strlen(domain);
    if (local_len < 1 || local_len > 64) return 0;
    if (domain_len < 3 || domain_len > 253) return 0;

    if (email[0] == '.' || email[local_len - 1] == '.') return 0;
    if (domain[0] == '.' || domain[domain_len - 1] == '.') return 0;

    /* Allowed characters checks (simplified) */
    for (size_t i = 0; i < local_len; i++) {
        unsigned char c = (unsigned char)email[i];
        if (!(isalnum(c) || c == '.' || c == '_' || c == '%' || c == '+' || c == '-' )) {
            return 0;
        }
    }
    for (size_t i = 0; i < domain_len; i++) {
        unsigned char c = (unsigned char)domain[i];
        if (!(isalnum(c) || c == '.' || c == '-' )) {
            return 0;
        }
    }
    if (strstr(email, "..") != NULL) return 0;
    /* Domain must contain at least one dot and TLD alpha */
    const char* dot = strrchr(domain, '.');
    if (!dot || dot == domain || *(dot + 1) == '\0') return 0;
    for (const char* p = dot + 1; *p; ++p) {
        if (!isalpha((unsigned char)*p)) return 0;
    }
    return 1;
}

static int validate_password(const char* password) {
    size_t len = strlen(password);
    if (len < 12 || len > 128) return 0;
    int hasLower = 0, hasUpper = 0, hasDigit = 0, hasSpecial = 0;
    const char* specials = "!@#$%^&*()-_=+[]{}|;:'\",.<>?/`~";
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)password[i];
        if (islower(c)) hasLower = 1;
        else if (isupper(c)) hasUpper = 1;
        else if (isdigit(c)) hasDigit = 1;
        else if (strchr(specials, (int)c) != NULL) hasSpecial = 1;
    }
    return (hasLower && hasUpper && hasDigit && hasSpecial) ? 1 : 0;
}

static int hash_password(const char* password,
                         unsigned char* salt_out, size_t salt_len,
                         unsigned char* hash_out, size_t hash_len) {
    if (RAND_bytes(salt_out, (int)salt_len) != 1) return 0;
    const EVP_MD* md = EVP_sha256();
    if (PKCS5_PBKDF2_HMAC(password, (int)strlen(password),
                          salt_out, (int)salt_len,
                          PBKDF2_ITERS, md,
                          (int)hash_len, hash_out) != 1) {
        return 0;
    }
    return 1;
}

typedef enum {
    RR_Success = 0,
    RR_InvalidName,
    RR_InvalidEmail,
    RR_WeakPassword,
    RR_EmailExists,
    RR_InternalError
} RegResult;

static const char* result_to_string(RegResult r) {
    switch (r) {
        case RR_Success: return "Success";
        case RR_InvalidName: return "Invalid name";
        case RR_InvalidEmail: return "Invalid email";
        case RR_WeakPassword: return "Weak password";
        case RR_EmailExists: return "Email already exists";
        case RR_InternalError: return "Internal error";
        default: return "Unknown";
    }
}

static RegResult register_user(char* name, char* email, char* password) {
    if (!validate_name(name)) return RR_InvalidName;
    if (!validate_email(email)) return RR_InvalidEmail;
    if (!validate_password(password)) return RR_WeakPassword;

    if (!db_init(DB_PATH)) return RR_InternalError;

    if (email_exists(DB_PATH, email)) return RR_EmailExists;

    unsigned char salt[SALT_LEN];
    unsigned char hash[HASH_LEN];
    memset(salt, 0, sizeof(salt));
    memset(hash, 0, sizeof(hash));

    int ok = hash_password(password, salt, SALT_LEN, hash, HASH_LEN);

    if (password && strlen(password) > 0) {
        OPENSSL_cleanse(password, strlen(password));
    }

    if (!ok) return RR_InternalError;

    if (!add_user(DB_PATH, name, email, salt, (uint16_t)SALT_LEN, hash, (uint16_t)HASH_LEN)) {
        return RR_InternalError;
    }

    /* Clear sensitive buffers */
    OPENSSL_cleanse(salt, sizeof(salt));
    OPENSSL_cleanse(hash, sizeof(hash));

    return RR_Success;
}

int main(void) {
    /* Reset database for test determinism */
    remove(DB_PATH);
    if (!db_init(DB_PATH)) {
        printf("Failed to initialize database\n");
        return 1;
    }

    struct TestCase {
        char name[128];
        char email[320];
        char password[160];
        const char* label;
    };

    struct TestCase tests[5] = {
        {"Alice Smith", "alice@example.com", "S3cure-P@ssw0rd!", "Test 1: valid"},
        {"Bob Jones", "alice@example.com", "An0ther$trongPass!", "Test 2: duplicate email"},
        {"Charlie", "bob@@example..com", "G00dP@ssword!!", "Test 3: invalid email"},
        {"Dana White", "dana.white@example.org", "short", "Test 4: weak password"},
        {"<script>", "eve@example.net", "Sup3r$trongPass!", "Test 5: invalid name"}
    };

    for (int i = 0; i < 5; i++) {
        RegResult r = register_user(tests[i].name, tests[i].email, tests[i].password);
        printf("%s -> %s\n", tests[i].label, result_to_string(r));
    }

    return 0;
}