#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

double get_price_for_product_code(const char* product_code) {
    if (product_code == NULL) return 0.0;
    const struct { const char* code; double price; } table[] = {
        {"A100", 10.00},
        {"B200", 5.50},
        {"C300", 2.25},
        {"D400", 12.75},
        {"E500", 99.99}
    };
    char buf[128];
    size_t n = strlen(product_code);
    if (n >= sizeof(buf)) n = sizeof(buf)-1;
    memcpy(buf, product_code, n);
    buf[n] = '\0';
    // trim
    char* s = buf;
    while (*s && isspace((unsigned char)*s)) s++;
    char* e = s + strlen(s);
    while (e > s && isspace((unsigned char)e[-1])) e--;
    *e = '\0';

    for (size_t i = 0; i < sizeof(table)/sizeof(table[0]); ++i) {
        if (strcmp(s, table[i].code) == 0) return table[i].price;
    }
    return 0.0;
}

static void trim_inplace(char* str) {
    if (!str) return;
    char* start = str;
    while (*start && isspace((unsigned char)*start)) start++;
    char* end = start + strlen(start);
    while (end > start && isspace((unsigned char)end[-1])) end--;
    size_t len = (size_t)(end - start);
    memmove(str, start, len);
    str[len] = '\0';
}

static char* strdup_portable(const char* s) {
    size_t len = strlen(s);
    char* p = (char*)malloc(len + 1);
    if (!p) return NULL;
    memcpy(p, s, len + 1);
    return p;
}

char* calculate_price(const char* yaml_query_param_payload) {
    char product_code[128] = {0};
    int quantity = 0;

    if (yaml_query_param_payload && *yaml_query_param_payload) {
        char* tmp = strdup_portable(yaml_query_param_payload);
        if (tmp) {
            char* saveptr = NULL;
            char* line = strtok_r(tmp, "\r\n", &saveptr);
            while (line) {
                char* colon = strchr(line, ':');
                if (colon) {
                    *colon = '\0';
                    char* key = line;
                    char* val = colon + 1;
                    trim_inplace(key);
                    trim_inplace(val);
                    if (strcmp(key, "product_code") == 0) {
                        strncpy(product_code, val, sizeof(product_code) - 1);
                        product_code[sizeof(product_code)-1] = '\0';
                    } else if (strcmp(key, "quantity") == 0) {
                        quantity = atoi(val);
                    }
                }
                line = strtok_r(NULL, "\r\n", &saveptr);
            }
            free(tmp);
        }
    }

    double price = get_price_for_product_code(product_code);
    double total = price * quantity;

    char buffer[64];
    snprintf(buffer, sizeof(buffer), "total_price: %.2f", total);
    return strdup_portable(buffer);
}

int main(void) {
    const char* tests[5] = {
        "product_code: A100\nquantity: 3",
        "product_code: B200\nquantity: 2",
        "product_code: C300\nquantity: 0",
        "product_code: UNKNOWN\nquantity: 5",
        "quantity: 4\nproduct_code: D400"
    };
    for (int i = 0; i < 5; ++i) {
        char* out = calculate_price(tests[i]);
        if (out) {
            printf("%s\n", out);
            free(out);
        }
    }
    return 0;
}