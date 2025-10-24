#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#define MAX_USERS 100
#define MAX_OTPS 100
#define USERNAME_MAX 64
#define SALT_LEN 16
#define HASH_LEN 32
#define PBKDF2_ITER 120000
#define OTP_TTL_SECONDS 120

typedef struct {
    char username[USERNAME_MAX + 1];
    unsigned char salt[SALT_LEN];
    unsigned char hash[HASH_LEN];
    int iterations;
    int used;
} User;

typedef struct {
    char username[USERNAME_MAX + 1];
    char otp[7]; // 6 digits + null
    time_t expires_at;
    int used;
    int present;
} PendingOtp;

static User users[MAX_USERS];
static int user_count = 0;

static PendingOtp pending[MAX_OTPS];

static int timing_safe_eq(const unsigned char* a, const unsigned char* b, size_t len) {
    unsigned char r = 0;
    for (size_t i = 0; i < len; i++) r |= (a[i] ^ b[i]);
    return r == 0;
}

static int timing_safe_eq_str(const char* a, const char* b) {
    size_t la = strlen(a), lb = strlen(b);
    if (la != lb) return 0;
    unsigned char r = 0;
    for (size_t i = 0; i < la; i++) r |= ((unsigned char)a[i] ^ (unsigned char)b[i]);
    return r == 0;
}

static int pbkdf2_hash(const char* password, const unsigned char* salt, int salt_len, int iterations, unsigned char* out, int out_len) {
    return PKCS5_PBKDF2_HMAC(password, (int)strlen(password), salt, salt_len, iterations, EVP_sha256(), out_len, out) == 1;
}

static int find_user_index(const char* username) {
    for (int i = 0; i < user_count; i++) {
        if (strncmp(users[i].username, username, USERNAME_MAX) == 0) return i;
    }
    return -1;
}

static int find_pending_index(const char* username) {
    for (int i = 0; i < MAX_OTPS; i++) {
        if (pending[i].present && strncmp(pending[i].username, username, USERNAME_MAX) == 0) return i;
    }
    return -1;
}

int register_user(const char* username, const char* password) {
    if (!username || !password) return 0;
    if (strlen(username) == 0 || strlen(username) > USERNAME_MAX || strlen(password) < 8) return 0;
    if (find_user_index(username) != -1) return 0;
    if (user_count >= MAX_USERS) return 0;

    User u;
    memset(&u, 0, sizeof(u));
    strncpy(u.username, username, USERNAME_MAX);
    if (RAND_bytes(u.salt, SALT_LEN) != 1) return 0;
    if (!pbkdf2_hash(password, u.salt, SALT_LEN, PBKDF2_ITER, u.hash, HASH_LEN)) return 0;
    u.iterations = PBKDF2_ITER;
    users[user_count++] = u;
    return 1;
}

static int generate_otp(char out[7]) {
    // Rejection sampling to avoid modulo bias
    unsigned int val;
    const unsigned int max = 4294000000u;
    while (1) {
        if (RAND_bytes((unsigned char*)&val, sizeof(val)) != 1) continue;
        if (val < max) {
            unsigned int n = val % 1000000u;
            snprintf(out, 7, "%06u", n);
            return 1;
        }
    }
    // unreachable
    // return 0;
}

char* login_request(const char* username, const char* password) {
    int idx = find_user_index(username);
    if (idx == -1) return NULL;
    unsigned char cand[HASH_LEN];
    if (!pbkdf2_hash(password, users[idx].salt, SALT_LEN, users[idx].iterations, cand, HASH_LEN)) return NULL;
    if (!timing_safe_eq(cand, users[idx].hash, HASH_LEN)) return NULL;

    int pidx = find_pending_index(username);
    if (pidx == -1) {
        for (int i = 0; i < MAX_OTPS; i++) {
            if (!pending[i].present) { pidx = i; break; }
        }
    }
    if (pidx == -1) return NULL;

    PendingOtp po;
    memset(&po, 0, sizeof(po));
    strncpy(po.username, username, USERNAME_MAX);
    if (!generate_otp(po.otp)) return NULL;
    po.expires_at = time(NULL) + OTP_TTL_SECONDS;
    po.used = 0;
    po.present = 1;
    pending[pidx] = po;

    char* ret = (char*)malloc(7);
    if (!ret) return NULL;
    memcpy(ret, po.otp, 7);
    return ret;
}

int verify_otp(const char* username, const char* otp) {
    int idx = find_pending_index(username);
    if (idx == -1) return 0;
    PendingOtp* po = &pending[idx];
    time_t now = time(NULL);
    if (po->used || now > po->expires_at) {
        po->present = 0;
        return 0;
    }
    int ok = timing_safe_eq_str(po->otp, otp);
    if (ok) {
        po->used = 1;
        po->present = 0;
        return 1;
    }
    return 0;
}

int main() {
    // Test 1: Successful login with correct password and OTP
    printf("Register alice: %s\n", register_user("alice", "password123") ? "true" : "false");
    char* otp1 = login_request("alice", "password123");
    printf("OTP issued (alice): %s\n", otp1 ? otp1 : "None");
    printf("Verify correct OTP (alice): %s\n", (otp1 && verify_otp("alice", otp1)) ? "true" : "false");
    if (otp1) free(otp1);

    // Test 2: Wrong password
    char* otpWrong = login_request("alice", "wrongpass!");
    printf("Login with wrong password returns OTP? %s\n", otpWrong ? "true" : "false");
    if (otpWrong) free(otpWrong);

    // Test 3: Wrong OTP
    char* otp2 = login_request("alice", "password123");
    printf("OTP issued (alice) second time: %s\n", otp2 ? otp2 : "None");
    printf("Verify wrong OTP (alice): %s\n", verify_otp("alice", "000000") ? "true" : "false");
    if (otp2) free(otp2);

    // Test 4: Nonexistent user
    char* otpNoUser = login_request("bob", "anything123");
    printf("OTP issued (nonexistent user bob): %s\n", otpNoUser ? otpNoUser : "None");
    if (otpNoUser) free(otpNoUser);

    // Test 5: OTP reuse should fail
    char* otp3 = login_request("alice", "password123");
    printf("OTP issued (alice third time): %s\n", otp3 ? otp3 : "None");
    int firstUse = (otp3 && verify_otp("alice", otp3)) ? 1 : 0;
    int secondUse = (otp3 && verify_otp("alice", otp3)) ? 1 : 0;
    printf("First use success? %s, second reuse should fail: %s\n", firstUse ? "true" : "false", secondUse ? "true" : "false");
    if (otp3) free(otp3);

    return 0;
}