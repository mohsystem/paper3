#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

/* Returns price in cents for a product code, or -1 if unknown */
int get_price_for_product_code(const char* code) {
    if (code == NULL) return -1;
    if (strcmp(code, "A100") == 0) return 1299; /* $12.99 */
    if (strcmp(code, "B200") == 0) return 2599; /* $25.99 */
    if (strcmp(code, "C300") == 0) return  499; /* $4.99  */
    if (strcmp(code, "D400") == 0) return 9999; /* $99.99 */
    if (strcmp(code, "E500") == 0) return  150; /* $1.50  */
    return -1;
}

static char* c_trim(char* s) {
    if (s == NULL) return s;
    while (isspace((unsigned char)*s)) s++;
    if (*s == 0) return s;
    char* end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return s;
}

static void strip_quotes_inplace(char* s) {
    size_t n = strlen(s);
    if (n >= 2) {
        if ((s[0] == '"' && s[n-1] == '"') || (s[0] == '\'' && s[n-1] == '\'')) {
            /* shift left and remove last */
            memmove(s, s+1, n-2);
            s[n-2] = '\0';
        }
    }
}

typedef struct {
    char* product_code;
    char* quantity;
} kv_result;

/* Very simple YAML "key: value" parser: fills product_code and quantity if present */
static int parse_simple_yaml(const char* in, kv_result* out) {
    if (!in || !out) return 0;
    size_t len = strlen(in);
    if (len > 10000) return 0;

    char* buf = (char*)malloc(len + 1);
    if (!buf) return 0;
    memcpy(buf, in, len + 1);

    out->product_code = NULL;
    out->quantity = NULL;

    char* saveptr = NULL;
    char* line = strtok_r(buf, "\r\n", &saveptr);
    while (line) {
        char* t = c_trim(line);
        if (*t == '\0' || *t == '#') {
            line = strtok_r(NULL, "\r\n", &saveptr);
            continue;
        }
        char* colon = strchr(t, ':');
        if (!colon || colon == t) {
            free(buf);
            return 0; /* malformed line */
        }
        *colon = '\0';
        char* key = c_trim(t);
        char* val = c_trim(colon + 1);
        strip_quotes_inplace(val);

        if (strcmp(key, "product_code") == 0 && out->product_code == NULL) {
            out->product_code = strdup(val);
        } else if (strcmp(key, "quantity") == 0 && out->quantity == NULL) {
            out->quantity = strdup(val);
        }

        line = strtok_r(NULL, "\r\n", &saveptr);
    }

    free(buf);
    return 1;
}

static void free_kv_result(kv_result* r) {
    if (!r) return;
    if (r->product_code) { free(r->product_code); r->product_code = NULL; }
    if (r->quantity) { free(r->quantity); r->quantity = NULL; }
}

static void cents_to_dollars(long long cents, char* out, size_t out_size) {
    int neg = cents < 0;
    unsigned long long absval = (unsigned long long)(neg ? -cents : cents);
    unsigned long long dollars = absval / 100ULL;
    unsigned long long rem = absval % 100ULL;
    if (neg) {
        snprintf(out, out_size, "-%llu.%02llu", dollars, rem);
    } else {
        snprintf(out, out_size, "%llu.%02llu", dollars, rem);
    }
}

/* Simulated web route: takes YAML query payload and returns YAML response (caller must free) */
char* calculate_price(const char* yaml_payload) {
    if (yaml_payload == NULL) {
        char* s = (char*)malloc(23);
        if (s) strcpy(s, "error: invalid_payload");
        return s;
    }
    if (strlen(yaml_payload) > 10000) {
        char* s = (char*)malloc(22);
        if (s) strcpy(s, "error: payload_too_large");
        return s;
    }

    kv_result kv;
    if (!parse_simple_yaml(yaml_payload, &kv)) {
        char* s = (char*)malloc(21);
        if (s) strcpy(s, "error: malformed_yaml");
        return s;
    }

    if (kv.product_code == NULL || kv.product_code[0] == '\0') {
        free_kv_result(&kv);
        char* s = (char*)malloc(26);
        if (s) strcpy(s, "error: missing_product_code");
        return s;
    }
    if (kv.quantity == NULL || kv.quantity[0] == '\0') {
        free_kv_result(&kv);
        char* s = (char*)malloc(22);
        if (s) strcpy(s, "error: missing_quantity");
        return s;
    }

    char* endptr = NULL;
    long long qty = strtoll(kv.quantity, &endptr, 10);
    if (endptr == kv.quantity || *c_trim(endptr) != '\0') {
        free_kv_result(&kv);
        char* s = (char*)malloc(23);
        if (s) strcpy(s, "error: invalid_quantity");
        return s;
    }
    if (qty <= 0 || qty > 1000000LL) {
        free_kv_result(&kv);
        char* s = (char*)malloc(27);
        if (s) strcpy(s, "error: quantity_out_of_range");
        return s;
    }

    int price_cents = get_price_for_product_code(kv.product_code);
    if (price_cents < 0) {
        free_kv_result(&kv);
        char* s = (char*)malloc(26);
        if (s) strcpy(s, "error: unknown_product_code");
        return s;
    }

    /* Overflow check: price_cents * qty fits in long long */
    if (qty != 0 && llabs((long long)price_cents) > LLONG_MAX / llabs(qty)) {
        free_kv_result(&kv);
        char* s = (char*)malloc(21);
        if (s) strcpy(s, "error: price_overflow");
        return s;
    }

    long long total = (long long)price_cents * qty;
    char price_buf[64];
    cents_to_dollars(total, price_buf, sizeof(price_buf));

    const char* prefix = "total_price: ";
    size_t out_len = strlen(prefix) + strlen(price_buf) + 1;
    char* out = (char*)malloc(out_len);
    if (out) {
        strcpy(out, prefix);
        strcat(out, price_buf);
    }

    free_kv_result(&kv);
    return out;
}

int main(void) {
    const char* tests[5] = {
        "product_code: A100\nquantity: 2",
        "product_code: B200\nquantity: 1",
        "product_code: C300\nquantity: 5",
        "product_code: Z999\nquantity: 1",
        "product_code: D400\nquantity: three"
    };
    for (int i = 0; i < 5; ++i) {
        printf("Test %d input:\n%s\n", i+1, tests[i]);
        char* out = calculate_price(tests[i]);
        if (out) {
            printf("Output:\n%s\n", out);
            free(out);
        } else {
            printf("Output:\n(error allocating output)\n");
        }
        printf("---\n");
    }
    return 0;
}