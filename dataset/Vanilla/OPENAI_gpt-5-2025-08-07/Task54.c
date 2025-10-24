#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
static void sleep_ms(int ms) { Sleep(ms); }
#else
#include <unistd.h>
static void sleep_ms(int ms) { usleep(ms * 1000); }
#endif

#define MAX_USERS 100
#define MAX_OTPS 100

typedef struct {
    char username[64];
    char password[64];
} User;

typedef struct {
    char username[64];
    char code[16];
    time_t expiry; // seconds since epoch
    int in_use;
} OTPEntry;

typedef struct {
    User users[MAX_USERS];
    int user_count;
    OTPEntry otps[MAX_OTPS];
} AuthService;

void init_service(AuthService* svc) {
    svc->user_count = 0;
    for (int i = 0; i < MAX_OTPS; ++i) svc->otps[i].in_use = 0;
}

void add_user(AuthService* svc, const char* username, const char* password) {
    if (svc->user_count >= MAX_USERS) return;
    strncpy(svc->users[svc->user_count].username, username, sizeof(svc->users[svc->user_count].username)-1);
    strncpy(svc->users[svc->user_count].password, password, sizeof(svc->users[svc->user_count].password)-1);
    svc->user_count++;
}

static int find_user(AuthService* svc, const char* username) {
    for (int i = 0; i < svc->user_count; ++i) {
        if (strcmp(svc->users[i].username, username) == 0) return i;
    }
    return -1;
}

static OTPEntry* get_or_create_otp_entry(AuthService* svc, const char* username) {
    for (int i = 0; i < MAX_OTPS; ++i) {
        if (svc->otps[i].in_use && strcmp(svc->otps[i].username, username) == 0) {
            return &svc->otps[i];
        }
    }
    for (int i = 0; i < MAX_OTPS; ++i) {
        if (!svc->otps[i].in_use) {
            svc->otps[i].in_use = 1;
            strncpy(svc->otps[i].username, username, sizeof(svc->otps[i].username)-1);
            svc->otps[i].code[0] = '\0';
            svc->otps[i].expiry = 0;
            return &svc->otps[i];
        }
    }
    return NULL;
}

static void invalidate_otp(AuthService* svc, const char* username) {
    for (int i = 0; i < MAX_OTPS; ++i) {
        if (svc->otps[i].in_use && strcmp(svc->otps[i].username, username) == 0) {
            svc->otps[i].in_use = 0;
            svc->otps[i].code[0] = '\0';
            svc->otps[i].expiry = 0;
            svc->otps[i].username[0] = '\0';
            return;
        }
    }
}

char* generate_otp(AuthService* svc, const char* username, int length, int ttl_seconds) {
    OTPEntry* entry = get_or_create_otp_entry(svc, username);
    if (!entry) return NULL;
    if (length > 15) length = 15;
    for (int i = 0; i < length; ++i) {
        entry->code[i] = (char)('0' + (rand() % 10));
    }
    entry->code[length] = '\0';
    entry->expiry = time(NULL) + ttl_seconds;
    char* out = (char*)malloc((size_t)length + 1);
    if (!out) return NULL;
    strcpy(out, entry->code);
    return out;
}

int verify_otp(AuthService* svc, const char* username, const char* code) {
    for (int i = 0; i < MAX_OTPS; ++i) {
        if (svc->otps[i].in_use && strcmp(svc->otps[i].username, username) == 0) {
            time_t now = time(NULL);
            if (now > svc->otps[i].expiry) {
                invalidate_otp(svc, username);
                return 0;
            }
            if (strcmp(svc->otps[i].code, code) == 0) {
                invalidate_otp(svc, username); // one-time
                return 1;
            } else {
                return 0;
            }
        }
    }
    return 0;
}

char* start_login(AuthService* svc, const char* username, const char* password, int otp_length, int ttl_seconds) {
    int idx = find_user(svc, username);
    if (idx >= 0 && strcmp(svc->users[idx].password, password) == 0) {
        return generate_otp(svc, username, otp_length, ttl_seconds);
    }
    return NULL;
}

int finish_login(AuthService* svc, const char* username, const char* otp) {
    if (otp == NULL) return 0;
    return verify_otp(svc, username, otp);
}

int main() {
    srand((unsigned int)time(NULL));
    AuthService svc;
    init_service(&svc);
    add_user(&svc, "alice", "password123");
    add_user(&svc, "bob", "qwerty");
    add_user(&svc, "charlie", "letmein");
    add_user(&svc, "dave", "passw0rd");
    add_user(&svc, "eve", "123456");

    // Test 1: Successful login
    char* otp1 = start_login(&svc, "alice", "password123", 6, 120);
    int t1 = finish_login(&svc, "alice", otp1);
    printf("Test1:%s\n", t1 ? "true" : "false");
    if (otp1) free(otp1);

    // Test 2: Wrong password
    char* otp2 = start_login(&svc, "bob", "wrong", 6, 120);
    int t2 = finish_login(&svc, "bob", "000000");
    printf("Test2:%s\n", t2 ? "true" : "false");
    if (otp2) free(otp2);

    // Test 3: Correct password, wrong OTP
    char* otp3 = start_login(&svc, "charlie", "letmein", 6, 120);
    int t3 = finish_login(&svc, "charlie", "111111");
    printf("Test3:%s\n", t3 ? "true" : "false");
    if (otp3) free(otp3);

    // Test 4: Expired OTP
    char* otp4 = start_login(&svc, "dave", "passw0rd", 6, 1);
    sleep_ms(1500);
    int t4 = finish_login(&svc, "dave", otp4);
    printf("Test4:%s\n", t4 ? "true" : "false");
    if (otp4) free(otp4);

    // Test 5: Reuse OTP attempt
    char* otp5 = start_login(&svc, "eve", "123456", 6, 120);
    int t5a = finish_login(&svc, "eve", otp5);
    int t5b = finish_login(&svc, "eve", otp5);
    printf("Test5:%s\n", (t5a && !t5b) ? "true" : "false");
    if (otp5) free(otp5);

    return 0;
}