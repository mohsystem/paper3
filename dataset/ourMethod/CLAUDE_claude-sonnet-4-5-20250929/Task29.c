
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>
#include <string.h>

// Simple hash table entry for mapping values to indices
typedef struct {
    int value;
    size_t index;
    int occupied;
} HashEntry;

// Hash table structure with fixed size
typedef struct {
    HashEntry* entries;
    size_t capacity;
} HashTable;

// Initialize hash table with given capacity
// Returns NULL on allocation failure
HashTable* hash_table_create(size_t capacity) {
    // Validate capacity to prevent integer overflow in allocation
    if (capacity == 0 || capacity > SIZE_MAX / sizeof(HashEntry)) {
        return NULL;
    }
    
    HashTable* table = (HashTable*)malloc(sizeof(HashTable));
    if (table == NULL) {
        return NULL;
    }
    
    table->entries = (HashEntry*)calloc(capacity, sizeof(HashEntry));
    if (table->entries == NULL) {
        free(table);
        return NULL;
    }
    
    table->capacity = capacity;
    
    // Initialize all entries as unoccupied
    for (size_t i = 0; i < capacity; i++) {
        table->entries[i].occupied = 0;
    }
    
    return table;
}

// Clean up hash table and free memory
void hash_table_destroy(HashTable* table) {
    if (table != NULL) {
        if (table->entries != NULL) {
            free(table->entries);
        }
        free(table);
    }
}

// Hash function using simple modulo
size_t hash_function(int value, size_t capacity) {
    // Handle negative numbers by using absolute value
    // Use unsigned for modulo to avoid undefined behavior
    uint32_t abs_val = (value >= 0) ? (uint32_t)value : (uint32_t)(-value);
    return abs_val % capacity;
}

// Insert value and index into hash table
// Returns 1 on success, 0 on failure
int hash_table_insert(HashTable* table, int value, size_t index) {
    if (table == NULL || table->entries == NULL) {
        return 0;
    }
    
    size_t hash_index = hash_function(value, table->capacity);
    size_t original_index = hash_index;
    
    // Linear probing for collision resolution
    while (table->entries[hash_index].occupied) {
        // Value already exists, don't overwrite (keep first occurrence)\n        if (table->entries[hash_index].value == value) {\n            return 1;\n        }\n        \n        hash_index = (hash_index + 1) % table->capacity;\n        \n        // Table is full (shouldn't happen with proper sizing)
        if (hash_index == original_index) {
            return 0;
        }
    }
    
    table->entries[hash_index].value = value;
    table->entries[hash_index].index = index;
    table->entries[hash_index].occupied = 1;
    
    return 1;
}

// Find value in hash table and return index
// Returns 1 if found (index stored in out_index), 0 if not found
int hash_table_find(HashTable* table, int value, size_t* out_index) {
    if (table == NULL || table->entries == NULL || out_index == NULL) {
        return 0;
    }
    
    size_t hash_index = hash_function(value, table->capacity);
    size_t original_index = hash_index;
    
    while (table->entries[hash_index].occupied) {
        if (table->entries[hash_index].value == value) {
            *out_index = table->entries[hash_index].index;
            return 1;
        }
        
        hash_index = (hash_index + 1) % table->capacity;
        
        // Completed full cycle without finding
        if (hash_index == original_index) {
            return 0;
        }
    }
    
    return 0;
}

// Main two_sum function
// Returns 1 on success, 0 on failure
// Indices returned via out_index1 and out_index2 parameters
int two_sum(const int* numbers, size_t length, int target, size_t* out_index1, size_t* out_index2) {
    // Validate input parameters
    if (numbers == NULL || out_index1 == NULL || out_index2 == NULL) {
        return 0;
    }
    
    // Validate minimum length requirement
    if (length < 2) {
        return 0;
    }
    
    // Prevent resource exhaustion with unreasonably large arrays
    if (length > 1000000) {
        return 0;
    }
    
    // Create hash table with capacity 2x array size for good performance
    // Check for overflow in capacity calculation
    size_t capacity = length * 2;
    if (capacity < length) { // Overflow occurred
        return 0;
    }
    
    HashTable* seen = hash_table_create(capacity);
    if (seen == NULL) {
        return 0;
    }
    
    int found = 0;
    
    // Iterate through array
    for (size_t i = 0; i < length; i++) {
        int current = numbers[i];
        
        // Calculate complement with overflow check
        // Use long long for intermediate calculation
        long long complement_ll = (long long)target - (long long)current;
        
        // Check if complement is within int range
        if (complement_ll < INT_MIN || complement_ll > INT_MAX) {
            continue; // Skip if overflow would occur
        }
        
        int complement = (int)complement_ll;
        
        // Check if complement exists in hash table
        size_t prev_index;
        if (hash_table_find(seen, complement, &prev_index)) {
            // Found the pair
            *out_index1 = prev_index;
            *out_index2 = i;
            found = 1;
            break;
        }
        
        // Add current number to hash table
        if (!hash_table_insert(seen, current, i)) {
            // Insertion failed, cleanup and return error
            hash_table_destroy(seen);
            return 0;
        }
    }
    
    // Clean up hash table
    hash_table_destroy(seen);
    
    return found;
}

int main(void) {
    // Test case 1: Basic example
    int test1[] = {1, 2, 3};
    size_t idx1, idx2;
    if (two_sum(test1, 3, 4, &idx1, &idx2)) {
        printf("Test 1: [1,2,3], target=4 -> (%zu, %zu)\\n", idx1, idx2);
    }
    
    // Test case 2: Second example
    int test2[] = {3, 2, 4};
    if (two_sum(test2, 3, 6, &idx1, &idx2)) {
        printf("Test 2: [3,2,4], target=6 -> (%zu, %zu)\\n", idx1, idx2);
    }
    
    // Test case 3: Negative numbers
    int test3[] = {-1, -2, -3, -4, -5};
    if (two_sum(test3, 5, -8, &idx1, &idx2)) {
        printf("Test 3: [-1,-2,-3,-4,-5], target=-8 -> (%zu, %zu)\\n", idx1, idx2);
    }
    
    // Test case 4: Mix of positive and negative
    int test4[] = {5, -2, 8, -7, 3};
    if (two_sum(test4, 5, 1, &idx1, &idx2)) {
        printf("Test 4: [5,-2,8,-7,3], target=1 -> (%zu, %zu)\\n", idx1, idx2);
    }
    
    // Test case 5: Large numbers
    int test5[] = {1000000, 500000, 250000, 750000};
    if (two_sum(test5, 4, 1500000, &idx1, &idx2)) {
        printf("Test 5: [1000000,500000,250000,750000], target=1500000 -> (%zu, %zu)\\n", idx1, idx2);
    }
    
    return 0;
}
