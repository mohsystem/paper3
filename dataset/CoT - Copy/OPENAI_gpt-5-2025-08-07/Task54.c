/* 
Chain-of-Through Step 1: Problem understanding
Implement 2FA by generating secure random numeric OTPs, validating with expiry and attempt limits.

Chain-of-Through Step 2: Security requirements
- Use OS CSPRNG (/dev/urandom) for randomness
- Constant-time comparisons
- Enforce expiry and attempt limits
- Avoid leaking OTPs (printed only for demo tests)
*/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

/* Secure random byte from /dev/urandom */
static int secure_random_byte(unsigned char* out) {
    FILE* f = fopen("/dev/urandom", "rb");
    if (!f) return 0;
    size_t n = fread(out, 1, 1, f);
    fclose(f);
    return n == 1;
}

/* Generate a single random digit [0-9] using rejection sampling to avoid bias */
static int secure_random_digit() {
    for (int i = 0; i < 16; ++i) {
        unsigned char b;
        if (!secure_random_byte(&b)) break;
        if (b < 250) { // 250 is divisible by 10
            return b % 10;
        }
    }
    // Fallback (should not happen): still use /dev/urandom byte modulo 10 with best effort
    unsigned char b = 0;
    if (!secure_random_byte(&b)) {
        // As a last resort, abort for security
        fprintf(stderr, "CSPRNG not available\n");
        exit(1);
    }
    return b % 10;
}

/* Constant-time string compare */
static int ct_eq(const char* a, const char* b) {
    if (!a || !b) return 0;
    size_t la = strlen(a), lb = strlen(b);
    size_t len = la < lb ? la : lb;
    unsigned char acc = 0;
    for (size_t i = 0; i < len; ++i) {
        acc |= (unsigned char)a[i] ^ (unsigned char)b[i];
    }
    acc |= (unsigned char)la ^ (unsigned char)lb;
    return acc == 0;
}

/* Simple user store (demo only) */
typedef struct {
    char username[64];
    char password[64];
    int in_use;
} User;

typedef struct {
    char user[64];
    char otp[16];
    time_t expires_at;
    int attempts_left;
    int valid;
    int in_use;
} OtpRecord;

typedef struct {
    int otp_length;
    int ttl_seconds;
    int max_attempts;
    OtpRecord records[128];
} TwoFAService;

/* Initialize 2FA service */
static void twofa_init(TwoFAService* svc, int otp_length, int ttl_seconds, int max_attempts) {
    if (!svc) return;
    if (otp_length < 4 || otp_length > 12) {
        fprintf(stderr, "OTP length out of bounds\n");
        exit(1);
    }
    if (ttl_seconds < 1) {
        fprintf(stderr, "TTL too short\n");
        exit(1);
    }
    if (max_attempts < 1) {
        fprintf(stderr, "Attempts must be >=1\n");
        exit(1);
    }
    svc->otp_length = otp_length;
    svc->ttl_seconds = ttl_seconds;
    svc->max_attempts = max_attempts;
    for (int i = 0; i < 128; ++i) svc->records[i].in_use = 0;
}

/* Find or allocate an OTP record slot for a user */
static OtpRecord* twofa_get_record(TwoFAService* svc, const char* user) {
    for (int i = 0; i < 128; ++i) {
        if (svc->records[i].in_use && strcmp(svc->records[i].user, user) == 0) {
            return &svc->records[i];
        }
    }
    for (int i = 0; i < 128; ++i) {
        if (!svc->records[i].in_use) {
            svc->records[i].in_use = 1;
            strncpy(svc->records[i].user, user, sizeof(svc->records[i].user)-1);
            svc->records[i].user[sizeof(svc->records[i].user)-1] = '\0';
            return &svc->records[i];
        }
    }
    return NULL;
}

/* Generate OTP; returns 1 on success, 0 on failure */
static int twofa_generate_otp(TwoFAService* svc, const char* user, char* out_otp, size_t out_size) {
    if (!svc || !user || !out_otp || out_size < (size_t)(svc->otp_length + 1)) return 0;
    OtpRecord* rec = twofa_get_record(svc, user);
    if (!rec) return 0;
    for (int i = 0; i < svc->otp_length; ++i) {
        int d = secure_random_digit();
        out_otp[i] = (char)('0' + d);
    }
    out_otp[svc->otp_length] = '\0';
    strncpy(rec->otp, out_otp, sizeof(rec->otp)-1);
    rec->otp[sizeof(rec->otp)-1] = '\0';
    rec->expires_at = time(NULL) + svc->ttl_seconds;
    rec->attempts_left = svc->max_attempts;
    rec->valid = 1;
    return 1;
}

