#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Trim leading and trailing whitespace in place
static void strtrim(char *s) {
    if (!s) return;
    char *start = s;
    while (*start && isspace((unsigned char)*start)) start++;
    if (start != s) memmove(s, start, strlen(start) + 1);
    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1])) {
        s[len - 1] = '\0';
        len--;
    }
}

// Compare strings for equality
static int streq(const char* a, const char* b) {
    return strcmp(a, b) == 0;
}

// Lookup price for product code
static double lookup_price(const char* code) {
    if (!code) return 0.0;
    if (streq(code, "P001")) return 10.0;
    if (streq(code, "P002")) return 15.5;
    if (streq(code, "P003")) return 7.25;
    if (streq(code, "P004")) return 100.0;
    if (streq(code, "P005")) return 0.99;
    return 0.0;
}

// Entry point function: receives YAML payload as C-string and returns a newly allocated string
// Caller must free the returned string.
char* calculate_price(const char* yaml_payload) {
    char* payload_copy = NULL;
    char* line = NULL;
    char* saveptr = NULL;
    char* product_code = NULL;
    int quantity = 0;

    if (!yaml_payload) yaml_payload = "";

    payload_copy = (char*)malloc(strlen(yaml_payload) + 1);
    if (!payload_copy) {
        char* res = (char*)malloc(5);
        if (res) strcpy(res, "0.00");
        return res;
    }
    strcpy(payload_copy, yaml_payload);

    line = strtok_r(payload_copy, "\r\n", &saveptr);
    while (line) {
        char* work = (char*)malloc(strlen(line) + 1);
        if (work) {
            strcpy(work, line);
            strtrim(work);
            if (work[0] != '#' && work[0] != '\0') {
                char* colon = strchr(work, ':');
                if (colon) {
                    *colon = '\0';
                    char* key = work;
                    char* value = colon + 1;
                    strtrim(key);
                    strtrim(value);
                    if (streq(key, "product_code")) {
                        // store a copy of value
                        if (product_code) free(product_code);
                        product_code = (char*)malloc(strlen(value) + 1);
                        if (product_code) strcpy(product_code, value);
                    } else if (streq(key, "quantity")) {
                        quantity = atoi(value);
                        if (quantity < 0) quantity = 0;
                    }
                }
            }
            free(work);
        }
        line = strtok_r(NULL, "\r\n", &saveptr);
    }

    double price = lookup_price(product_code);
    double total = price * quantity;

    char buf[64];
    snprintf(buf, sizeof(buf), "%.2f", total);
    char* result = (char*)malloc(strlen(buf) + 1);
    if (result) strcpy(result, buf);

    if (product_code) free(product_code);
    if (payload_copy) free(payload_copy);
    return result;
}

int main() {
    const char* tests[5] = {
        "product_code: P001\nquantity: 3",
        "quantity: 2\nproduct_code: P002",
        "product_code: P003\nquantity: 4",
        "product_code: P999\nquantity: 1",
        "product_code: P005\nquantity: 5"
    };
    for (int i = 0; i < 5; ++i) {
        char* res = calculate_price(tests[i]);
        if (res) {
            printf("%s\n", res);
            free(res);
        } else {
            printf("0.00\n");
        }
    }
    return 0;
}