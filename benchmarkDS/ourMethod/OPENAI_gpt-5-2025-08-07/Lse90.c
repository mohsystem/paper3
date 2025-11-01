#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    const char* code;
    double price;
} price_entry;

static const price_entry PRICE_MAP[] = {
    {"A1", 9.99},
    {"B2", 5.00},
    {"C3", 2.50},
    {"D4", 100.00},
    {"E5", 0.75}
};
static const size_t PRICE_MAP_LEN = sizeof(PRICE_MAP) / sizeof(PRICE_MAP[0]);

double get_price_for_product_code(const char* code) {
    if (code == NULL) return -1.0;
    for (size_t i = 0; i < PRICE_MAP_LEN; i++) {
        if (strcmp(PRICE_MAP[i].code, code) == 0) {
            return PRICE_MAP[i].price;
        }
    }
    return -1.0;
}

static int is_ascii_printable(const char* s) {
    if (s == NULL) return 0;
    for (const unsigned char* p = (const unsigned char*)s; *p; ++p) {
        unsigned char c = *p;
        if (c == '\n' || c == '\r' || c == '\t') continue;
        if (!(c >= 32 && c <= 126)) return 0;
    }
    return 1;
}

static void trim_inplace(char* s) {
    if (s == NULL) return;
    size_t len = strlen(s);
    size_t start = 0;
    while (start < len && isspace((unsigned char)s[start])) start++;
    size_t end = len;
    while (end > start && isspace((unsigned char)s[end - 1])) end--;
    if (start > 0) {
        memmove(s, s + start, end - start);
    }
    s[end - start] = '\0';
}

static void strip_quotes_inplace(char* s) {
    if (s == NULL) return;
    size_t len = strlen(s);
    if (len >= 2) {
        if ((s[0] == '\'' && s[len - 1] == '\'') || (s[0] == '"' && s[len - 1] == '"')) {
            s[len - 1] = '\0';
            memmove(s, s + 1, len - 1);
        }
    }
}

static int is_valid_product_code(const char* s) {
    if (s == NULL) return 0;
    size_t len = strlen(s);
    if (len < 1 || len > 20) return 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)s[i];
        if (!(isalnum(c) || c == '_' || c == '-')) return 0;
    }
    return 1;
}

static int parse_positive_int(const char* s, long long minv, long long maxv, long long* out) {
    if (s == NULL || out == NULL) return 0;
    size_t len = strlen(s);
    if (len == 0) return 0;
    for (size_t i = 0; i < len; i++) {
        if (!isdigit((unsigned char)s[i])) return 0;
    }
    // parse
    long long v = 0;
    for (size_t i = 0; i < len; i++) {
        int d = s[i] - '0';
        if (v > (9223372036854775807LL - d) / 10) return 0; // overflow guard
        v = v * 10 + d;
    }
    if (v < minv || v > maxv) return 0;
    *out = v;
    return 1;
}

static char* dup_str(const char* s) {
    size_t n = strlen(s);
    char* out = (char*)malloc(n + 1);
    if (!out) return NULL;
    memcpy(out, s, n + 1);
    return out;
}

char* calculate_price(const char* yamlPayload) {
    if (yamlPayload == NULL) {
        return dup_str("error: invalid_request");
    }
    size_t inLen = strlen(yamlPayload);
    if (inLen == 0 || inLen > 4096) {
        return dup_str("error: invalid_request");
    }
    if (!is_ascii_printable(yamlPayload)) {
        return dup_str("error: invalid_request");
    }

    // Make a modifiable copy
    char* buf = (char*)malloc(inLen + 1);
    if (!buf) return dup_str("error: invalid_request");
    memcpy(buf, yamlPayload, inLen + 1);

    char product_code[32];
    product_code[0] = '\0';
    char quantity_str[32];
    quantity_str[0] = '\0';

    size_t line_count = 0;
    char* saveptr = NULL;
    char* line = strtok_r(buf, "\r\n", &saveptr);
    while (line) {
        line_count++;
        if (line_count > 200) {
            free(buf);
            return dup_str("error: invalid_request");
        }
        // Trim line
        trim_inplace(line);
        if (line[0] != '\0' && line[0] != '#') {
            char* colon = strchr(line, ':');
            if (colon != NULL && colon != line) {
                *colon = '\0';
                char* key = line;
                char* val = colon + 1;
                trim_inplace(key);
                trim_inplace(val);
                strip_quotes_inplace(val);
                if (strcmp(key, "product_code") == 0) {
                    size_t vlen = strlen(val);
                    if (vlen < sizeof(product_code)) {
                        strncpy(product_code, val, sizeof(product_code) - 1);
                        product_code[sizeof(product_code) - 1] = '\0';
                    }
                } else if (strcmp(key, "quantity") == 0) {
                    size_t vlen = strlen(val);
                    if (vlen < sizeof(quantity_str)) {
                        strncpy(quantity_str, val, sizeof(quantity_str) - 1);
                        quantity_str[sizeof(quantity_str) - 1] = '\0';
                    }
                }
            }
        }
        line = strtok_r(NULL, "\r\n", &saveptr);
    }

    if (product_code[0] == '\0' || quantity_str[0] == '\0') {
        free(buf);
        return dup_str("error: invalid_request");
    }
    if (!is_valid_product_code(product_code)) {
        free(buf);
        return dup_str("error: invalid_request");
    }
    long long qty = 0;
    if (!parse_positive_int(quantity_str, 1, 1000000, &qty)) {
        free(buf);
        return dup_str("error: invalid_request");
    }

    double price = get_price_for_product_code(product_code);
    if (price < 0.0) {
        free(buf);
        return dup_str("error: unknown_product_code");
    }

    double total = price * (double)qty;
    char out[64];
    int written = snprintf(out, sizeof(out), "total_price: %.2f", total);
    free(buf);
    if (written < 0 || (size_t)written >= sizeof(out)) {
        return dup_str("error: invalid_request");
    }
    return dup_str(out);
}

int main(void) {
    const char* tests[5] = {
        "product_code: A1\nquantity: 3\n",
        "product_code: B2\nquantity: 10\n",
        "product_code: Z9\nquantity: 1\n",
        "product_code: C3\nquantity: -2\n",
        " product_code : 'E5' \n quantity : 4 \n"
    };
    for (int i = 0; i < 5; i++) {
        char* res = calculate_price(tests[i]);
        if (res) {
            printf("Test %d -> %s\n", i + 1, res);
            free(res);
        } else {
            printf("Test %d -> error: invalid_request\n", i + 1);
        }
    }
    return 0;
}