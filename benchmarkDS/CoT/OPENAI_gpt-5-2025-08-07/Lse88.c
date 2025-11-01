/* 
Step 1: Problem understanding
- Provide calculate_price that takes a YAML payload string, extracts product_code and quantity, computes total, and returns a newly allocated string with the result.

Step 2: Security requirements
- No dynamic code execution; simple, safe parsing.
- Enforce max payload length; validate formats strictly.
- Use integer cents for money to avoid floating-point errors.

Step 3: Secure coding generation
- Implement trimming, quote stripping, and strict validations.
- Handle overflows and bounds.

Step 4: Code review
- Ensure null checks, bounds, and safe memory handling.

Step 5: Secure code output
- Return price or "ERROR: ..." string. Caller must free().
- Include 5 test cases in main.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define MAX_PAYLOAD_LENGTH 4096

typedef struct {
    const char* code;
    long long cents;
} PriceItem;

static const PriceItem PRICE_MAP[] = {
    {"P001", 1999},
    {"P002", 550},
    {"P003", 9995},
    {"P100", 125},
    {"P900", 25000},
};
static const size_t PRICE_MAP_LEN = sizeof(PRICE_MAP) / sizeof(PRICE_MAP[0]);

static char* strdup_safe(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, s, len + 1);
    return out;
}

static char* trim(char* s) {
    if (!s) return s;
    // left trim
    while (*s && isspace((unsigned char)*s)) s++;
    // right trim
    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1])) {
        s[len - 1] = '\0';
        len--;
    }
    return s;
}

static int is_quoted(const char* s) {
    size_t len = s ? strlen(s) : 0;
    if (len >= 2) {
        if ((s[0] == '"' && s[len-1] == '"') || (s[0] == '\'' && s[len-1] == '\'')) return 1;
    }
    return 0;
}

static char* strip_quotes_inplace(char* s) {
    size_t len = s ? strlen(s) : 0;
    if (len >= 2 && ((s[0] == '"' && s[len-1] == '"') || (s[0] == '\'' && s[len-1] == '\''))) {
        s[len-1] = '\0';
        return s + 1;
    }
    return s;
}

static void to_upper_inplace(char* s) {
    for (; s && *s; ++s) {
        *s = (char)toupper((unsigned char)*s);
    }
}

static int is_valid_code(const char* s) {
    if (!s || !*s) return 0;
    size_t len = strlen(s);
    if (len > 20) return 0;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)s[i];
        if (!(isalnum(c) || c == '_' || c == '-')) return 0;
    }
    return 1;
}

static int is_digits(const char* s) {
    if (!s || !*s) return 0;
    for (const char* p = s; *p; ++p) {
        if (!isdigit((unsigned char)*p)) return 0;
    }
    return 1;
}

static long long find_price_cents(const char* code) {
    for (size_t i = 0; i < PRICE_MAP_LEN; ++i) {
        if (strcmp(PRICE_MAP[i].code, code) == 0) {
            return PRICE_MAP[i].cents;
        }
    }
    return -1;
}

static void safe_strcpy(char* dst, size_t dst_size, const char* src) {
    if (dst_size == 0) return;
    if (!src) {
        dst[0] = '\0';
        return;
    }
    size_t n = strlen(src);
    if (n >= dst_size) n = dst_size - 1;
    memcpy(dst, src, n);
    dst[n] = '\0';
}

static void format_cents_to_string(long long cents, char* out, size_t out_size) {
    long long dollars = cents / 100;
    long long rem = llabs(cents % 100);
    char buf[64];
    snprintf(buf, sizeof(buf), "%lld.%02lld", dollars, rem);
    safe_strcpy(out, out_size, buf);
}

char* calculate_price(const char* yaml_payload) {
    if (yaml_payload == NULL) {
        return strdup_safe("ERROR: Missing payload");
    }
    size_t len = strlen(yaml_payload);
    if (len > MAX_PAYLOAD_LENGTH) {
        return strdup_safe("ERROR: Payload too large");
    }

    char* payload_copy = strdup_safe(yaml_payload);
    if (!payload_copy) return strdup_safe("ERROR: Internal memory error");

    // Normalize CRLF and CR to LF in place
    for (char* p = payload_copy; *p; ++p) {
        if (*p == '\r') *p = '\n';
    }

    char* product_code = NULL;
    char* quantity_str = NULL;

    char* saveptr = NULL;
    char* line = strtok_r(payload_copy, "\n", &saveptr);
    while (line) {
        char* t = trim(line);
        if (*t != '#' && *t != '\0') {
            char* colon = strchr(t, ':');
            if (colon && colon != t) {
                *colon = '\0';
                char* key = trim(t);
                char* val = trim(colon + 1);
                if (is_quoted(val)) {
                    val = strip_quotes_inplace(val);
                    val = trim(val);
                }

                // Convert key to lowercase
                for (char* k = key; *k; ++k) *k = (char)tolower((unsigned char)*k);

                if (strcmp(key, "product_code") == 0) {
                    product_code = val;
                } else if (strcmp(key, "quantity") == 0) {
                    quantity_str = val;
                }
            }
        }
        line = strtok_r(NULL, "\n", &saveptr);
    }

    if (!product_code || !*product_code) {
        free(payload_copy);
        return strdup_safe("ERROR: Missing product_code");
    }
    if (!quantity_str || !*quantity_str) {
        free(payload_copy);
        return strdup_safe("ERROR: Missing quantity");
    }

    // Prepare product code uppercase
    char code_buf[64];
    safe_strcpy(code_buf, sizeof(code_buf), product_code);
    to_upper_inplace(code_buf);
    if (!is_valid_code(code_buf)) {
        free(payload_copy);
        return strdup_safe("ERROR: Invalid product_code format");
    }

    char qty_buf[64];
    safe_strcpy(qty_buf, sizeof(qty_buf), quantity_str);
    qty_buf[63] = '\0';
    if (is_quoted(qty_buf)) {
        // strip quotes by creating a temp buffer
        size_t qlen = strlen(qty_buf);
        if (qlen >= 2) {
            memmove(qty_buf, qty_buf + 1, qlen - 2);
            qty_buf[qlen - 2] = '\0';
        }
    }
    if (!is_digits(qty_buf)) {
        free(payload_copy);
        return strdup_safe("ERROR: Invalid quantity");
    }

    long long qty = 0;
    // strtoll parsing
    {
        char* endp = NULL;
        qty = strtoll(qty_buf, &endp, 10);
        if (endp == qty_buf || *endp != '\0') {
            free(payload_copy);
            return strdup_safe("ERROR: Invalid quantity");
        }
    }
    if (qty <= 0 || qty > 1000000LL) {
        free(payload_copy);
        return strdup_safe("ERROR: Quantity out of allowed bounds");
    }

    long long unit_cents = find_price_cents(code_buf);
    if (unit_cents < 0) {
        free(payload_copy);
        return strdup_safe("ERROR: Unknown product_code");
    }

    // Overflow check: qty * unit_cents
    if (qty > 0 && unit_cents > 0 && qty > LLONG_MAX / unit_cents) {
        free(payload_copy);
        return strdup_safe("ERROR: Total exceeds limit");
    }
    long long total_cents = qty * unit_cents;

    char result[64];
    format_cents_to_string(total_cents, result, sizeof(result));
    free(payload_copy);
    return strdup_safe(result);
}

int main(void) {
    const char* tests[5] = {
        "product_code: P001\nquantity: 3",
        "product_code: P003\nquantity: 2",
        "product_code: 'P100'\nquantity: '10'",
        "product_code: X999\nquantity: 1",
        "product_code: P002\nquantity: five"
    };
    for (int i = 0; i < 5; ++i) {
        char* out = calculate_price(tests[i]);
        printf("Test %d: %s\n", i + 1, out ? out : "ERROR: Internal memory error");
        free(out);
    }
    return 0;
}