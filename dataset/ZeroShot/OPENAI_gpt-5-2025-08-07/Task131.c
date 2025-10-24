#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define MAX_INPUT 4096
#define MAX_FIELD_LEN 256

static int is_ws(char c) {
    return c==' ' || c=='\n' || c=='\r' || c=='\t';
}

static size_t skip_ws(const char* s, size_t i, size_t end) {
    while (i < end && is_ws(s[i])) i++;
    return i;
}

typedef struct {
    char value[MAX_FIELD_LEN+1];
    size_t index;
    int ok;
} StrRes;

typedef struct {
    long long value;
    size_t index;
    int ok;
} NumRes;

static StrRes parse_json_string(const char* s, size_t i, size_t end) {
    StrRes r; r.value[0] = '\0'; r.index = i; r.ok = 0;
    if (i >= end || s[i] != '"') return r;
    i++;
    size_t out_len = 0;
    int esc = 0;
    while (i < end) {
        char c = s[i++];
        if (esc) {
            char o;
            switch (c) {
                case '"': o = '"'; break;
                case '\\': o = '\\'; break;
                case '/': o = '/'; break;
                case 'b': o = '\b'; break;
                case 'f': o = '\f'; break;
                case 'n': o = '\n'; break;
                case 'r': o = '\r'; break;
                case 't': o = '\t'; break;
                case 'u':
                    // reject for simplicity
                    return r;
                default:
                    return r;
            }
            if (out_len >= MAX_FIELD_LEN) return r;
            r.value[out_len++] = o;
            esc = 0;
        } else {
            if (c == '\\') {
                esc = 1;
            } else if (c == '"') {
                r.value[out_len] = '\0';
                r.index = i;
                r.ok = 1;
                return r;
            } else {
                if ((unsigned char)c < 0x20) return r;
                if (out_len >= MAX_FIELD_LEN) return r;
                r.value[out_len++] = c;
            }
        }
    }
    return r;
}

static NumRes parse_json_int(const char* s, size_t i, size_t end) {
    NumRes r; r.value = 0; r.index = i; r.ok = 0;
    int neg = 0;
    if (i < end && (s[i] == '+' || s[i] == '-')) {
        neg = s[i] == '-';
        i++;
    }
    if (i >= end || !isdigit((unsigned char)s[i])) return r;
    long long val = 0;
    while (i < end && isdigit((unsigned char)s[i])) {
        int d = s[i] - '0';
        val = val * 10 + d;
        if (val > INT_MAX) return r;
        i++;
    }
    if (neg) val = -val;
    r.value = val;
    r.index = i;
    r.ok = 1;
    return r;
}

static int is_valid_email(const char* email) {
    size_t len = strlen(email);
    if (len < 3 || len > MAX_FIELD_LEN) return 0;
    const char* atp = strchr(email, '@');
    if (!atp) return 0;
    size_t at = (size_t)(atp - email);
    if (at == 0 || at == len - 1) return 0;
    if (strchr(atp + 1, '@')) return 0;
    const char* dotp = strchr(atp + 1, '.');
    if (!dotp) return 0;
    size_t dot = (size_t)(dotp - email);
    if (dot == at + 1 || dot == len - 1) return 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)email[i];
        if (c <= 0x1F || c >= 0x7F) return 0;
        if (!(isalnum(c) || c=='.' || c=='-' || c=='_' || c=='+' || c=='@')) return 0;
    }
    return 1;
}

static void json_escape_into(const char* s, char* out, size_t out_cap) {
    // Writes escaped string including quotes into out
    size_t o = 0;
    if (o + 1 < out_cap) out[o++] = '"';
    for (size_t i = 0; s[i] != '\0'; i++) {
        char c = s[i];
        const char* rep = NULL;
        char buf[7];
        switch (c) {
            case '"': rep = "\\\""; break;
            case '\\': rep = "\\\\"; break;
            case '\b': rep = "\\b"; break;
            case '\f': rep = "\\f"; break;
            case '\n': rep = "\\n"; break;
            case '\r': rep = "\\r"; break;
            case '\t': rep = "\\t"; break;
            default:
                if ((unsigned char)c < 0x20) {
                    snprintf(buf, sizeof(buf), "\\u%04x", (unsigned char)c);
                    rep = buf;
                }
        }
        if (rep) {
            size_t rl = strlen(rep);
            for (size_t k = 0; k < rl && o + 1 < out_cap; k++) {
                out[o++] = rep[k];
            }
        } else {
            if (o + 1 < out_cap) out[o++] = c;
        }
    }
    if (o + 1 < out_cap) out[o++] = '"';
    if (o < out_cap) out[o] = '\0';
    else out[out_cap - 1] = '\0';
}

