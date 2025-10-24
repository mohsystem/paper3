#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

#if defined(_WIN32)
  #include <direct.h>
  #include <io.h>
  #include <fcntl.h>
  #include <sys/stat.h>
  #define mkdir(path,mode) _mkdir(path)
  #define O_NOFOLLOW 0
  #define fsync _commit
#else
  #include <unistd.h>
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <fcntl.h>
  #include <dirent.h>
#endif

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

#define SALT_LEN 16
#define KEY_LEN 32
#define PBKDF2_ITERS 210000
#define USER_MAX 32
#define PASS_MAX 256

static const char MAGIC[4] = {'U','C','D','B'};
static const uint8_t VERSION = 1;

typedef struct {
    char username[USER_MAX + 1];
    char password[PASS_MAX + 1];
} LoginRequest;

typedef struct {
    int success;
    char message[128];
} Response;

static void secure_clean_str(char* s) {
    if (s) {
        size_t n = strlen(s);
        if (n > 0) OPENSSL_cleanse(s, n);
    }
}

static int is_valid_username(const char* u) {
    if (!u) return 0;
    size_t len = strlen(u);
    if (len == 0 || len > USER_MAX) return 0;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)u[i];
        if (!(c == '_' || (c >= 'a' && c <= 'z') ||
              (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))) {
            return 0;
        }
    }
    return 1;
}

static int secure_random_bytes(unsigned char* buf, size_t len) {
    if (!buf || len == 0) return 0;
    return RAND_bytes(buf, (int)len) == 1;
}

static int derive_key_pbkdf2(const char* password, const unsigned char* salt, size_t salt_len,
                             unsigned char* out_key, size_t out_len) {
    if (!password || !salt || !out_key || salt_len == 0 || out_len == 0) return 0;
    const EVP_MD* md = EVP_sha256();
    int rc = PKCS5_PBKDF2_HMAC(password, (int)strlen(password),
                               salt, (int)salt_len,
                               PBKDF2_ITERS, md, (int)out_len, out_key);
    return rc == 1;
}

static int const_time_eq(const unsigned char* a, const unsigned char* b, size_t len) {
    if (!a || !b) return 0;
    return CRYPTO_memcmp(a, b, len) == 0;
}

static int ensure_dir_secure(const char* base_dir) {
    if (!base_dir) return 0;
#if defined(_WIN32)
    if (_access(base_dir, 0) != 0) {
        if (mkdir(base_dir, 0700) != 0) return 0;
    }
    // Windows permissions best-effort
#else
    struct stat st;
    if (stat(base_dir, &st) != 0) {
        if (mkdir(base_dir, 0700) != 0) return 0;
    } else {
        if (!S_ISDIR(st.st_mode)) return 0;
    }
    chmod(base_dir, S_IRWXU);
#endif
    return 1;
}

static int build_user_path(char* out, size_t out_sz, const char* base_dir, const char* username) {
    if (!out || !base_dir || !username) return 0;
    int n = snprintf(out, out_sz, "%s/%s.cred", base_dir, username);
    if (n <= 0 || (size_t)n >= out_sz) return 0;
    return 1;
}

