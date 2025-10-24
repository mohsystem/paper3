#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>
#include <limits>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <climits>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
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
    return (r != nullptr);
}

static bool join_path(const char* a, const char* b, char out[PATH_MAX]) {
    if (!a || !b) return false;
    size_t la = strlen(a), lb = strlen(b);
    if (lb > 0 && b[0] == '/') return false; // reject absolute relpath
    if (la + 1 + lb + 1 > PATH_MAX) return false;
    std::string s(a);
    if (!s.empty() && s.back() != '/') s.push_back('/');
    s.append(b);
    if (s.size() + 1 > PATH_MAX) return false;
    strncpy(out, s.c_str(), PATH_MAX);
    out[PATH_MAX - 1] = '\0';
    return true;
}

static bool get_parent_dir(const char* path, char parent[PATH_MAX], char fname[NAME_MAX + 1]) {
    const char* last = strrchr(path, '/');
    if (!last) {
        // current dir
        if (snprintf(parent, PATH_MAX, ".") < 0) return false;
        if (strlen(path) > NAME_MAX) return false;
        strncpy(fname, path, NAME_MAX);
        fname[NAME_MAX] = '\0';
        return true;
    } else {
        size_t dlen = (size_t)(last - path);
        if (dlen == 0) dlen = 1; // root
        if (dlen >= PATH_MAX) return false;
        memcpy(parent, path, dlen);
        parent[dlen] = '\0';
        const char* base = last + 1;
        if (strlen(base) == 0) return false;
        if (strlen(base) > NAME_MAX) return false;
        strncpy(fname, base, NAME_MAX);
        fname[NAME_MAX] = '\0';
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
    // reject if leaf is symlink
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

    // Compose final path as parent_real + "/" + fname
    if (snprintf(out_final, PATH_MAX, "%s/%s", parent_real, fname) >= (int)PATH_MAX) return false;
    strncpy(out_parent, parent_real, PATH_MAX);
    out_parent[PATH_MAX - 1] = '\0';

    // If target exists and is symlink, reject
    struct stat st;
    if (lstat(out_final, &st) == 0 && S_ISLNK(st.st_mode)) return false;

    return true;
}

static bool read_file_fd_all(int fd, std::vector<unsigned char>& out) {
    out.clear();
    const size_t CHUNK = 8192;
    unsigned char buf[CHUNK];
    for (;;) {
        ssize_t r = read(fd, buf, CHUNK);
        if (r < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        if (r == 0) break;
        out.insert(out.end(), buf, buf + (size_t)r);
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
                            std::vector<unsigned char>& ciphertext,
                            unsigned char tag[TAG_LEN]) {
    ciphertext.assign(pt_len, 0);
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return false;
    bool ok = false;
    int outlen = 0;
    do {
        if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) break;
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, (int)IV_LEN, nullptr) != 1) break;
        if (EVP_EncryptInit_ex(ctx, nullptr, nullptr, key, iv) != 1) break;

        int len = 0;
        if (pt_len > 0) {
            if (EVP_EncryptUpdate(ctx, ciphertext.data(), &len, plaintext, (int)pt_len) != 1) break;
            outlen = len;
        }
        if (EVP_EncryptFinal_ex(ctx, ciphertext.data() + outlen, &len) != 1) break;
        outlen += len;
        ciphertext.resize((size_t)outlen);
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_LEN, tag) != 1) break;
        ok = true;
    } while (false);
    EVP_CIPHER_CTX_free(ctx);
    return ok;
}

