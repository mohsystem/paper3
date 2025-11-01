#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>

#define MAX_PAYLOAD_SIZE 10000

typedef struct {
    const char* code;
    long long price_cents;
} PriceItem;

static const PriceItem PRICE_TABLE[] = {
    {"P001", 999},
    {"P002", 1999},
    {"P003", 2500},
    {"P004", 12345},
    {"P005", 50}
};
static const size_t PRICE_TABLE_LEN = sizeof(PRICE_TABLE)/sizeof(PRICE_TABLE[0]);

static long long lookup_price_cents(const char* code) {
    for (size_t i = 0; i < PRICE_TABLE_LEN; ++i) {
        if (strcmp(PRICE_TABLE[i].code, code) == 0) return PRICE_TABLE[i].price_cents;
    }
    return -1;
}

static char* alloc_message(const char* prefix, const char* msg) {
    size_t len = strlen(prefix) + (msg ? strlen(msg) : 0) + 1;
    char* out = (char*)malloc(len);
    if (!out) return NULL;
    if (msg) snprintf(out, len, "%s%s", prefix, msg);
    else snprintf(out, len, "%s", prefix);
    return out;
}

static char* alloc_error(const char* msg) {
    return alloc_message("error: ", msg);
}

static char* alloc_total_from_cents(long long total_cents) {
    char buf[64];
    long long dollars = total_cents / 100;
    long long cents = total_cents % 100;
    if (cents < 0) cents = -cents;
    snprintf(buf, sizeof(buf), "%lld.%02lld", dollars, cents);
    size_t len = strlen(buf) + 1;
    char* out = (char*)malloc(len);
    if (!out) return NULL;
    memcpy(out, buf, len);
    return out;
}

static char* trim_inplace(char* s) {
    if (!s) return s;
    size_t len = strlen(s);
    size_t start = 0;
    while (start < len && isspace((unsigned char)s[start])) start++;
    size_t end = len;
    while (end > start && isspace((unsigned char)s[end - 1])) end--;
    s[end] = '\0';
    return s + start;
}

static void unquote_inplace(char* s) {
    size_t len = strlen(s);
    if (len >= 2 && ((s[0] == '"' && s[len - 1] == '"') || (s[0] == '\'' && s[len - 1] == '\''))) {
        memmove(s, s + 1, len - 2);
        s[len - 2] = '\0';
    }
}

static bool is_valid_product_code(const char* code) {
    size_t len = strlen(code);
    if (len == 0 || len > 32) return false;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)code[i];
        if (!(isalnum(c) || c == '_' || c == '-')) return false;
    }
    return true;
}

char* calculate_price(const char* yaml_payload) {
    if (!yaml_payload) {
        return alloc_error("payload is null");
    }
    size_t payload_len = strlen(yaml_payload);
    if (payload_len > MAX_PAYLOAD_SIZE) {
        return alloc_error("payload too large");
    }

    char* buf = (char*)malloc(payload_len + 1);
    if (!buf) return alloc_error("memory allocation failed");
    memcpy(buf, yaml_payload, payload_len + 1);

    char* product_code = NULL;
    char* quantity_str = NULL;

    char* saveptr = NULL;
    char* line = strtok_r(buf, "\r\n", &saveptr);
    while (line) {
        char* t = trim_inplace(line);
        if (*t == '\0' || *t == '#') {
            line = strtok_r(NULL, "\r\n", &saveptr);
            continue;
        }
        char* colon = strchr(t, ':');
        if (colon && colon != t) {
            *colon = '\0';
            char* key = trim_inplace(t);
            char* value = trim_inplace(colon + 1);
            unquote_inplace(value);
            if (strcmp(key, "product_code") == 0) {
                product_code = value;
            } else if (strcmp(key, "quantity") == 0) {
                quantity_str = value;
            }
        }
        line = strtok_r(NULL, "\r\n", &saveptr);
    }

    if (!product_code || *product_code == '\0') {
        free(buf);
        return alloc_error("missing product_code");
    }
    if (!is_valid_product_code(product_code)) {
        free(buf);
        return alloc_error("invalid product_code");
    }

    if (!quantity_str || *quantity_str == '\0') {
        free(buf);
        return alloc_error("missing quantity");
    }

    // Ensure quantity_str is digits only
    for (char* p = quantity_str; *p; ++p) {
        if (!isdigit((unsigned char)*p)) {
            free(buf);
            return alloc_error("invalid quantity");
        }
    }

    errno = 0;
    char* endptr = NULL;
    long long quantity = strtoll(quantity_str, &endptr, 10);
    if (errno != 0 || (endptr && *endptr != '\0')) {
        free(buf);
        return alloc_error("invalid quantity");
    }
    if (quantity <= 0 || quantity > 1000000) {
        free(buf);
        return alloc_error("quantity out of range");
    }

    long long price_cents = lookup_price_cents(product_code);
    if (price_cents < 0) {
        free(buf);
        return alloc_error("unknown product_code");
    }

    long long total_cents = price_cents * quantity;
    char* result = alloc_total_from_cents(total_cents);
    free(buf);
    if (!result) return alloc_error("memory allocation failed");
    return result;
}

int main(void) {
    const char* tests[5] = {
        "product_code: P001\nquantity: 3",
        "# Sample order\nproduct_code: \"P002\"\nquantity: 1",
        "product_code: P999\nquantity: 2",
        "product_code: P001\nquantity: -5",
        "product_code: P004\nquantity: 1000000"
    };
    for (int i = 0; i < 5; ++i) {
        char* res = calculate_price(tests[i]);
        if (res) {
            printf("Test %d: %s\n", i + 1, res);
            free(res);
        } else {
            printf("Test %d: error: unknown\n", i + 1);
        }
    }
    return 0;
}