static int write_file_atomic(const char* final_path, const unsigned char* data, size_t len) {
    if (!final_path || !data || len == 0) return 0;
    char dir[512];
    const char* slash = strrchr(final_path, '/');
#if defined(_WIN32)
    if (!slash) slash = strrchr(final_path, '\\');
#endif
    size_t dlen = 0;
    if (slash) {
        dlen = (size_t)(slash - final_path);
        if (dlen >= sizeof(dir)) return 0;
        memcpy(dir, final_path, dlen);
        dir[dlen] = '\0';
    } else {
        dir[0] = '.';
        dir[1] = '\0';
    }
    if (!ensure_dir_secure(dir)) return 0;

    char tmp[600];
    int n = snprintf(tmp, sizeof(tmp), "%s/.tmp_%ld_%d.cred", dir, (long)getpid(), rand());
    if (n <= 0 || (size_t)n >= sizeof(tmp)) return 0;

#if defined(_WIN32)
    int fd = _open(tmp, _O_BINARY | _O_CREAT | _O_EXCL | _O_WRONLY, _S_IREAD | _S_IWRITE);
#else
    int fd = open(tmp, O_CREAT | O_EXCL | O_WRONLY, S_IRUSR | S_IWUSR);
#endif
    if (fd < 0) return 0;

    size_t written = 0;
    while (written < len) {
#if defined(_WIN32)
        int w = _write(fd, data + written, (unsigned int)((len - written) > (1<<20) ? (1<<20) : (len - written)));
#else
        ssize_t w = write(fd, data + written, (len - written) > (1<<20) ? (1<<20) : (len - written));
#endif
        if (w <= 0) {
#if defined(_WIN32)
            _close(fd);
            _unlink(tmp);
#else
            close(fd);
            unlink(tmp);
#endif
            return 0;
        }
        written += (size_t)w;
    }
    if (fsync(fd) != 0) {
#if defined(_WIN32)
        _close(fd);
        _unlink(tmp);
#else
        close(fd);
        unlink(tmp);
#endif
        return 0;
    }
#if defined(_WIN32)
    if (_close(fd) != 0) {
        _unlink(tmp);
        return 0;
    }
    if (rename(tmp, final_path) != 0) {
        _unlink(tmp);
        return 0;
    }
#else
    if (close(fd) != 0) {
        unlink(tmp);
        return 0;
    }
    if (rename(tmp, final_path) != 0) {
        unlink(tmp);
        return 0;
    }
    chmod(final_path, S_IRUSR | S_IWUSR);
#endif
    return 1;
}

static int read_file_secure(const char* path, unsigned char** out, size_t* out_len) {
    if (!path || !out || !out_len) return 0;
    *out = NULL;
    *out_len = 0;

#if !defined(_WIN32)
    struct stat st;
    if (lstat(path, &st) != 0) return 0;
    if (!S_ISREG(st.st_mode) || S_ISLNK(st.st_mode)) return 0;
#endif

    FILE* f = fopen(path, "rb");
    if (!f) return 0;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return 0; }
    long sz = ftell(f);
    if (sz <= 0 || sz > 1024*1024) { fclose(f); return 0; }
    if (fseek(f, 0, SEEK_SET) != 0) { fclose(f); return 0; }
    unsigned char* buf = (unsigned char*)malloc((size_t)sz);
    if (!buf) { fclose(f); return 0; }
    size_t n = fread(buf, 1, (size_t)sz, f);
    fclose(f);
    if (n != (size_t)sz) { free(buf); return 0; }
    *out = buf;
    *out_len = n;
    return 1;
}

static int serialize_record(const unsigned char* salt, const unsigned char* key, unsigned char** out, size_t* out_len) {
    if (!salt || !key || !out || !out_len) return 0;
    *out = NULL;
    *out_len = 0;
    size_t total = 4 + 1 + SALT_LEN + KEY_LEN;
    unsigned char* buf = (unsigned char*)malloc(total);
    if (!buf) return 0;
    memcpy(buf, MAGIC, 4);
    buf[4] = VERSION;
    memcpy(buf + 5, salt, SALT_LEN);
    memcpy(buf + 5 + SALT_LEN, key, KEY_LEN);
    *out = buf;
    *out_len = total;
    return 1;
}

static int parse_record(const unsigned char* in, size_t in_len, unsigned char* salt, unsigned char* key) {
    if (!in || !salt || !key) return 0;
    if (in_len != 4 + 1 + SALT_LEN + KEY_LEN) return 0;
    if (memcmp(in, MAGIC, 4) != 0) return 0;
    if (in[4] != VERSION) return 0;
    memcpy(salt, in + 5, SALT_LEN);
    memcpy(key, in + 5 + SALT_LEN, KEY_LEN);
    return 1;
}