static bool aes_gcm_decrypt(const unsigned char* key, const unsigned char* iv,
                            const unsigned char* ciphertext, size_t ct_len,
                            const unsigned char tag[TAG_LEN],
                            std::vector<unsigned char>& plaintext) {
    plaintext.assign(ct_len, 0);
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return false;
    bool ok = false;
    int outlen = 0;
    do {
        if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1) break;
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, (int)IV_LEN, nullptr) != 1) break;
        if (EVP_DecryptInit_ex(ctx, nullptr, nullptr, key, iv) != 1) break;

        int len = 0;
        if (ct_len > 0) {
            if (EVP_DecryptUpdate(ctx, plaintext.data(), &len, ciphertext, (int)ct_len) != 1) break;
            outlen = len;
        }
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_LEN, (void*)tag) != 1) break;
        if (EVP_DecryptFinal_ex(ctx, plaintext.data() + outlen, &len) != 1) {
            // Auth failed
            break;
        }
        outlen += len;
        plaintext.resize((size_t)outlen);
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

    // Prevent in-place overwrite
    if (strcmp(in_abs, out_abs) == 0) return -4;

    int infd = open(in_abs, O_RDONLY
#ifdef O_NOFOLLOW
                    | O_NOFOLLOW
#endif
    );
    if (infd < 0) return -5;

    std::vector<unsigned char> plaintext;
    bool ok = read_file_fd_all(infd, plaintext);
    close(infd);
    if (!ok) return -6;

    unsigned char salt[SALT_LEN];
    unsigned char iv[IV_LEN];
    if (RAND_bytes(salt, (int)SALT_LEN) != 1) return -7;
    if (RAND_bytes(iv, (int)IV_LEN) != 1) return -8;

    unsigned char key[32];
    if (!derive_key(passphrase, salt, key, sizeof(key))) {
        OPENSSL_cleanse(key, sizeof(key));
        return -9;
    }

    std::vector<unsigned char> ciphertext;
    unsigned char tag[TAG_LEN];
    if (!aes_gcm_encrypt(key, iv, plaintext.data(), plaintext.size(), ciphertext, tag)) {
        OPENSSL_cleanse(key, sizeof(key));
        return -10;
    }
    OPENSSL_cleanse(key, sizeof(key));

    // Prepare temp file in same dir
    char tmpl[PATH_MAX];
    if (snprintf(tmpl, PATH_MAX, "%s/.enc.tmp.XXXXXX", out_parent) >= (int)PATH_MAX) return -11;
    int tmpfd = mkstemp(tmpl);
    if (tmpfd < 0) return -12;
    // Restrict permissions
    fchmod(tmpfd, S_IRUSR | S_IWUSR);

    bool write_ok = true;
    write_ok &= write_all_fd(tmpfd, (const unsigned char*)MAGIC, sizeof(MAGIC));
    write_ok &= write_all_fd(tmpfd, &VERSION, 1);
    write_ok &= write_all_fd(tmpfd, salt, SALT_LEN);
    write_ok &= write_all_fd(tmpfd, iv, IV_LEN);
    if (!ciphertext.empty()) {
        write_ok &= write_all_fd(tmpfd, ciphertext.data(), ciphertext.size());
    }
    write_ok &= write_all_fd(tmpfd, tag, TAG_LEN);

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

    // Prevent in-place overwrite
    if (strcmp(in_abs, out_abs) == 0) return -4;

    int infd = open(in_abs, O_RDONLY
#ifdef O_NOFOLLOW
                    | O_NOFOLLOW
#endif
    );
    if (infd < 0) return -5;

    std::vector<unsigned char> filedata;
    bool ok = read_file_fd_all(infd, filedata);
    close(infd);
    if (!ok) return -6;

    if (filedata.size() < sizeof(MAGIC) + 1 + SALT_LEN + IV_LEN + TAG_LEN) return -7;
    size_t off = 0;
    if (memcmp(filedata.data() + off, MAGIC, sizeof(MAGIC)) != 0) return -8;
    off += sizeof(MAGIC);
    unsigned char version = filedata[off++];
    if (version != VERSION) return -9;
    const unsigned char* salt = filedata.data() + off; off += SALT_LEN;
    const unsigned char* iv = filedata.data() + off; off += IV_LEN;
    if (filedata.size() < off + TAG_LEN) return -10;
    size_t ct_len = filedata.size() - off - TAG_LEN;
    const unsigned char* ct = filedata.data() + off; off += ct_len;
    const unsigned char* tag = filedata.data() + off;

    unsigned char key[32];
    if (!derive_key(passphrase, salt, key, sizeof(key))) {
        OPENSSL_cleanse(key, sizeof(key));
        return -11;
    }

    std::vector<unsigned char> plaintext;
    if (!aes_gcm_decrypt(key, iv, ct, ct_len, tag, plaintext)) {
        OPENSSL_cleanse(key, sizeof(key));
        return -12;
    }
    OPENSSL_cleanse(key, sizeof(key));

    // Prepare temp file in same dir
    char tmpl[PATH_MAX];
    if (snprintf(tmpl, PATH_MAX, "%s/.dec.tmp.XXXXXX", out_parent) >= (int)PATH_MAX) return -13;
    int tmpfd = mkstemp(tmpl);
    if (tmpfd < 0) return -14;
    fchmod(tmpfd, S_IRUSR | S_IWUSR);

    bool write_ok = true;
    if (!plaintext.empty()) {
        write_ok &= write_all_fd(tmpfd, plaintext.data(), plaintext.size());
    }
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

