
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>

#define MAX_SIZE 40001
#define MIN_NUM 0
#define MAX_NUM 100

typedef struct {
    int* prefixProducts;
    size_t size;
    size_t capacity;
} ProductOfNumbers;

// Initialize the ProductOfNumbers structure with validation
ProductOfNumbers* productOfNumbersCreate(void) {
    // Allocate memory for the structure with NULL check
    ProductOfNumbers* obj = (ProductOfNumbers*)malloc(sizeof(ProductOfNumbers));
    if (obj == NULL) {
        fprintf(stderr, "Memory allocation failed for ProductOfNumbers\\n");
        return NULL;
    }
    
    // Initialize with capacity for expected size (start with reasonable capacity)
    obj->capacity = 1024;
    obj->prefixProducts = (int*)calloc(obj->capacity, sizeof(int));
    if (obj->prefixProducts == NULL) {
        fprintf(stderr, "Memory allocation failed for prefixProducts array\\n");
        free(obj);
        return NULL;
    }
    
    // Start with 1 as identity element for multiplication
    obj->prefixProducts[0] = 1;
    obj->size = 1;
    
    return obj;
}

// Add number to stream with comprehensive input validation
void productOfNumbersAdd(ProductOfNumbers* obj, int num) {
    // Validate object pointer is not NULL
    if (obj == NULL || obj->prefixProducts == NULL) {
        fprintf(stderr, "Invalid object pointer\\n");
        return;
    }
    
    // Validate input range according to constraints
    if (num < MIN_NUM || num > MAX_NUM) {
        fprintf(stderr, "Input must be between %d and %d\\n", MIN_NUM, MAX_NUM);
        return;
    }
    
    // Check if we need to expand capacity (with maximum limit)
    if (obj->size >= obj->capacity) {
        if (obj->capacity >= MAX_SIZE) {
            fprintf(stderr, "Maximum capacity reached\\n");
            return;
        }
        
        size_t newCapacity = obj->capacity * 2;
        if (newCapacity > MAX_SIZE) {
            newCapacity = MAX_SIZE;
        }
        
        // Reallocate with NULL check
        int* newArray = (int*)realloc(obj->prefixProducts, newCapacity * sizeof(int));
        if (newArray == NULL) {
            fprintf(stderr, "Memory reallocation failed\\n");
            return;
        }
        
        obj->prefixProducts = newArray;
        obj->capacity = newCapacity;
    }
    
    // Handle zero case - reset prefix products array
    if (num == 0) {
        // Clear sensitive data before resetting
        memset(obj->prefixProducts, 0, obj->size * sizeof(int));
        obj->size = 1;
        obj->prefixProducts[0] = 1;
    } else {
        // Validate multiplication for overflow before performing operation
        int64_t lastProduct = (int64_t)obj->prefixProducts[obj->size - 1];
        int64_t newProduct = lastProduct * (int64_t)num;
        
        // Check for 32-bit integer overflow (defensive check)
        if (newProduct > INT_MAX || newProduct < INT_MIN) {
            fprintf(stderr, "Product would overflow 32-bit integer\\n");
            return;
        }
        
        // Safe to add after bounds check
        if (obj->size < obj->capacity) {
            obj->prefixProducts[obj->size] = (int)newProduct;
            obj->size++;
        }
    }
}

// Get product of last k numbers with validation
int productOfNumbersGetProduct(ProductOfNumbers* obj, int k) {
    // Validate object pointer
    if (obj == NULL || obj->prefixProducts == NULL) {
        fprintf(stderr, "Invalid object pointer\\n");
        return 0;
    }
    
    // Validate k is in valid range
    if (k < 1 || k > 40000) {
        fprintf(stderr, "k must be between 1 and 40000\\n");
        return 0;
    }
    
    // Validate that we have enough numbers
    size_t numCount = obj->size - 1;
    if ((size_t)k > numCount) {
        fprintf(stderr, "Not enough numbers in stream\\n");
        return 0;
    }
    
    // If k extends beyond current sequence (after a zero), return 0
    if ((size_t)k >= obj->size) {
        return 0;
    }
    
    // Bounds-checked array access for division calculation
    size_t endIdx = obj->size - 1;
    size_t startIdx = endIdx - (size_t)k;
    
    // Additional bounds validation
    if (startIdx >= obj->size || endIdx >= obj->size) {
        fprintf(stderr, "Index out of bounds\\n");
        return 0;
    }
    
    // Safe integer division
    int result = obj->prefixProducts[endIdx] / obj->prefixProducts[startIdx];
    return result;
}

