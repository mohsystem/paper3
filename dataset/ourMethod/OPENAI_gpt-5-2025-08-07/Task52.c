#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif
#ifndef NAME_MAX
#define NAME_MAX 255
#endif

static const char MAGIC[4] = {'E','N','C','1'};
static const unsigned char VERSION = 1;
static const size_t SALT_LEN = 16;
static const size_t IV_LEN = 12;
static const size_t TAG_LEN = 16;
static const int PBKDF2_ITERS = 210000;

static bool starts_with_prefix(const char* path, const char* base) {
    size_t blen = strlen(base);
    if (strncmp(path, base, blen) != 0) return false;
    if (path[blen] == '\0') return true;
    if (base[blen - 1] == '/') return true;
    return path[blen] == '/';
}

static bool is_symlink(const char* p) {
    struct stat st;
    if (lstat(p, &st) != 0) return false;
    return S_ISLNK(st.st_mode);
}

static bool is_regular_file(const char* p) {
    struct stat st;
    if (lstat(p, &st) != 0) return false;
    return S_ISREG(st.st_mode);
}

static bool is_directory(const char* p) {
    struct stat st;
    if (lstat(p, &st) != 0) return false;
    return S_ISDIR(st.st_mode);
}

static bool get_canonical(const char* input, char out[PATH_MAX]) {
    char* r = realpath(input, out);
    return (r != NULL);
}

static bool join_path(const char* a, const char* b, char out[PATH_MAX]) {
    if (!a || !b) return false;
    size_t la = strlen(a), lb = strlen(b);
    if (lb > 0 && b[0] == '/') return false; // reject absolute
    if (la + 1 + lb + 1 > PATH_MAX) return false;
    int n = snprintf(out, PATH_MAX, "%s%s%s", a, (la > 0 && a[la - 1] == '/') ? "" : "/", b);
    if (n < 0 || n >= PATH_MAX) return false;
    return true;
}

static bool get_parent_dir(const char* path, char parent[PATH_MAX], char fname[NAME_MAX + 1]) {
    const char* last = strrchr(path, '/');
    if (!last) {
        if (snprintf(parent, PATH_MAX, ".") < 0) return false;
        size_t flen = strlen(path);
        if (flen == 0 || flen > NAME_MAX) return false;
        memcpy(fname, path, flen);
        fname[flen] = '\0';
        return true;
    } else {
        size_t dlen = (size_t)(last - path);
        if (dlen == 0) dlen = 1;
        if (dlen >= PATH_MAX) return false;
        memcpy(parent, path, dlen);
        parent[dlen] = '\0';
        const char* base = last + 1;
        size_t blen = strlen(base);
        if (blen == 0 || blen > NAME_MAX) return false;
        memcpy(fname, base, blen);
        fname[blen] = '\0';
        return true;
    }
}

static bool validate_input_path(const char* base_dir, const char* rel, char out_abs[PATH_MAX]) {
    if (!base_dir || !rel) return false;
    char base_real[PATH_MAX];
    if (!get_canonical(base_dir, base_real)) return false;
    if (is_symlink(base_real)) return false;
    char joined[PATH_MAX];
    if (!join_path(base_real, rel, joined)) return false;
    if (is_symlink(joined)) return false;
    char realp[PATH_MAX];
    if (!get_canonical(joined, realp)) return false;
    if (!starts_with_prefix(realp, base_real)) return false;
    if (!is_regular_file(realp)) return false;
    strncpy(out_abs, realp, PATH_MAX);
    out_abs[PATH_MAX - 1] = '\0';
    return true;
}

static bool validate_output_path(const char* base_dir, const char* rel, char out_final[PATH_MAX], char out_parent[PATH_MAX]) {
    if (!base_dir || !rel) return false;
    char base_real[PATH_MAX];
    if (!get_canonical(base_dir, base_real)) return false;
    if (is_symlink(base_real)) return false;
    char joined[PATH_MAX];
    if (!join_path(base_real, rel, joined)) return false;

    char parent[PATH_MAX];
    char fname[NAME_MAX + 1];
    if (!get_parent_dir(joined, parent, fname)) return false;
    char parent_real[PATH_MAX];
    if (!get_canonical(parent, parent_real)) return false;
    if (!starts_with_prefix(parent_real, base_real)) return false;
    if (is_symlink(parent_real)) return false;
    if (!is_directory(parent_real)) return false;

    if (snprintf(out_final, PATH_MAX, "%s/%s", parent_real, fname) >= PATH_MAX) return false;
    strncpy(out_parent, parent_real, PATH_MAX);
    out_parent[PATH_MAX - 1] = '\0';

    struct stat st;
    if (lstat(out_final, &st) == 0 && S_ISLNK(st.st_mode)) return false;
    return true;
}