static Response server_register_user(const char* base_dir, const char* username, const char* password) {
    Response r = {0, {0}};
    if (!is_valid_username(username)) {
        snprintf(r.message, sizeof(r.message), "%s", "Invalid username format.");
        return r;
    }
    size_t plen = strlen(password);
    if (plen < 10 || plen > PASS_MAX) {
        snprintf(r.message, sizeof(r.message), "%s", "Weak or invalid password length.");
        return r;
    }
    int hasU=0, hasL=0, hasD=0, hasS=0;
    for (size_t i = 0; i < plen; ++i) {
        unsigned char c = (unsigned char)password[i];
        if (c >= 'A' && c <= 'Z') hasU = 1;
        else if (c >= 'a' && c <= 'z') hasL = 1;
        else if (c >= '0' && c <= '9') hasD = 1;
        else hasS = 1;
    }
    if (!(hasU && hasL && hasD && hasS)) {
        snprintf(r.message, sizeof(r.message), "%s", "Password must include upper, lower, digit, and symbol.");
        return r;
    }

    unsigned char salt[SALT_LEN];
    if (!secure_random_bytes(salt, sizeof(salt))) {
        snprintf(r.message, sizeof(r.message), "%s", "RNG failure.");
        return r;
    }
    unsigned char key[KEY_LEN];
    if (!derive_key_pbkdf2(password, salt, sizeof(salt), key, sizeof(key))) {
        snprintf(r.message, sizeof(r.message), "%s", "KDF failure.");
        return r;
    }

    unsigned char* rec = NULL;
    size_t rec_len = 0;
    if (!serialize_record(salt, key, &rec, &rec_len)) {
        snprintf(r.message, sizeof(r.message), "%s", "Serialization failure.");
        return r;
    }

    char path[512];
    if (!build_user_path(path, sizeof(path), base_dir, username)) {
        OPENSSL_cleanse(rec, rec_len);
        free(rec);
        snprintf(r.message, sizeof(r.message), "%s", "Path build error.");
        return r;
    }

    // refuse overwrite
#if defined(_WIN32)
    if (_access(path, 0) == 0) {
        OPENSSL_cleanse(rec, rec_len);
        free(rec);
        snprintf(r.message, sizeof(r.message), "%s", "User already exists.");
        return r;
    }
#else
    if (access(path, F_OK) == 0) {
        OPENSSL_cleanse(rec, rec_len);
        free(rec);
        snprintf(r.message, sizeof(r.message), "%s", "User already exists.");
        return r;
    }
#endif

    if (!write_file_atomic(path, rec, rec_len)) {
        OPENSSL_cleanse(rec, rec_len);
        free(rec);
        snprintf(r.message, sizeof(r.message), "%s", "Failed to write credentials.");
        return r;
    }
    OPENSSL_cleanse(rec, rec_len);
    free(rec);

    r.success = 1;
    snprintf(r.message, sizeof(r.message), "%s", "User registered.");
    return r;
}

static Response server_authenticate(const char* base_dir, const char* username, const char* password) {
    Response r = {0, {0}};
    if (!is_valid_username(username)) {
        snprintf(r.message, sizeof(r.message), "%s", "Invalid username.");
        return r;
    }
    char path[512];
    if (!build_user_path(path, sizeof(path), base_dir, username)) {
        snprintf(r.message, sizeof(r.message), "%s", "Path error.");
        return r;
    }
    unsigned char* rec = NULL;
    size_t rec_len = 0;
    if (!read_file_secure(path, &rec, &rec_len)) {
        snprintf(r.message, sizeof(r.message), "%s", "User not found.");
        return r;
    }
    unsigned char salt[SALT_LEN];
    unsigned char stored[KEY_LEN];
    if (!parse_record(rec, rec_len, salt, stored)) {
        OPENSSL_cleanse(rec, rec_len);
        free(rec);
        snprintf(r.message, sizeof(r.message), "%s", "Corrupt credentials.");
        return r;
    }
    unsigned char cand[KEY_LEN];
    if (!derive_key_pbkdf2(password, salt, sizeof(salt), cand, sizeof(cand))) {
        OPENSSL_cleanse(rec, rec_len);
        free(rec);
        snprintf(r.message, sizeof(r.message), "%s", "KDF failure.");
        return r;
    }
    int ok = const_time_eq(cand, stored, KEY_LEN);
    OPENSSL_cleanse(cand, sizeof(cand));
    OPENSSL_cleanse(rec, rec_len);
    free(rec);
    if (!ok) {
        snprintf(r.message, sizeof(r.message), "%s", "Authentication failed.");
        return r;
    }
    r.success = 1;
    snprintf(r.message, sizeof(r.message), "%s", "Authentication successful.");
    return r;
}

