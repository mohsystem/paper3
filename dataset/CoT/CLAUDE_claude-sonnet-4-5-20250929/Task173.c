
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MOD 1000000007LL
#define BASE 26LL

typedef struct {
    long long hash;
    int* indices;
    int count;
    int capacity;
} HashEntry;

char* search(const char* s, int len, int slen) {
    if (len <= 0 || len > slen) {
        return NULL;
    }
    
    long long hash = 0;
    long long pow = 1;
    
    for (int i = 0; i < len; i++) {
        hash = (hash * BASE + (s[i] - 'a')) % MOD;
        if (i < len - 1) {
            pow = (pow * BASE) % MOD;
        }
    }
    
    HashEntry* hashTable = (HashEntry*)calloc(slen, sizeof(HashEntry));
    int tableSize = 0;
    
    hashTable[0].hash = hash;
    hashTable[0].indices = (int*)malloc(sizeof(int));
    hashTable[0].indices[0] = 0;
    hashTable[0].count = 1;
    hashTable[0].capacity = 1;
    tableSize = 1;
    
    for (int i = len; i < slen; i++) {
        hash = ((hash - (s[i - len] - 'a') * pow % MOD + MOD) % MOD * BASE + (s[i] - 'a')) % MOD;
        
        int found = -1;
        for (int j = 0; j < tableSize; j++) {
            if (hashTable[j].hash == hash) {
                found = j;
                break;
            }
        }
        
        if (found != -1) {
            for (int j = 0; j < hashTable[found].count; j++) {
                int idx = hashTable[found].indices[j];
                if (strncmp(s + idx, s + i - len + 1, len) == 0) {
                    char* result = (char*)malloc((len + 1) * sizeof(char));
                    strncpy(result, s + i - len + 1, len);
                    result[len] = '\\0';
                    
                    for (int k = 0; k < tableSize; k++) {
                        free(hashTable[k].indices);
                    }
                    free(hashTable);
                    return result;
                }
            }
            
            if (hashTable[found].count >= hashTable[found].capacity) {
                hashTable[found].capacity *= 2;
                hashTable[found].indices = (int*)realloc(hashTable[found].indices, hashTable[found].capacity * sizeof(int));
            }
            hashTable[found].indices[hashTable[found].count++] = i - len + 1;
        } else {
            hashTable[tableSize].hash = hash;
            hashTable[tableSize].indices = (int*)malloc(sizeof(int));
            hashTable[tableSize].indices[0] = i - len + 1;
            hashTable[tableSize].count = 1;
            hashTable[tableSize].capacity = 1;
            tableSize++;
        }
    }
    
    for (int k = 0; k < tableSize; k++) {
        free(hashTable[k].indices);
    }
    free(hashTable);
    return NULL;
}

char* longestDupSubstring(const char* s) {
    if (s == NULL || strlen(s) < 2) {
        char* empty = (char*)malloc(sizeof(char));
        empty[0] = '\\0';
        return empty;
    }
    
    int n = strlen(s);
    int left = 1, right = n;
    char* result = NULL;
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        char* dup = search(s, mid, n);
        if (dup != NULL) {
            if (result != NULL) free(result);
            result = dup;
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    
    if (result == NULL) {
        result = (char*)malloc(sizeof(char));
        result[0] = '\\0';
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
