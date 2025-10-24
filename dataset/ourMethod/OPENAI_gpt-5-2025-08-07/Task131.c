#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#define MAGIC0 'U'
#define MAGIC1 'S'
#define MAGIC2 'R'
#define MAGIC3 '1'
#define VERSION 1

#define USERNAME_MIN 1
#define USERNAME_MAX 64
#define EMAIL_MIN 3
#define EMAIL_MAX 254
#define AGE_MIN 0
#define AGE_MAX 120

typedef struct {
    uint32_t userId;
    char* username; // allocated
    uint8_t age;
    char* email; // allocated
} UserRecord;

static int is_ascii_printable(const char* s, size_t len) {
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)s[i];
        if (c < 0x20 || c > 0x7E) return 0;
    }
    return 1;
}

static int username_valid(const char* s, size_t len) {
    if (len < USERNAME_MIN || len > USERNAME_MAX) return 0;
    for (size_t i = 0; i < len; i++) {
        char ch = s[i];
        if (!((ch >= 'A' && ch <= 'Z') ||
              (ch >= 'a' && ch <= 'z') ||
              (ch >= '0' && ch <= '9') ||
              ch == '_' || ch == '.' || ch == '-')) return 0;
    }
    return 1;
}

static int email_valid(const char* s, size_t len) {
    if (len < EMAIL_MIN || len > EMAIL_MAX) return 0;
    size_t atPos = (size_t)-1;
    for (size_t i = 0; i < len; i++) {
        char ch = s[i];
        int allowed = ((ch >= 'A' && ch <= 'Z') ||
                       (ch >= 'a' && ch <= 'z') ||
                       (ch >= '0' && ch <= '9') ||
                       ch == '.' || ch == '_' || ch == '%' || ch == '+' || ch == '-' || ch == '@');
        if (!allowed) return 0;
        if (ch == '@') {
            if (atPos != (size_t)-1) return 0; // multiple @
            atPos = i;
        }
    }
    if (atPos == (size_t)-1 || atPos == 0 || atPos == len - 1) return 0;
    // dot after @
    size_t dotAfter = (size_t)-1;
    for (size_t i = atPos + 1; i < len; i++) {
        if (s[i] == '.') { dotAfter = i; break; }
    }
    if (dotAfter == (size_t)-1 || dotAfter == atPos + 1 || dotAfter == len - 1) return 0;
    return 1;
}

static void free_user_record(UserRecord* r) {
    if (!r) return;
    free(r->username);
    free(r->email);
    r->username = NULL;
    r->email = NULL;
}

static int serialize(const UserRecord* r, uint8_t** outBuf, size_t* outLen) {
    if (!r || !outBuf || !outLen) return 0;
    *outBuf = NULL; *outLen = 0;
    if (r->username == NULL || r->email == NULL) return 0;
    size_t ulen = strnlen(r->username, USERNAME_MAX + 1);
    if (ulen < USERNAME_MIN || ulen > USERNAME_MAX) return 0;
    if (!username_valid(r->username, ulen)) return 0;
    if (!is_ascii_printable(r->username, ulen)) return 0;
    size_t elen = strnlen(r->email, EMAIL_MAX + 1);
    if (elen < EMAIL_MIN || elen > EMAIL_MAX) return 0;
    if (!is_ascii_printable(r->email, elen)) return 0;
    if (!email_valid(r->email, elen)) return 0;
    if (r->age < AGE_MIN || r->age > AGE_MAX) return 0;

    size_t total = 4 + 1 + 4 + 1 + ulen + 1 + 1 + elen;
    uint8_t* buf = (uint8_t*)malloc(total);
    if (!buf) return 0;
    size_t pos = 0;
    buf[pos++] = MAGIC0; buf[pos++] = MAGIC1; buf[pos++] = MAGIC2; buf[pos++] = MAGIC3;
    buf[pos++] = VERSION;
    buf[pos++] = (uint8_t)((r->userId >> 24) & 0xFF);
    buf[pos++] = (uint8_t)((r->userId >> 16) & 0xFF);
    buf[pos++] = (uint8_t)((r->userId >> 8) & 0xFF);
    buf[pos++] = (uint8_t)(r->userId & 0xFF);
    buf[pos++] = (uint8_t)(ulen & 0xFF);
    memcpy(&buf[pos], r->username, ulen);
    pos += ulen;
    buf[pos++] = r->age;
    buf[pos++] = (uint8_t)(elen & 0xFF);
    memcpy(&buf[pos], r->email, elen);
    pos += elen;

    *outBuf = buf;
    *outLen = pos;
    return 1;
}

