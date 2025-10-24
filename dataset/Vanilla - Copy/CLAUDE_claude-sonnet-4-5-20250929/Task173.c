
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    long long key;
    struct Node* next;
} Node;

typedef struct {
    Node** buckets;
    int size;
} HashSet;

HashSet* createHashSet(int size) {
    HashSet* set = (HashSet*)malloc(sizeof(HashSet));
    set->size = size;
    set->buckets = (Node**)calloc(size, sizeof(Node*));
    return set;
}

int hashSetContains(HashSet* set, long long key) {
    int index = (key % set->size + set->size) % set->size;
    Node* node = set->buckets[index];
    while (node) {
        if (node->key == key) return 1;
        node = node->next;
    }
    return 0;
}

void hashSetAdd(HashSet* set, long long key) {
    int index = (key % set->size + set->size) % set->size;
    Node* node = (Node*)malloc(sizeof(Node));
    node->key = key;
    node->next = set->buckets[index];
    set->buckets[index] = node;
}

void freeHashSet(HashSet* set) {
    for (int i = 0; i < set->size; i++) {
        Node* node = set->buckets[i];
        while (node) {
            Node* temp = node;
            node = node->next;
            free(temp);
        }
    }
    free(set->buckets);
    free(set);
}

char* search(const char* s, int len, int n) {
    long long mod = (1LL << 32);
    long long base = 26;
    long long hash = 0;
    long long pow = 1;
    
    for (int i = 0; i < len; i++) {
        hash = (hash * base + (s[i] - 'a')) % mod;
        if (i < len - 1) {
            pow = (pow * base) % mod;
        }
    }
    
    HashSet* seen = createHashSet(10007);
    hashSetAdd(seen, hash);
    
    for (int i = len; i < n; i++) {
        hash = (hash * base - (s[i - len] - 'a') * pow % mod + mod) % mod;
        hash = (hash + (s[i] - 'a')) % mod;
        
        if (hashSetContains(seen, hash)) {
            char* result = (char*)malloc((len + 1) * sizeof(char));
            strncpy(result, s + i - len + 1, len);
            result[len] = '\\0';
            freeHashSet(seen);
            return result;
        }
        hashSetAdd(seen, hash);
    }
    
    freeHashSet(seen);
    return NULL;
}

char* longestDupSubstring(const char* s) {
    int n = strlen(s);
    int left = 1, right = n;
    char* result = (char*)malloc(sizeof(char));
    result[0] = '\\0';
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        char* dup = search(s, mid, n);
        if (dup) {
            free(result);
            result = dup;
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    
    return result;
}

int main() {
    // Test cases
    char* result;
    
    result = longestDupSubstring("banana");
    printf("Test 1: %s\\n", result);
    free(result);
    
    result = longestDupSubstring("abcd");
    printf("Test 2: %s\\n", result);
    free(result);
    
    result = longestDupSubstring("aa");
    printf("Test 3: %s\\n", result);
    free(result);
    
    result = longestDupSubstring("aaaaa");
    printf("Test 4: %s\\n", result);
    free(result);
    
    result = longestDupSubstring("abcabcabc");
    printf("Test 5: %s\\n", result);
    free(result);
    
    return 0;
}
