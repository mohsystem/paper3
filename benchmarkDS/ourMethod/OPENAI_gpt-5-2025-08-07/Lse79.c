#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { BUF_SIZE = 1024, READ_100 = 100 };

typedef struct {
    char *buf;
    char *buf1;
    size_t len1;
    size_t len2;
} ReadPair;

typedef struct {
    char *buf;
    size_t len;
} ReadSingle;

static size_t read_at_most(FILE *in, char *dest, size_t dest_size, size_t max_to_read) {
    if (in == NULL || dest == NULL || dest_size == 0) return 0;
    size_t limit = max_to_read;
    if (limit > dest_size - 1) { // leave space for NUL terminator
        limit = dest_size - 1;
    }
    size_t total = 0;
    while (total < limit) {
        size_t to_read = limit - total;
        size_t r = fread(dest + total, 1, to_read, in);
        total += r;
        if (r == 0) {
            if (feof(in) || ferror(in)) {
                break;
            }
        }
    }
    dest[total] = '\0';
    return total;
}

ReadPair allocate_and_copy_100_twice(FILE *in) {
    ReadPair rp;
    rp.buf = NULL;
    rp.buf1 = NULL;
    rp.len1 = 0;
    rp.len2 = 0;

    if (in == NULL) {
        return rp;
    }

    rp.buf = (char *)calloc(BUF_SIZE, 1);
    rp.buf1 = (char *)calloc(BUF_SIZE, 1);
    if (!rp.buf || !rp.buf1) {
        free(rp.buf);
        free(rp.buf1);
        rp.buf = NULL;
        rp.buf1 = NULL;
        rp.len1 = 0;
        rp.len2 = 0;
        return rp;
    }

    rp.len1 = read_at_most(in, rp.buf, BUF_SIZE, READ_100);
    rp.len2 = read_at_most(in, rp.buf1, BUF_SIZE, READ_100);
    return rp;
}

ReadSingle read_up_to_1024(FILE *in) {
    ReadSingle rs;
    rs.buf = NULL;
    rs.len = 0;

    if (in == NULL) {
        return rs;
    }

    rs.buf = (char *)calloc(BUF_SIZE, 1);
    if (!rs.buf) {
        return rs;
    }
    rs.len = read_at_most(in, rs.buf, BUF_SIZE, BUF_SIZE - 1);
    return rs;
}

static FILE* make_stream_from_cstr(const char *s) {
    FILE *f = tmpfile();
    if (!f) return NULL;
    if (s && *s) {
        size_t len = strlen(s);
        if (len > 0) {
            if (fwrite(s, 1, len, f) != len) {
                fclose(f);
                return NULL;
            }
        }
    }
    fflush(f);
    rewind(f);
    return f;
}

int main(void) {
    // Test case 1: Less than 100 bytes
    {
        const char *t1 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
        FILE *f1 = make_stream_from_cstr(t1);
        if (!f1) return 1;
        ReadPair rp1 = allocate_and_copy_100_twice(f1);
        printf("T1 buf: %s\n", rp1.buf ? rp1.buf : "");
        printf("T1 buf1: %s\n", rp1.buf1 ? rp1.buf1 : "");
        free(rp1.buf);
        free(rp1.buf1);
        fclose(f1);
    }

    // Test case 2: More than 100 bytes (150 'x')
    {
        char *t2 = (char *)malloc(151);
        if (!t2) return 1;
        memset(t2, 'x', 150);
        t2[150] = '\0';
        FILE *f2 = make_stream_from_cstr(t2);
        free(t2);
        if (!f2) return 1;
        ReadPair rp2 = allocate_and_copy_100_twice(f2);
        printf("T2 buf len: %zu\n", rp2.len1);
        printf("T2 buf1 len: %zu\n", rp2.len2);
        free(rp2.buf);
        free(rp2.buf1);
        fclose(f2);
    }

    // Test case 3: Empty input
    {
        FILE *f3 = make_stream_from_cstr("");
        if (!f3) return 1;
        ReadPair rp3 = allocate_and_copy_100_twice(f3);
        printf("T3 buf: '%s'\n", rp3.buf ? rp3.buf : "");
        printf("T3 buf1: '%s'\n", rp3.buf1 ? rp3.buf1 : "");
        free(rp3.buf);
        free(rp3.buf1);
        fclose(f3);
    }

    // Test case 4: read_up_to_1024 with 300 bytes of 'y'
    {
        char *t4 = (char *)malloc(301);
        if (!t4) return 1;
        memset(t4, 'y', 300);
        t4[300] = '\0';
        FILE *f4 = make_stream_from_cstr(t4);
        free(t4);
        if (!f4) return 1;
        ReadSingle rs4 = read_up_to_1024(f4);
        printf("T4 len: %zu\n", rs4.len);
        free(rs4.buf);
        fclose(f4);
    }

    // Test case 5: Unicode-like UTF-8 sequence (treated as bytes)
    {
        const char *t5 = "Hello \xE4\xBD\xA0\xE5\xA5\xBD\xE4\xB8\x96\xE7\x95\x8C \xF0\x9F\x99\x82\xF0\x9F\x9A\x80"
                         "Hello \xE4\xBD\xA0\xE5\xA5\xBD\xE4\xB8\x96\xE7\x95\x8C \xF0\x9F\x99\x82\xF0\x9F\x9A\x80"
                         "Hello \xE4\xBD\xA0\xE5\xA5\xBD\xE4\xB8\x96\xE7\x95\x8C \xF0\x9F\x99\x82\xF0\x9F\x9A\x80";
        FILE *f5 = make_stream_from_cstr(t5);
        if (!f5) return 1;
        ReadPair rp5 = allocate_and_copy_100_twice(f5);
        printf("T5 buf: %s\n", rp5.buf ? rp5.buf : "");
        printf("T5 buf1: %s\n", rp5.buf1 ? rp5.buf1 : "");
        free(rp5.buf);
        free(rp5.buf1);
        fclose(f5);
    }

    return 0;
}