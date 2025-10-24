#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void secure_memzero(void *v, size_t n) {
    volatile unsigned char *p = (volatile unsigned char *)v;
    while (n--) {
        *p++ = 0;
    }
}

static int constant_time_equals(const char* a, size_t alen, const char* b, size_t blen) {
    size_t maxlen = (alen > blen) ? alen : blen;
    unsigned int diff = (unsigned int)(alen ^ blen);
    for (size_t i = 0; i < maxlen; ++i) {
        unsigned char ac = (i < alen) ? (unsigned char)a[i] : 0;
        unsigned char bc = (i < blen) ? (unsigned char)b[i] : 0;
        diff |= (unsigned int)(ac ^ bc);
    }
    return diff == 0;
}

int compare_password_from_file(const char* path, const char* user_input) {
    if (!path || !user_input) {
        return 0;
    }
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        return 0;
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return 0;
    }
    long fsz = ftell(fp);
    if (fsz < 0) {
        fclose(fp);
        return 0;
    }
    if (fsz > 1024 * 1024) { // cap at 1MB
        fclose(fp);
        return 0;
    }
    if (fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return 0;
    }

    size_t buflen = (size_t)fsz;
    char *buf = (char*)malloc(buflen + 1);
    if (!buf) {
        fclose(fp);
        return 0;
    }

    size_t readn = fread(buf, 1, buflen, fp);
    fclose(fp);
    buf[readn] = '\0';

    // Take only up to first newline as stored password
    size_t linelen = 0;
    while (linelen < readn && buf[linelen] != '\n') {
        linelen++;
    }

    size_t user_len = strlen(user_input);
    int result = constant_time_equals(buf, linelen, user_input, user_len);

    secure_memzero(buf, buflen + 1);
    free(buf);
    return result ? 1 : 0;
}

static void write_file(const char* path, const char* content) {
    FILE *fp = fopen(path, "wb");
    if (!fp) return;
    fwrite(content, 1, strlen(content), fp);
    fclose(fp);
}

int main(void) {
    // Test case 1: exact match
    const char *f1 = "c_pw_test1.txt";
    write_file(f1, "Secret123!");
    printf("%d\n", compare_password_from_file(f1, "Secret123!"));

    // Test case 2: mismatch
    const char *f2 = "c_pw_test2.txt";
    write_file(f2, "Secret123!");
    printf("%d\n", compare_password_from_file(f2, "secret123!"));

    // Test case 3: different length
    const char *f3 = "c_pw_test3.txt";
    write_file(f3, "Short");
    printf("%d\n", compare_password_from_file(f3, "Shorter"));

    // Test case 4: trailing newline
    const char *f4 = "c_pw_test4.txt";
    write_file(f4, "WithNewline\n");
    printf("%d\n", compare_password_from_file(f4, "WithNewline"));

    // Test case 5: file not found
    printf("%d\n", compare_password_from_file("c_pw_nonexistent_abcdefg.txt", "anything"));

    remove(f1);
    remove(f2);
    remove(f3);
    remove(f4);
    return 0;
}