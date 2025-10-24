
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

/* Security: Maximum input lengths to prevent buffer overflow */
#define MAX_NAME_LEN 100
#define MAX_CATEGORY_LEN 50
#define MAX_INPUT_LEN 200
#define MAX_PRODUCTS 100

/* Security: Product structure with fixed-size buffers */
typedef struct {
    int id;
    char name[MAX_NAME_LEN + 1];  /* +1 for null terminator */
    char category[MAX_CATEGORY_LEN + 1];
    double price;
    int stock;
} Product;

/* Security: Database structure with capacity tracking */
typedef struct {
    Product products[MAX_PRODUCTS];
    int count;
} ProductDatabase;

/* Security: Initialize database with sample data */
void initDatabase(ProductDatabase* db) {
    if (db == NULL) return;
    
    /* Security: Zero-initialize entire structure */
    memset(db, 0, sizeof(ProductDatabase));
    
    db->count = 5;
    
    /* Security: Use strncpy to prevent buffer overflow */
    db->products[0].id = 1;
    strncpy(db->products[0].name, "Laptop", MAX_NAME_LEN);
    db->products[0].name[MAX_NAME_LEN] = '\\0';  /* Ensure null termination */
    strncpy(db->products[0].category, "Electronics", MAX_CATEGORY_LEN);
    db->products[0].category[MAX_CATEGORY_LEN] = '\\0';
    db->products[0].price = 999.99;
    db->products[0].stock = 10;
    
    db->products[1].id = 2;
    strncpy(db->products[1].name, "Mouse", MAX_NAME_LEN);
    db->products[1].name[MAX_NAME_LEN] = '\\0';
    strncpy(db->products[1].category, "Electronics", MAX_CATEGORY_LEN);
    db->products[1].category[MAX_CATEGORY_LEN] = '\\0';
    db->products[1].price = 29.99;
    db->products[1].stock = 50;
    
    db->products[2].id = 3;
    strncpy(db->products[2].name, "Keyboard", MAX_NAME_LEN);
    db->products[2].name[MAX_NAME_LEN] = '\\0';
    strncpy(db->products[2].category, "Electronics", MAX_CATEGORY_LEN);
    db->products[2].category[MAX_CATEGORY_LEN] = '\\0';
    db->products[2].price = 79.99;
    db->products[2].stock = 30;
    
    db->products[3].id = 4;
    strncpy(db->products[3].name, "Monitor", MAX_NAME_LEN);
    db->products[3].name[MAX_NAME_LEN] = '\\0';
    strncpy(db->products[3].category, "Electronics", MAX_CATEGORY_LEN);
    db->products[3].category[MAX_CATEGORY_LEN] = '\\0';
    db->products[3].price = 299.99;
    db->products[3].stock = 15;
    
    db->products[4].id = 5;
    strncpy(db->products[4].name, "Desk", MAX_NAME_LEN);
    db->products[4].name[MAX_NAME_LEN] = '\\0';
    strncpy(db->products[4].category, "Furniture", MAX_CATEGORY_LEN);
    db->products[4].category[MAX_CATEGORY_LEN] = '\\0';
    db->products[4].price = 399.99;
    db->products[4].stock = 8;
}

/* Security: Safe string sanitization - allow only alphanumeric, space, dash, underscore */
void sanitizeInput(const char* input, char* output, size_t maxLen) {
    if (input == NULL || output == NULL || maxLen == 0) return;
    
    size_t inputLen = strnlen(input, MAX_INPUT_LEN);
    size_t outIdx = 0;
    
    /* Security: Limit output length to prevent overflow */
    for (size_t i = 0; i < inputLen && outIdx < maxLen - 1; i++) {
        unsigned char c = (unsigned char)input[i];
        if (isalnum(c) || c == ' ' || c == '-' || c == '_') {
            output[outIdx++] = (char)c;
        }
    }
    output[outIdx] = '\\0';  /* Security: Ensure null termination */
}

/* Security: Safe integer parsing with overflow checks */
int parseInteger(const char* str, int* result) {
    if (str == NULL || result == NULL) return 0;
    
    size_t len = strnlen(str, 12);
    if (len == 0 || len > 10) return 0;  /* Prevent overflow */
    
    /* Security: Check for valid integer format */
    size_t start = 0;
    if (str[0] == '-' || str[0] == '+') start = 1;
    
    for (size_t i = start; i < len; i++) {
        if (!isdigit((unsigned char)str[i])) return 0;
    }
    
    /* Security: Use strtol with error checking */
    char* endptr = NULL;
    long val = strtol(str, &endptr, 10);
    
    /* Security: Check for conversion errors and overflow */
    if (endptr == str || *endptr != '\\0' || val < INT_MIN || val > INT_MAX) {
        return 0;
    }
    
    *result = (int)val;
    return 1;
}