static bool read_text_file(const char* path, std::string& out) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) return false;
    std::vector<unsigned char> buf;
    bool ok = read_file_fd_all(fd, buf);
    close(fd);
    if (!ok) return false;
    out.assign((const char*)buf.data(), buf.size());
    return true;
}

int main() {
    const char* base = "safe_base_cpp";
    mkdir(base, 0700);

    // Prepare test input files
    write_text_file("safe_base_cpp/file1.txt", "Hello, World!");
    write_text_file("safe_base_cpp/file2.txt", "The quick brown fox jumps over the lazy dog.");
    write_text_file("safe_base_cpp/file3.txt", "1234567890");
    write_text_file("safe_base_cpp/file4.txt", "Sample data with multiple lines\nLine2\nLine3");
    write_text_file("safe_base_cpp/file5.txt", "");

    const char* pass = "strong passphrase";

    // Test 1: basic encrypt/decrypt
    int rc1 = encrypt_file(base, "file1.txt", "file1.enc", pass);
    int rc2 = decrypt_file(base, "file1.enc", "file1.dec.txt", pass);
    std::string r1, r1d;
    read_text_file("safe_base_cpp/file1.txt", r1);
    read_text_file("safe_base_cpp/file1.dec.txt", r1d);
    std::cout << "T1 enc=" << rc1 << " dec=" << rc2 << " eq=" << (r1 == r1d) << "\n";

    // Test 2: longer text
    int rc3 = encrypt_file(base, "file2.txt", "nested/../file2.enc", pass); // normalized inside base
    std::cout << "T2 enc=" << rc3 << "\n";
    int rc4 = decrypt_file(base, "file2.enc", "file2.out.txt", pass);
    std::string r2, r2d;
    read_text_file("safe_base_cpp/file2.txt", r2);
    read_text_file("safe_base_cpp/file2.out.txt", r2d);
    std::cout << "T2 dec=" << rc4 << " eq=" << (r2 == r2d) << "\n";

    // Test 3: empty file
    int rc5 = encrypt_file(base, "file5.txt", "file5.enc", pass);
    int rc6 = decrypt_file(base, "file5.enc", "file5.out.txt", pass);
    std::string r5, r5d;
    read_text_file("safe_base_cpp/file5.txt", r5);
    read_text_file("safe_base_cpp/file5.out.txt", r5d);
    std::cout << "T3 enc=" << rc5 << " dec=" << rc6 << " eq=" << (r5 == r5d) << "\n";

    // Test 4: wrong passphrase should fail decrypt
    int rc7 = encrypt_file(base, "file3.txt", "file3.enc", pass);
    int rc8 = decrypt_file(base, "file3.enc", "file3.bad.txt", "wrong pass");
    std::cout << "T4 enc=" << rc7 << " dec_fail=" << rc8 << "\n";

    // Test 5: path traversal should be rejected
    int rc9 = encrypt_file(base, "../outside.txt", "bad.enc", pass);
    int rc10 = decrypt_file(base, "../outside.enc", "bad.txt", pass);
    std::cout << "T5 enc_trav=" << rc9 << " dec_trav=" << rc10 << "\n";

    return 0;
}