static bool read_file_fd_all(int fd, unsigned char** out_buf, size_t* out_len) {
    *out_buf = NULL;
    *out_len = 0;
    size_t cap = 0;
    const size_t CHUNK = 8192;
    for (;;) {
        if (*out_len + CHUNK > cap) {
            size_t newcap = cap == 0 ? 16384 : cap * 2;
            unsigned char* nb = (unsigned char*)realloc(*out_buf, newcap);
            if (!nb) {
                free(*out_buf);
                *out_buf = NULL;
                *out_len = 0;
                return false;
            }
            *out_buf = nb;
            cap = newcap;
        }
        ssize_t r = read(fd, *out_buf + *out_len, CHUNK);
        if (r < 0) {
            if (errno == EINTR) continue;
            free(*out_buf);
            *out_buf = NULL;
            *out_len = 0;
            return false;
        }
        if (r == 0) break;
        *out_len += (size_t)r;
    }
    return true;
}

static bool write_all_fd(int fd, const unsigned char* data, size_t len) {
    size_t off = 0;
    while (off < len) {
        ssize_t w = write(fd, data + off, len - off);
        if (w < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        off += (size_t)w;
    }
    return true;
}

static bool derive_key(const char* pass, const unsigned char* salt, unsigned char* key, size_t key_len) {
    if (!pass || !salt || !key) return false;
    int ok = PKCS5_PBKDF2_HMAC(pass, (int)strlen(pass), salt, (int)SALT_LEN, PBKDF2_ITERS, EVP_sha256(), (int)key_len, key);
    return ok == 1;
}

static bool aes_gcm_encrypt(const unsigned char* key, const unsigned char* iv,
                            const unsigned char* plaintext, size_t pt_len,
                            unsigned char** out_ct, size_t* out_ct_len,
                            unsigned char tag[TAG_LEN]) {
    *out_ct = NULL;
    *out_ct_len = 0;
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return false;
    bool ok = false;
    do {
        if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) break;
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, (int)IV_LEN, NULL) != 1) break;
        if (EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv) != 1) break;

        unsigned char* ct = NULL;
        int outlen1 = 0, outlen2 = 0;
        if (pt_len > 0) {
            ct = (unsigned char*)malloc(pt_len);
            if (!ct) break;
            if (EVP_EncryptUpdate(ctx, ct, &outlen1, plaintext, (int)pt_len) != 1) { free(ct); break; }
        } else {
            ct = (unsigned char*)malloc(1);
            if (!ct) break;
        }
        if (EVP_EncryptFinal_ex(ctx, ct + outlen1, &outlen2) != 1) { free(ct); break; }
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_LEN, tag) != 1) { free(ct); break; }

        *out_ct_len = (size_t)(outlen1 + outlen2);
        *out_ct = ct;
        ok = true;
    } while (false);
    EVP_CIPHER_CTX_free(ctx);
    return ok;
}

static bool aes_gcm_decrypt(const unsigned char* key, const unsigned char* iv,
                            const unsigned char* ciphertext, size_t ct_len,
                            const unsigned char tag[TAG_LEN],
                            unsigned char** out_pt, size_t* out_pt_len) {
    *out_pt = NULL;
    *out_pt_len = 0;
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return false;
    bool ok = false;
    do {
        if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) != 1) break;
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, (int)IV_LEN, NULL) != 1) break;
        if (EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv) != 1) break;

        unsigned char* pt = NULL;
        int outlen1 = 0, outlen2 = 0;
        if (ct_len > 0) {
            pt = (unsigned char*)malloc(ct_len);
            if (!pt) break;
            if (EVP_DecryptUpdate(ctx, pt, &outlen1, ciphertext, (int)ct_len) != 1) { free(pt); break; }
        } else {
            pt = (unsigned char*)malloc(1);
            if (!pt) break;
        }
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_LEN, (void*)tag) != 1) { free(pt); break; }
        if (EVP_DecryptFinal_ex(ctx, pt + outlen1, &outlen2) != 1) { free(pt); break; }

        *out_pt_len = (size_t)(outlen1 + outlen2);
        *out_pt = pt;
        ok = true;
    } while (false);
    EVP_CIPHER_CTX_free(ctx);
    return ok;
}