/* Verify OTP; returns 1 on success, 0 on failure */
static int twofa_verify_otp(TwoFAService* svc, const char* user, const char* otp_input) {
    if (!svc || !user || !otp_input) return 0;
    OtpRecord* rec = twofa_get_record(svc, user);
    if (!rec || !rec->in_use || !rec->valid) return 0;
    time_t now = time(NULL);
    if (now > rec->expires_at) {
        rec->valid = 0;
        return 0;
    }
    if (rec->attempts_left <= 0) {
        rec->valid = 0;
        return 0;
    }
    int ok = ct_eq(rec->otp, otp_input);
    if (ok) {
        rec->valid = 0;
        return 1;
    } else {
        rec->attempts_left--;
        if (rec->attempts_left <= 0) rec->valid = 0;
        return 0;
    }
}

/* Test helper to expire OTP immediately */
static void twofa_expire_now_for_test(TwoFAService* svc, const char* user) {
    OtpRecord* rec = twofa_get_record(svc, user);
    if (rec) rec->expires_at = time(NULL) - 1;
}

/* User store functions */
static void user_add(User* users, int capacity, const char* username, const char* password) {
    for (int i = 0; i < capacity; ++i) {
        if (!users[i].in_use) {
            users[i].in_use = 1;
            strncpy(users[i].username, username, sizeof(users[i].username)-1);
            users[i].username[sizeof(users[i].username)-1] = '\0';
            strncpy(users[i].password, password, sizeof(users[i].password)-1);
            users[i].password[sizeof(users[i].password)-1] = '\0';
            return;
        }
    }
}

static int user_verify_password(User* users, int capacity, const char* username, const char* password) {
    for (int i = 0; i < capacity; ++i) {
        if (users[i].in_use && strcmp(users[i].username, username) == 0) {
            return ct_eq(users[i].password, password);
        }
    }
    return 0;
}

static int login_step1(User* users, int capacity, const char* username, const char* password) {
    return user_verify_password(users, capacity, username, password);
}

int main() {
    /* Chain-of-Through Step 3: Secure coding generation */
    User users[16];
    for (int i = 0; i < 16; ++i) users[i].in_use = 0;
    user_add(users, 16, "alice", "pass123");
    user_add(users, 16, "bob", "secret!");
    user_add(users, 16, "carol", "p@ssw0rd");
    user_add(users, 16, "dave", "letmein");
    user_add(users, 16, "erin", "hunter2");

    TwoFAService svc;
    twofa_init(&svc, 6, 2, 3); /* 2s TTL for demo */

    /* Test 1: Success */
    {
        int pw = login_step1(users, 16, "alice", "pass123");
        char otp[16];
        if (pw) twofa_generate_otp(&svc, "alice", otp, sizeof(otp)); else otp[0] = '\0';
        printf("Test1 OTP (simulated send): %s\n", otp);
        int ok = pw && twofa_verify_otp(&svc, "alice", otp);
        printf("Test1 success: %s\n", ok ? "true" : "false");
    }

    /* Test 2: Wrong password */
    {
        int pw = login_step1(users, 16, "bob", "wrong");
        printf("Test2 success (should be false): %s\n", pw ? "true" : "false");
    }

    /* Test 3: Wrong OTP */
    {
        int pw = login_step1(users, 16, "carol", "p@ssw0rd");
        char otp[16];
        if (pw) twofa_generate_otp(&svc, "carol", otp, sizeof(otp)); else otp[0] = '\0';
        printf("Test3 OTP (simulated send): %s\n", otp);
        int ok = pw && twofa_verify_otp(&svc, "carol", "000000");
        printf("Test3 success (should be false): %s\n", ok ? "true" : "false");
    }

    /* Test 4: Expired OTP */
    {
        int pw = login_step1(users, 16, "dave", "letmein");
        char otp[16];
        if (pw) twofa_generate_otp(&svc, "dave", otp, sizeof(otp)); else otp[0] = '\0';
        printf("Test4 OTP (simulated send): %s\n", otp);
        twofa_expire_now_for_test(&svc, "dave");
        int ok = pw && twofa_verify_otp(&svc, "dave", otp);
        printf("Test4 success (should be false): %s\n", ok ? "true" : "false");
    }

    /* Test 5: Attempt limit exceeded */
    {
        int pw = login_step1(users, 16, "erin", "hunter2");
        char otp[16];
        if (pw) twofa_generate_otp(&svc, "erin", otp, sizeof(otp)); else otp[0] = '\0';
        printf("Test5 OTP (simulated send): %s\n", otp);
        int a = twofa_verify_otp(&svc, "erin", "111111");
        int b = twofa_verify_otp(&svc, "erin", "222222");
        int c = twofa_verify_otp(&svc, "erin", "333333");
        int d = twofa_verify_otp(&svc, "erin", otp);
        printf("Test5 sequence (should be false,false,false,false): %s,%s,%s,%s\n",
               a ? "true" : "false", b ? "true" : "false", c ? "true" : "false", d ? "true" : "false");
    }

    /* Chain-of-Through Step 4 & 5: Reviewed and finalized */
    return 0;
}