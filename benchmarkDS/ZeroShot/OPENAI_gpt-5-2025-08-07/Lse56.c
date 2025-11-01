#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

#define SALT_LEN 16
#define HASH_LEN 32
#define ITERATIONS 100000

typedef struct {
    char email[321]; // max 320 + null
    unsigned char salt[SALT_LEN];
    unsigned char passHash[HASH_LEN];
    int loggedIn;
} User;

typedef struct {
    int success;
    char message[128];
} ChangeResult;

int isEmailValid(const char* email) {
    if (!email) return 0;
    size_t len = strlen(email);
    if (len == 0 || len > 320) return 0;
    const char* at = strchr(email, '@');
    if (!at || at == email) return 0;
    const char* dot = strrchr(at, '.');
    if (!dot || dot <= at + 1 || dot == email + len - 1) return 0;
    return 1;
}

int deriveKey(const char* password, const unsigned char* salt, unsigned char* out, size_t outlen) {
    if (!password || !salt || !out) return 0;
    int rc = PKCS5_PBKDF2_HMAC(password, (int)strlen(password),
                               salt, SALT_LEN, ITERATIONS,
                               EVP_sha256(), (int)outlen, out);
    return rc == 1;
}

int constantTimeEquals(const unsigned char* a, const unsigned char* b, size_t len) {
    if (!a || !b) return 0;
    return CRYPTO_memcmp(a, b, len) == 0;
}

User createUser(const char* email, const char* password) {
    User u;
    memset(&u, 0, sizeof(User));
    if (!isEmailValid(email)) {
        fprintf(stderr, "Invalid email\n");
        return u;
    }
    strncpy(u.email, email, sizeof(u.email) - 1);
    if (RAND_bytes(u.salt, SALT_LEN) != 1) {
        fprintf(stderr, "Salt generation failed\n");
        return u;
    }
    if (!deriveKey(password, u.salt, u.passHash, HASH_LEN)) {
        fprintf(stderr, "Hashing failed\n");
        memset(&u, 0, sizeof(User));
        return u;
    }
    u.loggedIn = 0;
    return u;
}

int login(User* user, const char* password) {
    if (!user || !password) return 0;
    unsigned char computed[HASH_LEN];
    if (!deriveKey(password, user->salt, computed, HASH_LEN)) return 0;
    int ok = constantTimeEquals(computed, user->passHash, HASH_LEN);
    OPENSSL_cleanse(computed, sizeof(computed));
    user->loggedIn = ok ? 1 : 0;
    return ok;
}

ChangeResult changeEmail(User* user, const char* oldEmail, const char* newEmail,
                         const char* password, const char* confirmPassword) {
    ChangeResult res;
    res.success = 0;
    snprintf(res.message, sizeof(res.message), "Unknown error");
    if (!user) { snprintf(res.message, sizeof(res.message), "User not found"); return res; }
    if (!user->loggedIn) { snprintf(res.message, sizeof(res.message), "User not logged in"); return res; }
    if (!oldEmail || !newEmail || !password || !confirmPassword) {
        snprintf(res.message, sizeof(res.message), "Missing parameters"); return res;
    }
    if (strcmp(user->email, oldEmail) != 0) {
        snprintf(res.message, sizeof(res.message), "Old email does not match"); return res;
    }
    if (!isEmailValid(newEmail)) {
        snprintf(res.message, sizeof(res.message), "Invalid new email format"); return res;
    }
    if (strcasecmp(oldEmail, newEmail) == 0) {
        snprintf(res.message, sizeof(res.message), "New email must be different from old email"); return res;
    }
    if (strcmp(password, confirmPassword) != 0) {
        snprintf(res.message, sizeof(res.message), "Password confirmation does not match"); return res;
    }
    unsigned char computed[HASH_LEN];
    if (!deriveKey(password, user->salt, computed, HASH_LEN)) {
        snprintf(res.message, sizeof(res.message), "Authentication failed"); return res;
    }
    int ok = constantTimeEquals(computed, user->passHash, HASH_LEN);
    OPENSSL_cleanse(computed, sizeof(computed));
    if (!ok) {
        snprintf(res.message, sizeof(res.message), "Authentication failed"); return res;
    }
    strncpy(user->email, newEmail, sizeof(user->email) - 1);
    user->email[sizeof(user->email) - 1] = '\0';
    res.success = 1;
    snprintf(res.message, sizeof(res.message), "Email updated");
    return res;
}

int main(void) {
    // Test 1: Successful change
    User u1 = createUser("alice@example.com", "Str0ngP@ss!");
    login(&u1, "Str0ngP@ss!");
    ChangeResult r1 = changeEmail(&u1, "alice@example.com", "alice2@example.com", "Str0ngP@ss!", "Str0ngP@ss!");
    printf("Test1: %s: %s\n", r1.success ? "SUCCESS" : "ERROR", r1.message);

    // Test 2: Not logged in
    User u2 = createUser("bob@example.com", "S3cure#Pass");
    ChangeResult r2 = changeEmail(&u2, "bob@example.com", "bob2@example.com", "S3cure#Pass", "S3cure#Pass");
    printf("Test2: %s: %s\n", r2.success ? "SUCCESS" : "ERROR", r2.message);

    // Test 3: Wrong password
    User u3 = createUser("carol@example.com", "P@ssw0rd!");
    login(&u3, "P@ssw0rd!");
    ChangeResult r3 = changeEmail(&u3, "carol@example.com", "carol2@example.com", "WrongPass!", "WrongPass!");
    printf("Test3: %s: %s\n", r3.success ? "SUCCESS" : "ERROR", r3.message);

    // Test 4: Wrong old email
    User u4 = createUser("dave@example.com", "GoodP@ss1");
    login(&u4, "GoodP@ss1");
    ChangeResult r4 = changeEmail(&u4, "notdave@example.com", "dave2@example.com", "GoodP@ss1", "GoodP@ss1");
    printf("Test4: %s: %s\n", r4.success ? "SUCCESS" : "ERROR", r4.message);

    // Test 5: New email same as old
    User u5 = createUser("eve@example.com", "Sup3r#Pass");
    login(&u5, "Sup3r#Pass");
    ChangeResult r5 = changeEmail(&u5, "eve@example.com", "eve@example.com", "Sup3r#Pass", "Sup3r#Pass");
    printf("Test5: %s: %s\n", r5.success ? "SUCCESS" : "ERROR", r5.message);

    return 0;
}