static Response server_process_request(const char* base_dir, const LoginRequest* req) {
    Response r = {0, {0}};
    if (!req) {
        snprintf(r.message, sizeof(r.message), "%s", "Invalid request.");
        return r;
    }
    return server_authenticate(base_dir, req->username, req->password);
}

static LoginRequest client_build_login_request(const char* username, const char* password) {
    LoginRequest lr;
    memset(&lr, 0, sizeof(lr));
    if (username) {
        strncpy(lr.username, username, USER_MAX);
        lr.username[USER_MAX] = '\0';
    }
    if (password) {
        strncpy(lr.password, password, PASS_MAX);
        lr.password[PASS_MAX] = '\0';
    }
    return lr;
}

// Optional prompt (not used in tests)
static int client_prompt_and_build_request(LoginRequest* out) {
    if (!out) return 0;
    char u[USER_MAX + 2] = {0};
    char p[PASS_MAX + 2] = {0};
    printf("Enter username: ");
    if (!fgets(u, sizeof(u), stdin)) return 0;
    printf("Enter password: ");
    if (!fgets(p, sizeof(p), stdin)) return 0;
    // strip newline
    char* nl = strchr(u, '\n'); if (nl) *nl = '\0';
    nl = strchr(p, '\n'); if (nl) *nl = '\0';
    memset(out, 0, sizeof(*out));
    strncpy(out->username, u, USER_MAX);
    strncpy(out->password, p, PASS_MAX);
    secure_clean_str(p);
    secure_clean_str(u);
    return 1;
}

int main(void) {
    const char* base = "secure_user_store_c";
    // Clean start best-effort
#if !defined(_WIN32)
    // Remove directory recursively (simple best-effort; ignoring errors)
    DIR* d = opendir(base);
    if (d) {
        struct dirent* de;
        char path[512];
        while ((de = readdir(d)) != NULL) {
            if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;
            snprintf(path, sizeof(path), "%s/%s", base, de->d_name);
            unlink(path);
        }
        closedir(d);
        rmdir(base);
    }
#else
    // Windows: best-effort remove files if exist
    _rmdir(base); // will fail if not empty
#endif

    ensure_dir_secure(base);

    Response r1 = server_register_user(base, "alice", "Password!234");
    Response r2 = server_register_user(base, "bob",   "B0b_S3cret$");
    Response r3 = server_register_user(base, "alice", "Another$Pass1"); // duplicate
    printf("[Test 1] Register alice: %s - %s\n", r1.success ? "OK" : "FAIL", r1.message);
    printf("[Test 2] Register bob:   %s - %s\n", r2.success ? "OK" : "FAIL", r2.message);
    printf("[Test 3] Register alice again: %s - %s\n", r3.success ? "OK" : "FAIL", r3.message);

    LoginRequest q4 = client_build_login_request("alice", "Password!234");
    Response a4 = server_process_request(base, &q4);
    secure_clean_str(q4.password);
    printf("[Test 4] Login alice correct: %s - %s\n", a4.success ? "OK" : "FAIL", a4.message);

    LoginRequest q5 = client_build_login_request("alice", "WrongPass#1");
    Response a5 = server_process_request(base, &q5);
    secure_clean_str(q5.password);
    printf("[Test 5] Login alice wrong: %s - %s\n", a5.success ? "OK" : "FAIL", a5.message);

    LoginRequest q6 = client_build_login_request("charlie", "Some$Pass123");
    Response a6 = server_process_request(base, &q6);
    secure_clean_str(q6.password);
    printf("[Test 6] Login unknown user charlie: %s - %s\n", a6.success ? "OK" : "FAIL", a6.message);

    LoginRequest q7 = client_build_login_request("eve/../../etc", "XyZ!234567");
    Response a7 = server_process_request(base, &q7);
    secure_clean_str(q7.password);
    printf("[Test 7] Login invalid username: %s - %s\n", a7.success ? "OK" : "FAIL", a7.message);

    Response r8 = server_register_user(base, "mallory", "weakpass");
    printf("[Test 8] Register weak password: %s - %s\n", r8.success ? "OK" : "FAIL", r8.message);

    return 0;
}