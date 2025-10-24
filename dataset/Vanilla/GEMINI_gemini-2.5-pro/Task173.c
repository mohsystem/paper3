#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Hash Table Implementation ---
#define HASH_TABLE_SIZE 100003 

typedef struct Node {
    long long hash;
    int index;
    struct Node* next;
} Node;

typedef struct HashTable {
    Node** buckets;
    int size;
} HashTable;

Node* createNode(long long hash, int index) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->hash = hash;
    newNode->index = index;
    newNode->next = NULL;
    return newNode;
}

HashTable* createHashTable() {
    HashTable* ht = (HashTable*)malloc(sizeof(HashTable));
    ht->size = HASH_TABLE_SIZE;
    ht->buckets = (Node**)calloc(ht->size, sizeof(Node*));
    return ht;
}

void freeHashTable(HashTable* ht) {
    for (int i = 0; i < ht->size; i++) {
        Node* current = ht->buckets[i];
        while (current != NULL) {
            Node* temp = current;
            current = current->next;
            free(temp);
        }
    }
    free(ht->buckets);
    free(ht);
}

void insert(HashTable* ht, long long hash, int index) {
    long long bucketIndex = (hash % ht->size + ht->size) % ht->size;
    Node* newNode = createNode(hash, index);
    newNode->next = ht->buckets[bucketIndex];
    ht->buckets[bucketIndex] = newNode;
}

Node* find(HashTable* ht, long long hash) {
    long long bucketIndex = (hash % ht->size + ht->size) % ht->size;
    return ht->buckets[bucketIndex];
}

// --- Main Logic ---

long long power(long long base, long long exp, long long mod) {
    long long res = 1;
    base %= mod;
    while (exp > 0) {
        if (exp % 2 == 1) res = ((__int128)res * base) % mod;
        base = ((__int128)base * base) % mod;
        exp /= 2;
    }
    return res;
}

int search(int L, int n, const char* s, long long modulus, long long base) {
    if (L == 0) return 0;

    long long h = power(base, L - 1, modulus);

    long long current_hash = 0;
    for (int i = 0; i < L; ++i) {
        current_hash = ((__int128)current_hash * base + (s[i] - 'a')) % modulus;
    }

    HashTable* ht = createHashTable();
    insert(ht, current_hash, 0);

    int found_start = -1;

    for (int i = 1; i <= n - L; ++i) {
        long long term_to_remove = ((__int128)(s[i - 1] - 'a') * h) % modulus;
        current_hash = (current_hash - term_to_remove + modulus) % modulus;
        current_hash = ((__int128)current_hash * base) % modulus;
        current_hash = (current_hash + (s[i + L - 1] - 'a')) % modulus;

        Node* bucket_head = find(ht, current_hash);
        Node* current = bucket_head;
        while (current != NULL) {
            if (current->hash == current_hash) {
                if (strncmp(s + current->index, s + i, L) == 0) {
                    found_start = i;
                    goto end_search;
                }
            }
            current = current->next;
        }
        insert(ht, current_hash, i);
    }

end_search:
    freeHashTable(ht);
    return found_start;
}

char* longestDupSubstring(char* s) {
    int n = strlen(s);
    int low = 1, high = n;
    int start = -1;
    int len = 0;
    
    long long base = 26;
    long long modulus = (1LL << 61) - 1;

    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (mid == 0) {
            low = mid + 1;
            continue;
        }
        int found_start = search(mid, n, s, modulus, base);
        if (found_start != -1) {
            len = mid;
            start = found_start;
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    
    char* result;
    if (start != -1) {
        result = (char*)malloc(len + 1);
        strncpy(result, s + start, len);
        result[len] = '\0';
    } else {
        result = (char*)malloc(1);
        result[0] = '\0';
    }
    return result;
}

int main() {
    char s1[] = "banana";
    char s2[] = "abcd";
    char s3[] = "ababab";
    char s4[] = "aaaaa";
    char s5[] = "aabaaabaaaba";
    
    char* testCases[] = {s1, s2, s3, s4, s5};
    int num_tests = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < num_tests; ++i) {
        printf("Input: \"%s\"\n", testCases[i]);
        char* result = longestDupSubstring(testCases[i]);
        printf("Output: \"%s\"\n\n", result);
        free(result);
    }
    return 0;
}