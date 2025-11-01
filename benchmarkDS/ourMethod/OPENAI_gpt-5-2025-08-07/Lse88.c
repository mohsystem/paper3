#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define MAX_INPUT_LEN 4096
#define MAX_KEY_VAL_LEN 64

typedef struct {
    const char *code;
    long long price_cents;
} CatalogItem;

static const CatalogItem CATALOG[] = {
    {"P001", 999},   // 9.99
    {"P002", 550},   // 5.50
    {"P003", 2000},  // 20.00
};
static const size_t CATALOG_SIZE = sizeof(CATALOG) / sizeof(CATALOG[0]);

static char* dupstr_heap(const char* s) {
    if (!s) return NULL;
    size_t n = strlen(s);
    char* p = (char*)malloc(n + 1);
    if (!p) return NULL;
    memcpy(p, s, n + 1);
    return p;
}

static void trim_copy(const char* src, char* dest, size_t dest_size) {
    size_t len = strlen(src);
    size_t start = 0;
    while (start < len && isspace((unsigned char)src[start])) start++;
    size_t end = len;
    while (end > start && isspace((unsigned char)src[end - 1])) end--;
    size_t out_len = (end > start) ? (end - start) : 0;
    if (out_len >= dest_size) out_len = dest_size - 1;
    if (out_len > 0) {
        memcpy(dest, src + start, out_len);
    }
    dest[out_len] = '\0';
}

static void strip_quotes_inplace(char* s) {
    size_t n = strlen(s);
    if (n >= 2) {
        char a = s[0];
        char b = s[n - 1];
        if ((a == b) && (a == '\'' || a == '"')) {
            memmove(s, s + 1, n - 2);
            s[n - 2] = '\0';
        }
    }
}

static int is_valid_product_code(const char* s) {
    size_t n = strlen(s);
    if (n == 0 || n > 32) return 0;
    for (size_t i = 0; i < n; i++) {
        unsigned char c = (unsigned char)s[i];
        if (!(isalnum(c) || c == '_' || c == '-')) return 0;
    }
    return 1;
}

static int parse_quantity(const char* s, long long* out) {
    if (!s || !out) return 0;
    size_t n = strlen(s);
    if (n == 0 || n > 9) return 0;
    size_t i = 0;
    if (s[0] == '+' || s[0] == '-') {
        if (n == 1) return 0;
        i = 1;
    }
    for (; i < n; i++) {
        if (!isdigit((unsigned char)s[i])) return 0;
    }
    char* endp = NULL;
    long long val = strtoll(s, &endp, 10);
    if (endp == NULL || *endp != '\0') return 0;
    if (val <= 0 || val > 1000000) return 0;
    *out = val;
    return 1;
}

static long long get_price_cents(const char* code) {
    for (size_t i = 0; i < CATALOG_SIZE; i++) {
        if (strcmp(CATALOG[i].code, code) == 0) {
            return CATALOG[i].price_cents;
        }
    }
    return -1;
}

static char* format_cents(long long cents) {
    // Max digits for dollars: up to 19 digits for long long, plus dot and 2 cents and null
    char buf[64];
    long long dollars = cents / 100;
    long long rem = cents % 100;
    if (rem < 0) rem = -rem;
    // snprintf is safe here with fixed-size buffer
    int written = snprintf(buf, sizeof(buf), "%lld.%02lld", dollars, rem);
    if (written < 0 || written >= (int)sizeof(buf)) {
        return dupstr_heap("error: processing failed");
    }
    return dupstr_heap(buf);
}

char* calculate_price(const char* yaml_payload) {
    if (yaml_payload == NULL) {
        return dupstr_heap("error: invalid input");
    }
    size_t n = strnlen(yaml_payload, MAX_INPUT_LEN + 1);
    if (n == 0 || n > MAX_INPUT_LEN) {
        return dupstr_heap("error: invalid input");
    }
    for (size_t i = 0; i < n; i++) {
        if (yaml_payload[i] == '\0') {
            return dupstr_heap("error: invalid input");
        }
    }

    char *copy = (char*)malloc(n + 1);
    if (!copy) return dupstr_heap("error: processing failed");
    memcpy(copy, yaml_payload, n);
    copy[n] = '\0';

    char product_code[MAX_KEY_VAL_LEN + 1] = {0};
    char quantity_str[MAX_KEY_VAL_LEN + 1] = {0};
    int have_product_code = 0, have_quantity = 0;

    char linebuf[512];
    char* p = copy;
    while (*p != '\0') {
        char* eol = strpbrk(p, "\r\n");
        size_t len = eol ? (size_t)(eol - p) : strlen(p);
        if (len >= sizeof(linebuf)) {
            free(copy);
            return dupstr_heap("error: invalid input");
        }
        memcpy(linebuf, p, len);
        linebuf[len] = '\0';

        char trimmed[512];
        trim_copy(linebuf, trimmed, sizeof(trimmed));
        if (trimmed[0] != '\0' && trimmed[0] != '#') {
            char* colon = strchr(trimmed, ':');
            if (colon) {
                *colon = '\0';
                char key[MAX_KEY_VAL_LEN + 1];
                char val[MAX_KEY_VAL_LEN + 1];
                trim_copy(trimmed, key, sizeof(key));
                trim_copy(colon + 1, val, sizeof(val));

                if (key[0] == '\0' || strlen(key) > MAX_KEY_VAL_LEN || strlen(val) > MAX_KEY_VAL_LEN) {
                    free(copy);
                    return dupstr_heap("error: invalid input");
                }
                strip_quotes_inplace(val);

                if (strcmp(key, "product_code") == 0) {
                    strncpy(product_code, val, MAX_KEY_VAL_LEN);
                    product_code[MAX_KEY_VAL_LEN] = '\0';
                    have_product_code = 1;
                } else if (strcmp(key, "quantity") == 0) {
                    strncpy(quantity_str, val, MAX_KEY_VAL_LEN);
                    quantity_str[MAX_KEY_VAL_LEN] = '\0';
                    have_quantity = 1;
                }
            }
        }

        if (!eol) break;
        // skip CRLF or LF
        p = eol + 1;
        if (*eol == '\r' && *p == '\n') p++;
    }

    free(copy);

    if (!have_product_code || !have_quantity) {
        return dupstr_heap("error: missing product_code or quantity");
    }
    if (!is_valid_product_code(product_code)) {
        return dupstr_heap("error: invalid product_code");
    }
    long long quantity = 0;
    if (!parse_quantity(quantity_str, &quantity)) {
        return dupstr_heap("error: invalid quantity");
    }
    long long price = get_price_cents(product_code);
    if (price < 0) {
        return dupstr_heap("error: unknown product_code");
    }
    if (quantity > 0 && price > 0 && quantity > LLONG_MAX / price) {
        return dupstr_heap("error: overflow");
    }
    long long total = price * quantity;
    return format_cents(total);
}

int main(void) {
    const char* tests[5] = {
        "product_code: P001\nquantity: 3\n",
        "quantity: 10\nproduct_code: P002\n",
        "product_code: UNKNOWN\nquantity: 1\n",
        "product_code: P001\nquantity: -5\n",
        "product_code: P003\n"
    };
    for (int i = 0; i < 5; i++) {
        char* result = calculate_price(tests[i]);
        if (result) {
            printf("Test %d: %s\n", i + 1, result);
            free(result);
        } else {
            printf("Test %d: error: processing failed\n", i + 1);
        }
    }
    return 0;
}