// Free allocated memory securely
void productOfNumbersFree(ProductOfNumbers* obj) {
    if (obj != NULL) {
        if (obj->prefixProducts != NULL) {
            // Clear sensitive data before freeing
            memset(obj->prefixProducts, 0, obj->capacity * sizeof(int));
            free(obj->prefixProducts);
            obj->prefixProducts = NULL;
        }
        free(obj);
    }
}

// Test cases
int main(void) {
    // Test case 1: Example from problem
    printf("Test 1: Example from problem\\n");
    ProductOfNumbers* obj1 = productOfNumbersCreate();
    if (obj1 == NULL) return 1;
    
    productOfNumbersAdd(obj1, 3);
    productOfNumbersAdd(obj1, 0);
    productOfNumbersAdd(obj1, 2);
    productOfNumbersAdd(obj1, 5);
    productOfNumbersAdd(obj1, 4);
    printf("getProduct(2): %d (expected 20)\\n", productOfNumbersGetProduct(obj1, 2));
    printf("getProduct(3): %d (expected 40)\\n", productOfNumbersGetProduct(obj1, 3));
    printf("getProduct(4): %d (expected 0)\\n", productOfNumbersGetProduct(obj1, 4));
    productOfNumbersAdd(obj1, 8);
    printf("getProduct(2): %d (expected 32)\\n\\n", productOfNumbersGetProduct(obj1, 2));
    productOfNumbersFree(obj1);
    
    // Test case 2: All non-zero
    printf("Test 2: All non-zero numbers\\n");
    ProductOfNumbers* obj2 = productOfNumbersCreate();
    if (obj2 == NULL) return 1;
    
    productOfNumbersAdd(obj2, 2);
    productOfNumbersAdd(obj2, 3);
    productOfNumbersAdd(obj2, 4);
    printf("getProduct(3): %d (expected 24)\\n\\n", productOfNumbersGetProduct(obj2, 3));
    productOfNumbersFree(obj2);
    
    // Test case 3: Single element
    printf("Test 3: Single element\\n");
    ProductOfNumbers* obj3 = productOfNumbersCreate();
    if (obj3 == NULL) return 1;
    
    productOfNumbersAdd(obj3, 5);
    printf("getProduct(1): %d (expected 5)\\n\\n", productOfNumbersGetProduct(obj3, 1));
    productOfNumbersFree(obj3);
    
    // Test case 4: Multiple zeros
    printf("Test 4: Multiple zeros\\n");
    ProductOfNumbers* obj4 = productOfNumbersCreate();
    if (obj4 == NULL) return 1;
    
    productOfNumbersAdd(obj4, 1);
    productOfNumbersAdd(obj4, 0);
    productOfNumbersAdd(obj4, 2);
    productOfNumbersAdd(obj4, 0);
    productOfNumbersAdd(obj4, 3);
    printf("getProduct(1): %d (expected 3)\\n\\n", productOfNumbersGetProduct(obj4, 1));
    productOfNumbersFree(obj4);
    
    // Test case 5: Large sequence
    printf("Test 5: Large sequence\\n");
    ProductOfNumbers* obj5 = productOfNumbersCreate();
    if (obj5 == NULL) return 1;
    
    productOfNumbersAdd(obj5, 1);
    productOfNumbersAdd(obj5, 2);
    productOfNumbersAdd(obj5, 3);
    productOfNumbersAdd(obj5, 4);
    productOfNumbersAdd(obj5, 5);
    printf("getProduct(3): %d (expected 60)\\n", productOfNumbersGetProduct(obj5, 3));
    productOfNumbersFree(obj5);
    
    return 0;
}
