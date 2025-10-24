
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_PRODUCTS 100
#define MAX_STRING 100

typedef struct {
    char id[MAX_STRING];
    char name[MAX_STRING];
    double price;
    char category[MAX_STRING];
    int stock;
} Product;

Product productTable[MAX_PRODUCTS];
int productCount = 0;

void initializeProductTable() {
    strcpy(productTable[0].id, "P001");
    strcpy(productTable[0].name, "Laptop");
    productTable[0].price = 999.99;
    strcpy(productTable[0].category, "Electronics");
    productTable[0].stock = 50;
    
    strcpy(productTable[1].id, "P002");
    strcpy(productTable[1].name, "Mouse");
    productTable[1].price = 29.99;
    strcpy(productTable[1].category, "Electronics");
    productTable[1].stock = 200;
    
    strcpy(productTable[2].id, "P003");
    strcpy(productTable[2].name, "Keyboard");
    productTable[2].price = 79.99;
    strcpy(productTable[2].category, "Electronics");
    productTable[2].stock = 150;
    
    strcpy(productTable[3].id, "P004");
    strcpy(productTable[3].name, "Monitor");
    productTable[3].price = 299.99;
    strcpy(productTable[3].category, "Electronics");
    productTable[3].stock = 75;
    
    strcpy(productTable[4].id, "P005");
    strcpy(productTable[4].name, "Desk Chair");
    productTable[4].price = 199.99;
    strcpy(productTable[4].category, "Furniture");
    productTable[4].stock = 30;
    
    productCount = 5;
}

void printProduct(Product* p) {
    printf("ID: %s, Name: %s, Price: $%.2f, Category: %s, Stock: %d\\n",
           p->id, p->name, p->price, p->category, p->stock);
}

Product* queryProduct(const char* productId) {
    for (int i = 0; i < productCount; i++) {
        if (strcmp(productTable[i].id, productId) == 0) {
            return &productTable[i];
        }
    }
    return NULL;
}

void toLowerCase(char* str) {
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

int queryProductsByCategory(const char* category, Product* results, int maxResults) {
    int count = 0;
    char catLower[MAX_STRING], searchLower[MAX_STRING];
    
    strcpy(searchLower, category);
    toLowerCase(searchLower);
    
    for (int i = 0; i < productCount && count < maxResults; i++) {
        strcpy(catLower, productTable[i].category);
        toLowerCase(catLower);
        
        if (strcmp(catLower, searchLower) == 0) {
            results[count++] = productTable[i];
        }
    }
    return count;
}

int queryProductsByPriceRange(double minPrice, double maxPrice, Product* results, int maxResults) {
    int count = 0;
    for (int i = 0; i < productCount && count < maxResults; i++) {
        if (productTable[i].price >= minPrice && productTable[i].price <= maxPrice) {
            results[count++] = productTable[i];
        }
    }
    return count;
}

int main() {
    initializeProductTable();
    Product results[MAX_PRODUCTS];
    int resultCount;
    
    printf("Test Case 1: Query by Product ID\\n");
    Product* product1 = queryProduct("P001");
    if (product1) {
        printProduct(product1);
    } else {
        printf("Product not found\\n");
    }
    printf("\\n");
    
    printf("Test Case 2: Query by Product ID (Non-existent)\\n");
    Product* product2 = queryProduct("P999");
    if (product2) {
        printProduct(product2);
    } else {
        printf("Product not found\\n");
    }
    printf("\\n");
    
    printf("Test Case 3: Query by Category\\n");
    resultCount = queryProductsByCategory("Electronics", results, MAX_PRODUCTS);
    printf("Electronics products: %d\\n", resultCount);
    for (int i = 0; i < resultCount; i++) {
        printProduct(&results[i]);
    }
    printf("\\n");
    
    printf("Test Case 4: Query by Price Range (50-100)\\n");
    resultCount = queryProductsByPriceRange(50.0, 100.0, results, MAX_PRODUCTS);
    printf("Products in range: %d\\n", resultCount);
    for (int i = 0; i < resultCount; i++) {
        printProduct(&results[i]);
    }
    printf("\\n");
    
    printf("Test Case 5: Query by Category (Furniture)\\n");
    resultCount = queryProductsByCategory("Furniture", results, MAX_PRODUCTS);
    printf("Furniture products: %d\\n", resultCount);
    for (int i = 0; i < resultCount; i++) {
        printProduct(&results[i]);
    }
    
    return 0;
}