char* secure_deserialize(const char* input) {
    if (input == NULL) return NULL;
    size_t len = strnlen(input, MAX_INPUT + 1);
    if (len == 0 || len > MAX_INPUT) return NULL;

    size_t i = 0, n = len;
    while (i < n && is_ws(input[i])) i++;
    if (i >= n || input[i] != '{') return NULL;
    i++;
    size_t j = n;
    if (j == 0) return NULL;
    j--;
    while (j < n && is_ws(input[j])) {
        if (j == 0) break;
        j--;
    }
    if (input[j] != '}') return NULL;
    size_t limit = j;

    int idSet=0, nameSet=0, emailSet=0, ageSet=0;
    int idVal=0, ageVal=0;
    char nameVal[MAX_FIELD_LEN+1]; nameVal[0]='\0';
    char emailVal[MAX_FIELD_LEN+1]; emailVal[0]='\0';

    i = skip_ws(input, i, limit);
    if (i < limit && input[i] == '}') {
        return NULL;
    }

    while (i < limit) {
        i = skip_ws(input, i, limit);
        StrRes k = parse_json_string(input, i, limit);
        if (!k.ok) return NULL;
        char key[MAX_FIELD_LEN+1];
        strncpy(key, k.value, MAX_FIELD_LEN);
        key[MAX_FIELD_LEN] = '\0';
        i = k.index;
        i = skip_ws(input, i, limit);
        if (i >= limit || input[i] != ':') return NULL;
        i++;
        i = skip_ws(input, i, limit);

        if (strcmp(key, "id") == 0 || strcmp(key, "age") == 0) {
            NumRes nr = parse_json_int(input, i, limit);
            if (!nr.ok) return NULL;
            long long v = nr.value;
            if (v < 0 || v > INT_MAX) return NULL;
            if (strcmp(key, "id") == 0) {
                if (idSet) return NULL;
                idVal = (int)v; idSet = 1;
            } else {
                if (ageSet) return NULL;
                ageVal = (int)v; ageSet = 1;
            }
            i = nr.index;
        } else if (strcmp(key, "name") == 0 || strcmp(key, "email") == 0) {
            StrRes vr = parse_json_string(input, i, limit);
            if (!vr.ok) return NULL;
            if (strcmp(key, "name") == 0) {
                if (nameSet) return NULL;
                if (vr.value[0] == '\0') return NULL;
                strncpy(nameVal, vr.value, MAX_FIELD_LEN);
                nameVal[MAX_FIELD_LEN] = '\0';
                nameSet = 1;
            } else {
                if (emailSet) return NULL;
                if (!is_valid_email(vr.value)) return NULL;
                strncpy(emailVal, vr.value, MAX_FIELD_LEN);
                emailVal[MAX_FIELD_LEN] = '\0';
                emailSet = 1;
            }
            i = vr.index;
        } else {
            return NULL;
        }

        i = skip_ws(input, i, limit);
        if (i < limit && input[i] == ',') {
            i++;
            continue;
        } else if (i == limit) {
            break;
        } else {
            return NULL;
        }
    }

    if (i != limit) return NULL;
    if (!(idSet && nameSet && emailSet)) return NULL;

    // Build canonical JSON string
    // Maximum length: a bit conservative
    char escName[MAX_FIELD_LEN*6 + 3]; // worst-case escaping
    char escEmail[MAX_FIELD_LEN*6 + 3];
    json_escape_into(nameVal, escName, sizeof(escName));
    json_escape_into(emailVal, escEmail, sizeof(escEmail));

    size_t bufSize = 64 + strlen(escName) + strlen(escEmail);
    char* out = (char*)malloc(bufSize);
    if (!out) return NULL;
    if (ageSet) {
        snprintf(out, bufSize, "{\"id\":%d,\"name\":%s,\"email\":%s,\"age\":%d}", idVal, escName, escEmail, ageVal);
    } else {
        snprintf(out, bufSize, "{\"id\":%d,\"name\":%s,\"email\":%s}", idVal, escName, escEmail);
    }
    return out;
}

int main(void) {
    const char* tests[5] = {
        "{\"id\":123,\"name\":\"Alice\",\"email\":\"alice@example.com\",\"age\":30}",
        "{\"id\":1,\"name\":\"Bob\",\"email\":\"bob@example.com\"}",
        "{\"id\":2,\"name\":\"Mallory\",\"email\":\"m@ex.com\",\"role\":\"admin\"}",
        "{\"id\":3,\"name\":\"Eve\",\"email\":\"eve@example.com\",\"age\":\"30\"}",
        "{\"id\":4,\"name\":\"A \\\"quoted\\\" Name\",\"email\":\"a.q@example.com\",\"age\":45}"
    };
    for (int t = 0; t < 5; t++) {
        char* res = secure_deserialize(tests[t]);
        if (res == NULL) {
            puts("INVALID");
        } else {
            puts(res);
            free(res);
        }
    }
    return 0;
}