static int encrypt_file(const char* base_dir, const char* input_rel, const char* output_rel, const char* passphrase) {
    if (!base_dir || !input_rel || !output_rel || !passphrase) return -1;
    char in_abs[PATH_MAX];
    if (!validate_input_path(base_dir, input_rel, in_abs)) return -2;

    char out_abs[PATH_MAX], out_parent[PATH_MAX];
    if (!validate_output_path(base_dir, output_rel, out_abs, out_parent)) return -3;

    if (strcmp(in_abs, out_abs) == 0) return -4;

    int infd = open(in_abs, O_RDONLY
#ifdef O_NOFOLLOW
                    | O_NOFOLLOW
#endif
    );
    if (infd < 0) return -5;

    unsigned char* plaintext = NULL;
    size_t pt_len = 0;
    bool ok = read_file_fd_all(infd, &plaintext, &pt_len);
    close(infd);
    if (!ok) return -6;

    unsigned char salt[SALT_LEN];
    unsigned char iv[IV_LEN];
    if (RAND_bytes(salt, (int)SALT_LEN) != 1) { free(plaintext); return -7; }
    if (RAND_bytes(iv, (int)IV_LEN) != 1) { free(plaintext); return -8; }

    unsigned char key[32];
    if (!derive_key(passphrase, salt, key, sizeof(key))) {
        OPENSSL_cleanse(key, sizeof(key));
        free(plaintext);
        return -9;
    }

    unsigned char* ciphertext = NULL;
    size_t ct_len = 0;
    unsigned char tag[TAG_LEN];
    if (!aes_gcm_encrypt(key, iv, plaintext, pt_len, &ciphertext, &ct_len, tag)) {
        OPENSSL_cleanse(key, sizeof(key));
        free(plaintext);
        return -10;
    }
    OPENSSL_cleanse(key, sizeof(key));
    free(plaintext);

    char tmpl[PATH_MAX];
    if (snprintf(tmpl, PATH_MAX, "%s/.enc.tmp.XXXXXX", out_parent) >= PATH_MAX) { free(ciphertext); return -11; }
    int tmpfd = mkstemp(tmpl);
    if (tmpfd < 0) { free(ciphertext); return -12; }
    fchmod(tmpfd, S_IRUSR | S_IWUSR);

    bool write_ok = true;
    write_ok &= write_all_fd(tmpfd, (const unsigned char*)MAGIC, sizeof(MAGIC));
    write_ok &= write_all_fd(tmpfd, &VERSION, 1);
    write_ok &= write_all_fd(tmpfd, salt, SALT_LEN);
    write_ok &= write_all_fd(tmpfd, iv, IV_LEN);
    if (ct_len > 0) write_ok &= write_all_fd(tmpfd, ciphertext, ct_len);
    write_ok &= write_all_fd(tmpfd, tag, TAG_LEN);

    free(ciphertext);

    if (!write_ok) {
        close(tmpfd);
        unlink(tmpl);
        return -13;
    }
    if (fsync(tmpfd) != 0) {
        close(tmpfd);
        unlink(tmpl);
        return -14;
    }
    if (close(tmpfd) != 0) {
        unlink(tmpl);
        return -15;
    }
    if (rename(tmpl, out_abs) != 0) {
        unlink(tmpl);
        return -16;
    }
    return 0;
}

static int decrypt_file(const char* base_dir, const char* input_rel, const char* output_rel, const char* passphrase) {
    if (!base_dir || !input_rel || !output_rel || !passphrase) return -1;
    char in_abs[PATH_MAX];
    if (!validate_input_path(base_dir, input_rel, in_abs)) return -2;

    char out_abs[PATH_MAX], out_parent[PATH_MAX];
    if (!validate_output_path(base_dir, output_rel, out_abs, out_parent)) return -3;

    if (strcmp(in_abs, out_abs) == 0) return -4;

    int infd = open(in_abs, O_RDONLY
#ifdef O_NOFOLLOW
                    | O_NOFOLLOW
#endif
    );
    if (infd < 0) return -5;

    unsigned char* filedata = NULL;
    size_t flen = 0;
    bool ok = read_file_fd_all(infd, &filedata, &flen);
    close(infd);
    if (!ok) return -6;

    if (flen < sizeof(MAGIC) + 1 + SALT_LEN + IV_LEN + TAG_LEN) { free(filedata); return -7; }
    size_t off = 0;
    if (memcmp(filedata + off, MAGIC, sizeof(MAGIC)) != 0) { free(filedata); return -8; }
    off += sizeof(MAGIC);
    unsigned char version = filedata[off++];
    if (version != VERSION) { free(filedata); return -9; }
    const unsigned char* salt = filedata + off; off += SALT_LEN;
    const unsigned char* iv = filedata + off; off += IV_LEN;
    if (flen < off + TAG_LEN) { free(filedata); return -10; }
    size_t ct_len = flen - off - TAG_LEN;
    const unsigned char* ct = filedata + off; off += ct_len;
    const unsigned char* tag = filedata + off;

    unsigned char key[32];
    if (!derive_key(passphrase, salt, key, sizeof(key))) {
        OPENSSL_cleanse(key, sizeof(key));
        free(filedata);
        return -11;
    }

    unsigned char* plaintext = NULL;
    size_t pt_len = 0;
    if (!aes_gcm_decrypt(key, iv, ct, ct_len, tag, &plaintext, &pt_len)) {
        OPENSSL_cleanse(key, sizeof(key));
        free(filedata);
        return -12;
    }
    OPENSSL_cleanse(key, sizeof(key));
    free(filedata);

    char tmpl[PATH_MAX];
    if (snprintf(tmpl, PATH_MAX, "%s/.dec.tmp.XXXXXX", out_parent) >= PATH_MAX) { free(plaintext); return -13; }
    int tmpfd = mkstemp(tmpl);
    if (tmpfd < 0) { free(plaintext); return -14; }
    fchmod(tmpfd, S_IRUSR | S_IWUSR);

    bool write_ok = true;
    if (pt_len > 0) write_ok &= write_all_fd(tmpfd, plaintext, pt_len);
    free(plaintext);

    if (!write_ok) {
        close(tmpfd);
        unlink(tmpl);
        return -15;
    }
    if (fsync(tmpfd) != 0) {
        close(tmpfd);
        unlink(tmpl);
        return -16;
    }
    if (close(tmpfd) != 0) {
        unlink(tmpl);
        return -17;
    }
    if (rename(tmpl, out_abs) != 0) {
        unlink(tmpl);
        return -18;
    }
    return 0;
}

