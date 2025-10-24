
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Hash table node for storing distinct substrings
typedef struct HashNode {
    char *key;
    struct HashNode *next;
} HashNode;

// Hash table structure
typedef struct {
    HashNode **buckets;
    size_t size;
} HashTable;

// Initialize hash table with input validation
HashTable* createHashTable(size_t size) {
    // Validate size to prevent integer overflow and excessive allocation
    if (size == 0 || size > 1000000) {
        return NULL;
    }
    
    HashTable *table = (HashTable*)malloc(sizeof(HashTable));
    if (table == NULL) {
        return NULL;
    }
    
    table->buckets = (HashNode**)calloc(size, sizeof(HashNode*));
    if (table->buckets == NULL) {
        free(table);
        return NULL;
    }
    
    table->size = size;
    return table;
}

// Hash function for strings with bounds checking
unsigned int hash(const char *str, size_t tableSize) {
    if (str == NULL || tableSize == 0) {
        return 0;
    }
    
    unsigned int hash = 5381;
    int c;
    size_t len = 0;
    const size_t MAX_LEN = 2000; // Based on constraint
    
    while ((c = *str++) && len < MAX_LEN) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
        len++;
    }
    
    return hash % tableSize;
}

// Insert string into hash table, returns true if newly inserted
bool hashTableInsert(HashTable *table, const char *str, size_t len) {
    // Input validation
    if (table == NULL || str == NULL || len == 0 || len > 2000) {
        return false;
    }
    
    unsigned int index = hash(str, table->size);
    HashNode *current = table->buckets[index];
    
    // Check if string already exists
    while (current != NULL) {
        if (current->key != NULL && strncmp(current->key, str, len) == 0 && 
            strlen(current->key) == len) {
            return false; // Already exists
        }
        current = current->next;
    }
    
    // Allocate new node
    HashNode *newNode = (HashNode*)malloc(sizeof(HashNode));
    if (newNode == NULL) {
        return false;
    }
    
    // Allocate and copy key with null termination
    newNode->key = (char*)malloc(len + 1);
    if (newNode->key == NULL) {
        free(newNode);
        return false;
    }
    
    // Use memcpy for bounded copy and ensure null termination
    memcpy(newNode->key, str, len);
    newNode->key[len] = '\\0';
    
    newNode->next = table->buckets[index];
    table->buckets[index] = newNode;
    
    return true;
}

// Free hash table and all allocated memory
void freeHashTable(HashTable *table) {
    if (table == NULL) {
        return;
    }
    
    if (table->buckets != NULL) {
        for (size_t i = 0; i < table->size; i++) {
            HashNode *current = table->buckets[i];
            while (current != NULL) {
                HashNode *temp = current;
                current = current->next;
                
                // Free the key string
                if (temp->key != NULL) {
                    free(temp->key);
                }
                free(temp);
            }
        }
        free(table->buckets);
    }
    
    free(table);
}

// Check if a substring can be written as a+a with bounds checking
bool isRepeatedString(const char *text, size_t start, size_t length) {
    // Validate inputs
    if (text == NULL || length == 0 || length % 2 != 0) {
        return false;
    }
    
    size_t halfLen = length / 2;
    
    // Compare first half with second half
    for (size_t i = 0; i < halfLen; i++) {
        if (text[start + i] != text[start + halfLen + i]) {
            return false;
        }
    }
    
    return true;
}

int distinctEchoSubstrings(const char *text) {
    // Input validation
    if (text == NULL) {
        return 0;
    }
    
    size_t textLen = strlen(text);
    
    // Validate length constraint
    if (textLen < 1 || textLen > 2000) {
        return 0;
    }
    
    // Validate that text contains only lowercase English letters
    for (size_t i = 0; i < textLen; i++) {
        if (text[i] < 'a' || text[i] > 'z') {
            return 0;
        }
    }
    
    // Create hash table for storing distinct substrings
    HashTable *distinctSubstrings = createHashTable(10007);
    if (distinctSubstrings == NULL) {
        return 0;
    }
    
    int count = 0;
    
    // Iterate through all possible substrings with even length
    for (size_t i = 0; i < textLen; i++) {
        for (size_t len = 2; len <= textLen - i; len += 2) {
            // Check bounds to prevent overflow
            if (i + len > textLen) {
                break;
            }
            
            // Check if substring is repeated pattern (a+a)
            if (isRepeatedString(text, i, len)) {
                // Try to insert into hash table
                if (hashTableInsert(distinctSubstrings, text + i, len)) {
                    count++;
                }
            }
        }
    }
    
    freeHashTable(distinctSubstrings);
    return count;
}

int main(void) {
    // Test case 1
    const char *test1 = "abcabcabc";
    int result1 = distinctEchoSubstrings(test1);
    printf("Test 1: text = \\"%s\\", Output = %d (Expected: 3)\\n", test1, result1);
    
    // Test case 2
    const char *test2 = "leetcodeleetcode";
    int result2 = distinctEchoSubstrings(test2);
    printf("Test 2: text = \\"%s\\", Output = %d (Expected: 2)\\n", test2, result2);
    
    // Test case 3
    const char *test3 = "aa";
    int result3 = distinctEchoSubstrings(test3);
    printf("Test 3: text = \\"%s\\", Output = %d (Expected: 1)\\n", test3, result3);
    
    // Test case 4
    const char *test4 = "aaaa";
    int result4 = distinctEchoSubstrings(test4);
    printf("Test 4: text = \\"%s\\", Output = %d (Expected: 2)\\n", test4, result4);
    
    // Test case 5
    const char *test5 = "abcdef";
    int result5 = distinctEchoSubstrings(test5);
    printf("Test 5: text = \\"%s\\", Output = %d (Expected: 0)\\n", test5, result5);
    
    return 0;
}
