
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MOD 2147483647LL

typedef struct Node {
    long long hash;
    int* indices;
    int count;
    int capacity;
    struct Node* next;
} Node;

typedef struct {
    Node** buckets;
    int size;
} HashMap;

HashMap* createHashMap(int size) {
    HashMap* map = (HashMap*)malloc(sizeof(HashMap));
    map->size = size;
    map->buckets = (Node**)calloc(size, sizeof(Node*));
    return map;
}

void insertHashMap(HashMap* map, long long hash, int index) {
    int bucket = (int)(hash % map->size);
    Node* node = map->buckets[bucket];
    
    while (node != NULL) {
        if (node->hash == hash) {
            if (node->count == node->capacity) {
                node->capacity *= 2;
                node->indices = (int*)realloc(node->indices, node->capacity * sizeof(int));
            }
            node->indices[node->count++] = index;
            return;
        }
        node = node->next;
    }
    
    node = (Node*)malloc(sizeof(Node));
    node->hash = hash;
    node->capacity = 4;
    node->indices = (int*)malloc(node->capacity * sizeof(int));
    node->indices[0] = index;
    node->count = 1;
    node->next = map->buckets[bucket];
    map->buckets[bucket] = node;
}

Node* findHashMap(HashMap* map, long long hash) {
    int bucket = (int)(hash % map->size);
    Node* node = map->buckets[bucket];
    
    while (node != NULL) {
        if (node->hash == hash) return node;
        node = node->next;
    }
    return NULL;
}

void freeHashMap(HashMap* map) {
    for (int i = 0; i < map->size; i++) {
        Node* node = map->buckets[i];
        while (node != NULL) {
            Node* temp = node;
            node = node->next;
            free(temp->indices);
            free(temp);
        }
    }
    free(map->buckets);
    free(map);
}

char* search(const char* s, int len, int n) {
    long long hash = 0;
    long long power = 1;
    long long base = 26;
    
    HashMap* map = createHashMap(10000);
    
    for (int i = 0; i < len; i++) {
        hash = (hash * base + (s[i] - 'a')) % MOD;
        if (i < len - 1) {
            power = (power * base) % MOD;
        }
    }
    
    insertHashMap(map, hash, 0);
    
    for (int i = len; i < n; i++) {
        hash = ((hash - (s[i - len] - 'a') * power % MOD + MOD) % MOD * base 
                + (s[i] - 'a')) % MOD;
        
        Node* node = findHashMap(map, hash);
        if (node != NULL) {
            for (int j = 0; j < node->count; j++) {
                int idx = node->indices[j];
                if (strncmp(s + idx, s + i - len + 1, len) == 0) {
                    char* result = (char*)malloc((len + 1) * sizeof(char));
                    strncpy(result, s + i - len + 1, len);
                    result[len] = '\\0';
                    freeHashMap(map);
                    return result;
                }
            }
        }
        insertHashMap(map, hash, i - len + 1);
    }
    
    freeHashMap(map);
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
        
        if (dup != NULL) {
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