/* Security: Case-insensitive string comparison with length limits */
int strcasecmp_safe(const char* s1, const char* s2, size_t maxLen) {
    if (s1 == NULL || s2 == NULL) return -1;
    
    for (size_t i = 0; i < maxLen; i++) {
        int c1 = tolower((unsigned char)s1[i]);
        int c2 = tolower((unsigned char)s2[i]);
        
        if (c1 != c2) return c1 - c2;
        if (c1 == '\\0') return 0;
    }
    return 0;
}

/* Security: Query by ID with validation */
int queryById(ProductDatabase* db, int id, Product* results, int maxResults) {
    if (db == NULL || results == NULL || maxResults <= 0) return 0;
    
    /* Security: Validate ID range */
    if (id <= 0 || id > 1000000) return 0;
    
    int count = 0;
    for (int i = 0; i < db->count && i < MAX_PRODUCTS; i++) {
        if (db->products[i].id == id) {
            /* Security: Bounds check before copy */
            if (count < maxResults) {
                memcpy(&results[count], &db->products[i], sizeof(Product));
                count++;
            }
            break;  /* ID should be unique */
        }
    }
    return count;
}

/* Security: Query by name with validation */
int queryByName(ProductDatabase* db, const char* name, Product* results, int maxResults) {
    if (db == NULL || name == NULL || results == NULL || maxResults <= 0) return 0;
    
    /* Security: Validate input length */
    size_t nameLen = strnlen(name, MAX_NAME_LEN + 1);
    if (nameLen == 0 || nameLen > MAX_NAME_LEN) return 0;
    
    int count = 0;
    for (int i = 0; i < db->count && i < MAX_PRODUCTS; i++) {
        /* Security: Safe case-insensitive comparison */
        if (strcasecmp_safe(db->products[i].name, name, MAX_NAME_LEN) == 0) {
            if (count < maxResults) {
                memcpy(&results[count], &db->products[i], sizeof(Product));
                count++;
            }
        }
    }
    return count;
}

/* Security: Query by category with validation */
int queryByCategory(ProductDatabase* db, const char* category, Product* results, int maxResults) {
    if (db == NULL || category == NULL || results == NULL || maxResults <= 0) return 0;
    
    /* Security: Validate input length */
    size_t catLen = strnlen(category, MAX_CATEGORY_LEN + 1);
    if (catLen == 0 || catLen > MAX_CATEGORY_LEN) return 0;
    
    int count = 0;
    for (int i = 0; i < db->count && i < MAX_PRODUCTS; i++) {
        if (strcasecmp_safe(db->products[i].category, category, MAX_CATEGORY_LEN) == 0) {
            if (count < maxResults) {
                memcpy(&results[count], &db->products[i], sizeof(Product));
                count++;
            }
        }
    }
    return count;
}

void displayProducts(Product* products, int count) {
    if (products == NULL || count <= 0) {
        printf("No products found.\\n");
        return;
    }
    
    printf("\\n=== Product Details ===\\n");
    for (int i = 0; i < count; i++) {
        printf("ID: %d\\n", products[i].id);
        printf("Name: %s\\n", products[i].name);
        printf("Category: %s\\n", products[i].category);
        printf("Price: $%.2f\\n", products[i].price);
        printf("Stock: %d\\n", products[i].stock);
        printf("----------------------\\n");
    }
}

int main(void) {
    ProductDatabase db;
    Product results[MAX_PRODUCTS];
    int resultCount;
    
    /* Security: Initialize database */
    initDatabase(&db);
    
    /* Test case 1: Query by valid ID */
    printf("Test 1: Query by ID (1)\\n");
    resultCount = queryById(&db, 1, results, MAX_PRODUCTS);
    displayProducts(results, resultCount);
    
    /* Test case 2: Query by name */
    printf("\\nTest 2: Query by name (Mouse)\\n");
    resultCount = queryByName(&db, "Mouse", results, MAX_PRODUCTS);
    displayProducts(results, resultCount);
    
    /* Test case 3: Query by category */
    printf("\\nTest 3: Query by category (Electronics)\\n");
    resultCount = queryByCategory(&db, "Electronics", results, MAX_PRODUCTS);
    displayProducts(results, resultCount);
    
    /* Test case 4: Invalid ID (negative) */
    printf("\\nTest 4: Query by invalid ID (-1)\\n");
    resultCount = queryById(&db, -1, results, MAX_PRODUCTS);
    displayProducts(results, resultCount);
    
    /* Test case 5: Non-existent product */
    printf("\\nTest 5: Query by non-existent name (Tablet)\\n");
    resultCount = queryByName(&db, "Tablet", results, MAX_PRODUCTS);
    displayProducts(results, resultCount);
    
    return 0;
}
