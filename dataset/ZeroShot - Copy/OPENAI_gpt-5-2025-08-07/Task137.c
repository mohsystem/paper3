#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>

#define NAME_MAX_LEN 63
#define DESC_MAX_LEN 127

typedef struct {
    int id;
    char name[NAME_MAX_LEN+1];
    double price;
    char description[DESC_MAX_LEN+1];
} Product;

// Validate ID
static bool normalize_id(int id, int* out_id) {
    if (id < 0 || id > 1000000000) return false;
    if (out_id) *out_id = id;
    return true;
}

// Validate query: allowed chars and length 1..100
static bool is_safe_char(char c) {
    return isalnum((unsigned char)c) || c==' ' || c=='_' || c=='-' || c=='.' || c=='\'';
}

static bool normalize_query(const char* q, char* out_lower, size_t out_size) {
    if (!q || !out_lower || out_size == 0) return false;
    size_t len = 0;
    // trim leading
    while (*q && isspace((unsigned char)*q)) q++;
    const char* start = q;
    const char* end = q + strlen(q);
    while (end > start && isspace((unsigned char)*(end-1))) end--;
    len = (size_t)(end - start);
    if (len == 0 || len > 100) return false;
    // validate and to-lower copy
    if (len >= out_size) return false;
    for (size_t i = 0; i < len; i++) {
        char c = start[i];
        if (!is_safe_char(c)) return false;
        out_lower[i] = (char)tolower((unsigned char)c);
    }
    out_lower[len] = '\0';
    return true;
}

// case-insensitive substring search
static bool contains_case_insensitive(const char* haystack, const char* needle) {
    if (!haystack || !needle) return false;
    size_t nlen = strlen(needle);
    if (nlen == 0) return true;
    size_t hlen = strlen(haystack);
    if (nlen > hlen) return false;

    for (size_t i = 0; i + nlen <= hlen; i++) {
        size_t j = 0;
        for (; j < nlen; j++) {
            unsigned char hc = (unsigned char)tolower((unsigned char)haystack[i+j]);
            unsigned char nc = (unsigned char)tolower((unsigned char)needle[j]);
            if (hc != nc) break;
        }
        if (j == nlen) return true;
    }
    return false;
}

// Function: query product details by ID
// Returns 1 if found and writes to out_product; else 0
int get_product_by_id(const Product* table, size_t n, int id, Product* out_product) {
    int safe_id;
    if (!table || !out_product) return 0;
    if (!normalize_id(id, &safe_id)) return 0;
    for (size_t i = 0; i < n; i++) {
        if (table[i].id == safe_id) {
            *out_product = table[i];
            return 1;
        }
    }
    return 0;
}

// Function: query products by name (case-insensitive substring)
// Returns number of matches written to out_results (up to out_cap)
size_t get_products_by_name(const Product* table, size_t n, const char* name_query, Product* out_results, size_t out_cap) {
    if (!table || !out_results || out_cap == 0) return 0;
    char qlower[101];
    if (!normalize_query(name_query, qlower, sizeof(qlower))) return 0;

    size_t count = 0;
    for (size_t i = 0; i < n; i++) {
        if (contains_case_insensitive(table[i].name, qlower)) {
            if (count < out_cap) {
                out_results[count] = table[i];
            }
            count++;
            if (count >= out_cap) {
                // continue counting but not writing to avoid overflow
                // In a real system, we'd reallocate or indicate truncation
            }
        }
    }
    if (count > out_cap) count = out_cap; // only returning how many were stored
    return count;
}

static void print_product(const Product* p) {
    if (!p) {
        printf("null\n");
        return;
    }
    printf("Product{id=%d, name='%s', price=%.2f, description='%s'}\n",
           p->id, p->name, p->price, p->description);
}

int main(void) {
    Product table[] = {
        {1, "Apple iPhone 14", 799.00, "Latest Apple smartphone"},
        {2, "Samsung Galaxy S23", 749.99, "Flagship Android phone"},
        {3, "Google Pixel 8", 699.00, "Google's premium device"},
        {4, "Apple AirPods Pro", 249.00, "Noise-cancelling earbuds"},
        {5, "Dell XPS 13", 1199.00, "Ultrabook laptop"},
    };
    size_t n = sizeof(table)/sizeof(table[0]);

    // Test case 1: by ID existing
    Product out;
    printf("Test1 (ID=3): ");
    if (get_product_by_id(table, n, 3, &out)) print_product(&out); else print_product(NULL);

    // Test case 2: by ID not existing
    printf("Test2 (ID=999): ");
    if (get_product_by_id(table, n, 999, &out)) print_product(&out); else print_product(NULL);

    // Test case 3: by name "apple"
    printf("Test3 (name='apple'):\n");
    Product results[10];
    size_t found = get_products_by_name(table, n, "apple", results, 10);
    for (size_t i = 0; i < found; i++) {
        print_product(&results[i]);
    }

    // Test case 4: by name "XPS"
    printf("Test4 (name='XPS'):\n");
    found = get_products_by_name(table, n, "XPS", results, 10);
    for (size_t i = 0; i < found; i++) {
        print_product(&results[i]);
    }

    // Test case 5: invalid query input
    printf("Test5 (invalid name):\n");
    found = get_products_by_name(table, n, "bad; DROP TABLE product;--", results, 10);
    if (found == 0) {
        printf("  Caught expected error or no results due to invalid query.\n");
    } else {
        for (size_t i = 0; i < found; i++) print_product(&results[i]);
    }

    return 0;
}