static bool write_text_file(const char* path, const char* text) {
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd < 0) return false;
    size_t len = strlen(text);
    bool ok = write_all_fd(fd, (const unsigned char*)text, len);
    fsync(fd);
    close(fd);
    return ok;
}

static bool read_text_file(const char* path, char** out, size_t* out_len) {
    *out = NULL;
    *out_len = 0;
    int fd = open(path, O_RDONLY);
    if (fd < 0) return false;
    unsigned char* buf = NULL;
    size_t blen = 0;
    bool ok = read_file_fd_all(fd, &buf, &blen);
    close(fd);
    if (!ok) return false;
    char* s = (char*)malloc(blen + 1);
    if (!s) { free(buf); return false; }
    memcpy(s, buf, blen);
    s[blen] = '\0';
    free(buf);
    *out = s;
    *out_len = blen;
    return true;
}

int main(void) {
    const char* base = "safe_base_c";
    mkdir(base, 0700);

    write_text_file("safe_base_c/a.txt", "Alpha");
    write_text_file("safe_base_c/b.txt", "Bravo Charlie Delta");
    write_text_file("safe_base_c/c.txt", "1234567890");
    write_text_file("safe_base_c/d.txt", "Line1\nLine2\nLine3\n");
    write_text_file("safe_base_c/e.txt", "");

    const char* pass = "very strong passphrase";

    int rc1 = encrypt_file(base, "a.txt", "a.enc", pass);
    int rc2 = decrypt_file(base, "a.enc", "a.dec.txt", pass);
    char *t1 = NULL, *t1d = NULL; size_t l1=0,l1d=0;
    read_text_file("safe_base_c/a.txt", &t1, &l1);
    read_text_file("safe_base_c/a.dec.txt", &t1d, &l1d);
    printf("T1 enc=%d dec=%d eq=%d\n", rc1, rc2, (t1 && t1d && strcmp(t1, t1d) == 0));
    free(t1); free(t1d);

    int rc3 = encrypt_file(base, "b.txt", "b.enc", pass);
    int rc4 = decrypt_file(base, "b.enc", "b.out.txt", pass);
    char *t2=NULL,*t2d=NULL; size_t l2=0,l2d=0;
    read_text_file("safe_base_c/b.txt", &t2, &l2);
    read_text_file("safe_base_c/b.out.txt", &t2d, &l2d);
    printf("T2 enc=%d dec=%d eq=%d\n", rc3, rc4, (t2 && t2d && strcmp(t2, t2d) == 0));
    free(t2); free(t2d);

    int rc5 = encrypt_file(base, "e.txt", "e.enc", pass);
    int rc6 = decrypt_file(base, "e.enc", "e.out.txt", pass);
    char *t5=NULL,*t5d=NULL; size_t l5=0,l5d=0;
    read_text_file("safe_base_c/e.txt", &t5, &l5);
    read_text_file("safe_base_c/e.out.txt", &t5d, &l5d);
    printf("T3 enc=%d dec=%d eq=%d\n", rc5, rc6, (t5 && t5d && strcmp(t5, t5d) == 0));
    free(t5); free(t5d);

    int rc7 = encrypt_file(base, "c.txt", "c.enc", pass);
    int rc8 = decrypt_file(base, "c.enc", "c.bad.txt", "wrong pass");
    printf("T4 enc=%d dec_fail=%d\n", rc7, rc8);

    int rc9 = encrypt_file(base, "../evil.txt", "bad.enc", pass);
    int rc10 = decrypt_file(base, "../evil.enc", "bad.txt", pass);
    printf("T5 enc_trav=%d dec_trav=%d\n", rc9, rc10);

    return 0;
}