static int deserialize(const uint8_t* data, size_t len, UserRecord* out, char* errBuf, size_t errBufLen) {
    if (errBuf && errBufLen) errBuf[0] = '\0';
    if (!data || !out) { if (errBuf) snprintf(errBuf, errBufLen, "Null input"); return 0; }
    if (len < 12) { if (errBuf) snprintf(errBuf, errBufLen, "Data too short"); return 0; }

    size_t pos = 0;
    if (!(data[0]==MAGIC0 && data[1]==MAGIC1 && data[2]==MAGIC2 && data[3]==MAGIC3)) {
        if (errBuf) snprintf(errBuf, errBufLen, "Invalid magic"); return 0;
    }
    pos += 4;
    if (pos >= len) { if (errBuf) snprintf(errBuf, errBufLen, "Missing version"); return 0; }
    uint8_t version = data[pos++];
    if (version != VERSION) { if (errBuf) snprintf(errBuf, errBufLen, "Unsupported version"); return 0; }
    if (pos + 4 > len) { if (errBuf) snprintf(errBuf, errBufLen, "Insufficient userId"); return 0; }
    uint32_t userId = ((uint32_t)data[pos] << 24) | ((uint32_t)data[pos+1] << 16) |
                      ((uint32_t)data[pos+2] << 8) | (uint32_t)data[pos+3];
    pos += 4;
    if (pos >= len) { if (errBuf) snprintf(errBuf, errBufLen, "Missing username length"); return 0; }
    size_t ulen = data[pos++];
    if (ulen < USERNAME_MIN || ulen > USERNAME_MAX) { if (errBuf) snprintf(errBuf, errBufLen, "Invalid username length"); return 0; }
    if (pos + ulen > len) { if (errBuf) snprintf(errBuf, errBufLen, "Insufficient username"); return 0; }
    char* username = (char*)malloc(ulen + 1);
    if (!username) { if (errBuf) snprintf(errBuf, errBufLen, "OOM"); return 0; }
    memcpy(username, &data[pos], ulen);
    username[ulen] = '\0';
    pos += ulen;
    if (!is_ascii_printable(username, ulen) || !username_valid(username, ulen)) {
        if (errBuf) snprintf(errBuf, errBufLen, "Invalid username content");
        free(username); return 0;
    }

    if (pos >= len) { if (errBuf) snprintf(errBuf, errBufLen, "Missing age"); free(username); return 0; }
    uint8_t age = data[pos++];
    if (age < AGE_MIN || age > AGE_MAX) { if (errBuf) snprintf(errBuf, errBufLen, "Invalid age"); free(username); return 0; }

    if (pos >= len) { if (errBuf) snprintf(errBuf, errBufLen, "Missing email length"); free(username); return 0; }
    size_t elen = data[pos++];
    if (elen < EMAIL_MIN || elen > EMAIL_MAX) { if (errBuf) snprintf(errBuf, errBufLen, "Invalid email length"); free(username); return 0; }
    if (pos + elen > len) { if (errBuf) snprintf(errBuf, errBufLen, "Insufficient email"); free(username); return 0; }
    char* email = (char*)malloc(elen + 1);
    if (!email) { if (errBuf) snprintf(errBuf, errBufLen, "OOM"); free(username); return 0; }
    memcpy(email, &data[pos], elen);
    email[elen] = '\0';
    pos += elen;
    if (!is_ascii_printable(email, elen) || !email_valid(email, elen)) {
        if (errBuf) snprintf(errBuf, errBufLen, "Invalid email content");
        free(username); free(email); return 0;
    }

    if (pos != len) { if (errBuf) snprintf(errBuf, errBufLen, "Trailing data"); free(username); free(email); return 0; }

    out->userId = userId;
    out->username = username;
    out->age = age;
    out->email = email;
    return 1;
}

static void print_record(const UserRecord* r) {
    if (!r) return;
    printf("UserRecord{id=%u, username='%s', age=%u, email='%s'}\n",
           r->userId, r->username, (unsigned)r->age, r->email);
}

int main(void) {
    // 1) Valid
    UserRecord r1;
    r1.userId = 123456u;
    r1.username = "alice_01";
    r1.age = 30u;
    r1.email = "alice@example.com";
    uint8_t* d1 = NULL; size_t d1len = 0;
    if (!serialize(&r1, &d1, &d1len)) {
        printf("T1 serialize failed\n");
        return 1;
    }
    UserRecord out = {0};
    char err[128];
    if (deserialize(d1, d1len, &out, err, sizeof(err))) {
        printf("T1: "); print_record(&out);
        free_user_record(&out);
    } else {
        printf("T1 fail: %s\n", err);
    }

    // 2) Invalid magic
    if (d1len >= 1) d1[0] = 'B';
    if (deserialize(d1, d1len, &out, err, sizeof(err))) {
        printf("T2 unexpected success\n");
        free_user_record(&out);
    } else {
        printf("T2: %s\n", err);
    }
    // restore for next tests
    d1[0] = MAGIC0;

    // 3) Invalid age (200)
    size_t ulen = d1[4 + 1 + 4];
    size_t agePos = 4 + 1 + 4 + 1 + ulen;
    if (agePos < d1len) d1[agePos] = 200;
    if (deserialize(d1, d1len, &out, err, sizeof(err))) {
        printf("T3 unexpected success\n");
        free_user_record(&out);
    } else {
        printf("T3: %s\n", err);
    }
    // fix age for further tests
    d1[agePos] = 30;

    // 4) Invalid username content (space) - serializer should fail
    UserRecord r4;
    r4.userId = 42u;
    r4.username = "bad name";
    r4.age = 25u;
    r4.email = "bn@example.org";
    uint8_t* d4 = NULL; size_t d4len = 0;
    if (!serialize(&r4, &d4, &d4len)) {
        printf("T4: serializer rejected invalid username\n");
    } else {
        printf("T4 unexpected success size=%zu\n", d4len);
        free(d4);
    }

    // 5) Invalid email content (missing @) - serializer should fail
    UserRecord r5;
    r5.userId = 77u;
    r5.username = "bob-77";
    r5.age = 44u;
    r5.email = "bob#example.com";
    uint8_t* d5 = NULL; size_t d5len = 0;
    if (!serialize(&r5, &d5, &d5len)) {
        printf("T5: serializer rejected invalid email\n");
    } else {
        printf("T5 unexpected success size=%zu\n", d5len);
        free(d5);
    }

    free(d1);
    return 0;
}