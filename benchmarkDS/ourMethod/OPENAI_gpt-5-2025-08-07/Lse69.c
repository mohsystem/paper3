/*
Secure image upload handler in C:
- Reads an uploaded image file from a trusted base directory using openat with O_NOFOLLOW.
- Validates filename strictly with an allow list and length limits.
- Enforces a maximum file size.
- Encodes the image bytes as Base64.
- Writes the encoded output to a new file safely with O_CREAT|O_EXCL and restrictive permissions.
- Includes a main function with 5 test cases.

Build:
  cc -Wall -Wextra -O2 -std=c11 upload_image.c -o upload_image

Run:
  ./upload_image
*/

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0
#endif

#define MAX_FILENAME_LEN 128        /* Restrictive, safer than 255 */
#define MAX_IMAGE_SIZE   (10u * 1024u * 1024u) /* 10 MiB maximum */
#define OUTPUT_EXT       ".b64"

/* Utility: safe strnlen with output parameter */
static bool safe_strnlen_range(const char *s, size_t max, size_t *out_len) {
    if (!s || !out_len) return false;
    size_t n = 0;
    while (n < max && s[n] != '\0') {
        n++;
    }
    if (n == max && s[n] != '\0') {
        return false; /* exceeded max or no terminator within limit */
    }
    *out_len = n;
    return true;
}

/* Validate filename: allow only [A-Za-z0-9._-], length 1..MAX_FILENAME_LEN, not "." or ".." */
static bool validate_filename(const char *name, size_t *len_out) {
    if (!name) return false;
    size_t n = 0;
    if (!safe_strnlen_range(name, MAX_FILENAME_LEN, &n)) return false;
    if (n == 0) return false;
    if (n == 1 && name[0] == '.') return false;
    if (n == 2 && name[0] == '.' && name[1] == '.') return false;
    for (size_t i = 0; i < n; i++) {
        unsigned char c = (unsigned char)name[i];
        bool ok = (c >= 'A' && c <= 'Z') ||
                  (c >= 'a' && c <= 'z') ||
                  (c >= '0' && c <= '9') ||
                  c == '.' || c == '_' || c == '-';
        if (!ok) return false;
    }
    if (len_out) *len_out = n;
    return true;
}

/* Open a trusted base directory safely */
static int open_trusted_dir(const char *base_dir) {
    if (!base_dir) return -1;
    int dfd = open(base_dir, O_RDONLY | O_DIRECTORY | O_CLOEXEC | O_NOFOLLOW);
    if (dfd < 0) return -1;
    struct stat st;
    if (fstat(dfd, &st) != 0) {
        close(dfd);
        return -1;
    }
    if (!S_ISDIR(st.st_mode)) {
        close(dfd);
        errno = ENOTDIR;
        return -1;
    }
    return dfd;
}

