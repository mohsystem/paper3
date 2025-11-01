#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define MAX_PAYLOAD_LEN 4096

typedef struct {
    const char* code;
    long long price_cents;
} PriceEntry;

static const PriceEntry PRICE_MAP[] = {
    {"A100", 1999},   /* $19.99 */
    {"B200", 599},    /* $5.99 */
    {"C300", 2500},   /* $25.00 */
    {"D400", 12345},  /* $123.45 */
    {"E500", 1}       /* $0.01 */
};
static const size_t PRICE_MAP_LEN = sizeof(PRICE_MAP) / sizeof(PRICE_MAP[0]);

static int valid_code_format(const char* code) {
    if (!code) return 0;
    size_t len = strlen(code);
    if (len == 0 || len > 32) return 0;
    for (size_t i = 0; i < len; i++) {
        char c = code[i];
        if (!( (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-' )) {
            return 0;
        }
    }
    return 1;
}

static void strtoupper_inplace(char* s) {
    if (!s) return;
    while (*s) {
        *s = (char)toupper((unsigned char)*s);
        s++;
    }
}

int get_price_for_product_code(const char* product_code, long long* out_price, int* found) {
    if (found) *found = 0;
    if (out_price) *out_price = 0;
    if (!product_code) return 0;
    char buf[64];
    size_t n = strlen(product_code);
    if (n >= sizeof(buf)) return 0;
    memcpy(buf, product_code, n + 1);
    // trim
    char* start = buf;
    while (*start && isspace((unsigned char)*start)) start++;
    char* end = start + strlen(start);
    while (end > start && isspace((unsigned char)*(end - 1))) *(--end) = '\0';
    strtoupper_inplace(start);
    if (!valid_code_format(start)) return 1; // invalid format, not found
    for (size_t i = 0; i < PRICE_MAP_LEN; i++) {
        if (strcmp(PRICE_MAP[i].code, start) == 0) {
            if (out_price) *out_price = PRICE_MAP[i].price_cents;
            if (found) *found = 1;
            return 1;
        }
    }
    return 1;
}

static void strip_quotes_inplace(char* s) {
    size_t len = strlen(s);
    if (len >= 2 && ((s[0] == '"' && s[len-1] == '"') || (s[0] == '\'' && s[len-1] == '\''))) {
        memmove(s, s + 1, len - 2);
        s[len - 2] = '\0';
    }
}

static void trim_inplace(char* s) {
    char* p = s;
    while (*p && isspace((unsigned char)*p)) p++;
    if (p != s) memmove(s, p, strlen(p) + 1);
    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1])) {
        s[--len] = '\0';
    }
}

static int parse_simple_yaml(const char* payload, char* out_code, size_t out_code_sz, char* out_qty, size_t out_qty_sz) {
    out_code[0] = '\0';
    out_qty[0] = '\0';
    char* dup = strdup(payload ? payload : "");
    if (!dup) return 0;
    char* saveptr = NULL;
    for (char* line = strtok_r(dup, "\r\n", &saveptr); line; line = strtok_r(NULL, "\r\n", &saveptr)) {
        char* t = line;
        trim_inplace(t);
        if (t[0] == '\0' || t[0] == '#') continue;
        char* colon = strchr(t, ':');
        if (!colon || colon == t) continue;
        *colon = '\0';
        char* key = t;
        char* val = colon + 1;
        trim_inplace(key);
        trim_inplace(val);
        // lowercase key
        for (char* k = key; *k; ++k) *k = (char)tolower((unsigned char)*k);
        strip_quotes_inplace(val);
        if (strcmp(key, "product_code") == 0) {
            strncpy(out_code, val, out_code_sz - 1);
            out_code[out_code_sz - 1] = '\0';
        } else if (strcmp(key, "quantity") == 0) {
            strncpy(out_qty, val, out_qty_sz - 1);
            out_qty[out_qty_sz - 1] = '\0';
        }
    }
    free(dup);
    return 1;
}

static void escape_yaml(const char* in, char* out, size_t out_sz) {
    size_t j = 0;
    for (size_t i = 0; in && in[i] && j + 2 < out_sz; i++) {
        char c = in[i];
        if (c == '\\') { out[j++]='\\'; out[j++]='\\'; }
        else if (c == '"') { out[j++]='\\'; out[j++]='"'; }
        else if (c == '\n') { out[j++]='\\'; out[j++]='n'; }
        else if (c == '\r') { out[j++]='\\'; out[j++]='r'; }
        else { out[j++]=c; }
    }
    out[j] = '\0';
}

static void format_cents(long long cents, char* out, size_t out_sz) {
    long long dollars = cents / 100;
    long long rem = llabs(cents % 100);
    snprintf(out, out_sz, "%lld.%02lld", dollars, rem);
}

char* calculate_price(const char* yaml_payload) {
    if (!yaml_payload || yaml_payload[0] == '\0') {
        char* r = (char*)malloc(28); if (!r) return NULL;
        strcpy(r, "error: \"missing payload\"\n");
        return r;
    }
    if (strlen(yaml_payload) > MAX_PAYLOAD_LEN) {
        char* r = (char*)malloc(28); if (!r) return NULL;
        strcpy(r, "error: \"payload too large\"\n");
        return r;
    }
    char code[64], qtytxt[64];
    if (!parse_simple_yaml(yaml_payload, code, sizeof(code), qtytxt, sizeof(qtytxt))) {
        char* r = (char*)malloc(27); if (!r) return NULL;
        strcpy(r, "error: \"parse failure\"\n");
        return r;
    }
    if (code[0] == '\0') {
        char* r = (char*)malloc(32); if (!r) return NULL;
        strcpy(r, "error: \"missing product_code\"\n");
        return r;
    }
    // Uppercase and validate
    strtoupper_inplace(code);
    if (!valid_code_format(code)) {
        char* r = (char*)malloc(38); if (!r) return NULL;
        strcpy(r, "error: \"invalid product_code format\"\n");
        return r;
    }
    long long price_cents = 0;
    int found = 0;
    get_price_for_product_code(code, &price_cents, &found);
    if (!found) {
        char* r = (char*)malloc(32); if (!r) return NULL;
        strcpy(r, "error: \"unknown product_code\"\n");
        return r;
    }
    if (qtytxt[0] == '\0') {
        char* r = (char*)malloc(28); if (!r) return NULL;
        strcpy(r, "error: \"missing quantity\"\n");
        return r;
    }
    char* endp = NULL;
    if (qtytxt[0] == '+') {
        memmove(qtytxt, qtytxt + 1, strlen(qtytxt));
    }
    long long qty = strtoll(qtytxt, &endp, 10);
    if (endp == qtytxt || *endp != '\0') {
        char* r = (char*)malloc(38); if (!r) return NULL;
        strcpy(r, "error: \"quantity must be an integer\"\n");
        return r;
    }
    if (qty <= 0) {
        char* r = (char*)malloc(34); if (!r) return NULL;
        strcpy(r, "error: \"quantity must be positive\"\n");
        return r;
    }
    if (qty > 1000000LL) {
        char* r = (char*)malloc(31); if (!r) return NULL;
        strcpy(r, "error: \"quantity too large\"\n");
        return r;
    }
    long long total = price_cents * qty; /* Safe under constraints */
    char amt[64];
    format_cents(total, amt, sizeof(amt));
    size_t need = strlen(amt) + 20;
    char* r = (char*)malloc(need);
    if (!r) return NULL;
    snprintf(r, need, "total_price: \"%s\"\n", amt);
    return r;
}

int main(void) {
    const char* tests[5] = {
        "product_code: A100\nquantity: 2",
        "product_code: \"B200\"\nquantity: 3",
        "product_code: Z999\nquantity: 1",
        "product_code: C300\nquantity: -5",
        "product_code: D400\nquantity: two"
    };
    for (int i = 0; i < 5; i++) {
        char* out = calculate_price(tests[i]);
        if (out) {
            fputs(out, stdout);
            free(out);
        }
    }
    return 0;
}