/* Read an entire file at dirfd/filename with max size enforcement */
static int read_file_at(int dirfd, const char *filename, uint8_t **out_buf, size_t *out_len, size_t max_len) {
    if (!out_buf || !out_len) { errno = EINVAL; return -1; }
    *out_buf = NULL; *out_len = 0;

    size_t flen = 0;
    if (!validate_filename(filename, &flen)) { errno = EINVAL; return -1; }

    int fd = openat(dirfd, filename, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (fd < 0) return -1;

    struct stat st;
    if (fstat(fd, &st) != 0) {
        int e = errno;
        close(fd);
        errno = e;
        return -1;
    }
    if (!S_ISREG(st.st_mode)) {
        close(fd);
        errno = EINVAL;
        return -1;
    }
    if (st.st_size < 0) {
        close(fd);
        errno = EIO;
        return -1;
    }
    if ((uint64_t)st.st_size > (uint64_t)max_len) {
        close(fd);
        errno = EFBIG;
        return -1;
    }
    size_t sz = (size_t)st.st_size;
    uint8_t *buf = NULL;
    if (sz > 0) {
        buf = (uint8_t *)malloc(sz);
        if (!buf) {
            close(fd);
            errno = ENOMEM;
            return -1;
        }
        size_t off = 0;
        while (off < sz) {
            ssize_t r = read(fd, buf + off, sz - off);
            if (r < 0) {
                if (errno == EINTR) continue;
                int e = errno;
                free(buf);
                close(fd);
                errno = e;
                return -1;
            }
            if (r == 0) break; /* EOF */
            off += (size_t)r;
        }
        if (off != sz) {
            free(buf);
            close(fd);
            errno = EIO;
            return -1;
        }
    }
    close(fd);
    *out_buf = buf;
    *out_len = sz;
    return 0;
}

/* Base64 encode */
static int base64_encode(const uint8_t *data, size_t len, char **out_str, size_t *out_len) {
    static const char b64[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    if (!out_str || !out_len) { errno = EINVAL; return -1; }
    *out_str = NULL; *out_len = 0;

    /* overflow-safe calculation: out_len = 4 * ((len + 2) / 3) */
    if (len > SIZE_MAX - 2) { errno = EOVERFLOW; return -1; }
    size_t t = (len + 2) / 3;
    if (t > SIZE_MAX / 4) { errno = EOVERFLOW; return -1; }
    size_t enc_len = 4 * t;

    char *out = NULL;
    if (enc_len > 0) {
        out = (char *)malloc(enc_len);
        if (!out) { errno = ENOMEM; return -1; }
    }

    size_t i = 0, o = 0;
    while (i + 3 <= len) {
        uint32_t v = ((uint32_t)data[i] << 16) | ((uint32_t)data[i + 1] << 8) | (uint32_t)data[i + 2];
        out[o++] = b64[(v >> 18) & 0x3F];
        out[o++] = b64[(v >> 12) & 0x3F];
        out[o++] = b64[(v >> 6) & 0x3F];
        out[o++] = b64[v & 0x3F];
        i += 3;
    }
    size_t rem = len - i;
    if (rem == 1) {
        uint32_t v = ((uint32_t)data[i] << 16);
        out[o++] = b64[(v >> 18) & 0x3F];
        out[o++] = b64[(v >> 12) & 0x3F];
        out[o++] = '=';
        out[o++] = '=';
    } else if (rem == 2) {
        uint32_t v = ((uint32_t)data[i] << 16) | ((uint32_t)data[i + 1] << 8);
        out[o++] = b64[(v >> 18) & 0x3F];
        out[o++] = b64[(v >> 12) & 0x3F];
        out[o++] = b64[(v >> 6) & 0x3F];
        out[o++] = '=';
    }
    /* In case len == 0, enc_len==0 and out==NULL; that's fine. */
    *out_str = out;
    *out_len = enc_len;
    return 0;
}

/* Compose output filename: either use provided validated name or input + OUTPUT_EXT */
static int build_output_name(const char *input_name, const char *provided_out, char out_name[MAX_FILENAME_LEN + 1]) {
    size_t in_len = 0, out_len = 0;
    if (!validate_filename(input_name, &in_len)) { errno = EINVAL; return -1; }

    if (provided_out && provided_out[0] != '\0') {
        if (!validate_filename(provided_out, &out_len)) { errno = EINVAL; return -1; }
        if (out_len > MAX_FILENAME_LEN) { errno = ENAMETOOLONG; return -1; }
        memcpy(out_name, provided_out, out_len);
        out_name[out_len] = '\0';
        return 0;
    }

    const char *ext = OUTPUT_EXT;
    size_t ext_len = strlen(ext);
    if (in_len > MAX_FILENAME_LEN - ext_len) {
        errno = ENAMETOOLONG;
        return -1;
    }
    memcpy(out_name, input_name, in_len);
    memcpy(out_name + in_len, ext, ext_len);
    out_name[in_len + ext_len] = '\0';
    return 0;
}

/* Write a file atomically without overwrite: open with O_CREAT|O_EXCL and write all bytes */
static int write_new_file_at(int dirfd, const char *filename, const void *buf, size_t len) {
    size_t flen = 0;
    if (!validate_filename(filename, &flen)) { errno = EINVAL; return -1; }

    int fd = openat(dirfd, filename, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW, 0600);
    if (fd < 0) return -1;

    const uint8_t *p = (const uint8_t *)buf;
    size_t off = 0;
    while (off < len) {
        ssize_t w = write(fd, p + off, len - off);
        if (w < 0) {
            if (errno == EINTR) continue;
            int e = errno;
            close(fd);
            errno = e;
            return -1;
        }
        off += (size_t)w;
    }

    /* Flush to disk */
    if (fsync(fd) != 0) {
        int e = errno;
        close(fd);
        errno = e;
        return -1;
    }
    if (close(fd) != 0) return -1;
    return 0;
}

/*
 upload_image:
  - base_dir: trusted base folder path
  - image_filename: validated relative file name inside base_dir
  - out_filename_opt: optional validated relative output name; if NULL, uses image_filename + ".b64"
  - max_size: maximum allowed image size in bytes
 Returns: 0 on success, -1 on error with errno set.
*/
int upload_image(const char *base_dir, const char *image_filename, const char *out_filename_opt, size_t max_size) {
    if (!base_dir || !image_filename) { errno = EINVAL; return -1; }
    if (max_size == 0 || max_size > MAX_IMAGE_SIZE) {
        /* Clamp to policy maximum */
        max_size = MAX_IMAGE_SIZE;
    }

    int dfd = open_trusted_dir(base_dir);
    if (dfd < 0) return -1;

    int rc = -1;
    uint8_t *img = NULL;
    size_t img_len = 0;
    char out_name[MAX_FILENAME_LEN + 1];

    if (build_output_name(image_filename, out_filename_opt, out_name) != 0) {
        goto cleanup;
    }

    if (read_file_at(dfd, image_filename, &img, &img_len, max_size) != 0) {
        goto cleanup;
    }

    char *b64 = NULL;
    size_t b64_len = 0;
    if (base64_encode(img, img_len, &b64, &b64_len) != 0) {
        goto cleanup_img;
    }

    if (write_new_file_at(dfd, out_name, b64, b64_len) != 0) {
        goto cleanup_b64;
    }

    rc = 0;

cleanup_b64:
    if (b64) {
        /* Not sensitive data; simple free is fine */
        free(b64);
    }
cleanup_img:
    if (img) {
        free(img);
    }
cleanup:
    close(dfd);
    return rc;
}

/* Helpers for test cases: create a test file inside directory */
static int create_test_file_at(int dirfd, const char *filename, const uint8_t *data, size_t len) {
    size_t flen = 0;
    if (!validate_filename(filename, &flen)) { errno = EINVAL; return -1; }
    int fd = openat(dirfd, filename, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC | O_NOFOLLOW, 0600);
    if (fd < 0) return -1;
    size_t off = 0;
    while (off < len) {
        ssize_t w = write(fd, data + off, len - off);
        if (w < 0) {
            if (errno == EINTR) continue;
            int e = errno;
            close(fd);
            errno = e;
            return -1;
        }
        off += (size_t)w;
    }
    if (fsync(fd) != 0) {
        int e = errno;
        close(fd);
        errno = e;
        return -1;
    }
    if (close(fd) != 0) return -1;
    return 0;
}

/* Minimal test output helper: prints constant messages only */
static void print_result(int test_no, int rc) {
    if (rc == 0) {
        printf("test %d: ok\n", test_no);
    } else {
        /* generic error message */
        printf("test %d: error\n", test_no);
    }
}

int main(void) {
    const char *base = "uploads";
    /* Ensure base directory exists with restrictive permissions */
    if (mkdir(base, 0700) != 0) {
        if (errno != EEXIST) {
            printf("setup failed\n");
            return 1;
        }
    }
    int dfd = open_trusted_dir(base);
    if (dfd < 0) {
        printf("setup failed\n");
        return 1;
    }

    /* Prepare 3 input files */
    /* Test 1 data: "abc" */
    const uint8_t t1[] = { 'a', 'b', 'c' };
    /* Test 2 data: empty */
    const uint8_t *t2 = NULL; size_t t2_len = 0;
    /* Test 3 data: binary bytes */
    const uint8_t t3[] = { 0x00, 0xFF, 0x10, 0x20, 0x7F };

    (void)unlinkat(dfd, "img1.bin", 0);
    (void)unlinkat(dfd, "img2.bin", 0);
    (void)unlinkat(dfd, "img3.bin", 0);
    (void)unlinkat(dfd, "img1_out.b64", 0);
    (void)unlinkat(dfd, "img2.b64", 0);
    (void)unlinkat(dfd, "img3_encoded.b64", 0);

    int rc;
    rc = create_test_file_at(dfd, "img1.bin", t1, sizeof(t1));
    if (rc != 0) { close(dfd); printf("setup failed\n"); return 1; }
    rc = create_test_file_at(dfd, "img2.bin", t2, t2_len);
    if (rc != 0) { close(dfd); printf("setup failed\n"); return 1; }
    rc = create_test_file_at(dfd, "img3.bin", t3, sizeof(t3));
    if (rc != 0) { close(dfd); printf("setup failed\n"); return 1; }

    close(dfd);

    /* Test 1: valid file with explicit output filename */
    rc = upload_image(base, "img1.bin", "img1_out.b64", MAX_IMAGE_SIZE);
    print_result(1, rc);

    /* Test 2: valid empty file using default output filename (input + .b64) */
    rc = upload_image(base, "img2.bin", NULL, MAX_IMAGE_SIZE);
    print_result(2, rc);

    /* Test 3: valid binary file with custom output filename */
    rc = upload_image(base, "img3.bin", "img3_encoded.b64", MAX_IMAGE_SIZE);
    print_result(3, rc);

    /* Test 4: invalid filename with traversal characters - should reject */
    rc = upload_image(base, "../secret.png", NULL, MAX_IMAGE_SIZE);
    print_result(4, rc);

    /* Test 5: overly long filename - should reject */
    char longname[MAX_FILENAME_LEN + 50];
    size_t pos = 0;
    for (; pos < MAX_FILENAME_LEN + 10; pos++) longname[pos] = 'a';
    longname[pos] = '\0';
    rc = upload_image(base, longname, NULL, MAX_IMAGE_SIZE);
    print_result(5, rc);

    return 0;
}