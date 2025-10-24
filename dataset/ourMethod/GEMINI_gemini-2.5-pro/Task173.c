#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct Node {
    unsigned int hash1;
    unsigned int hash2;
    struct Node* next;
} Node;

void free_table(Node** table, int table_size) {
    if (!table) return;
    for (int i = 0; i < table_size; i++) {
        Node* current = table[i];
        while (current) {
            Node* temp = current;
            current = current->next;
            free(temp);
        }
    }
    free(table);
}

int search(const char* s, int L, int n) {
    if (L == 0) return 0;
    if (L >= n) return -1;

    unsigned int base1 = 26;
    unsigned int base2 = 31;
    unsigned int mod1 = 1000000007;
    unsigned int mod2 = 1000000009;

    unsigned int h1 = 1;
    unsigned int h2 = 1;
    for (int i = 0; i < L - 1; ++i) {
        h1 = ((unsigned long long)h1 * base1) % mod1;
        h2 = ((unsigned long long)h2 * base2) % mod2;
    }

    unsigned int currentHash1 = 0;
    unsigned int currentHash2 = 0;
    for (int i = 0; i < L; ++i) {
        currentHash1 = ((unsigned long long)currentHash1 * base1 + (s[i] - 'a')) % mod1;
        currentHash2 = ((unsigned long long)currentHash2 * base2 + (s[i] - 'a')) % mod2;
    }

    int table_size = 30011; // A prime number > 30000
    Node** table = (Node**)calloc(table_size, sizeof(Node*));
    if (!table) return -1;

    unsigned int bucket_idx = currentHash1 % table_size;
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (!newNode) {
        free_table(table, table_size);
        return -1;
    }
    newNode->hash1 = currentHash1;
    newNode->hash2 = currentHash2;
    newNode->next = table[bucket_idx];
    table[bucket_idx] = newNode;

    int found_start = -1;

    for (int i = 1; i <= n - L; ++i) {
        unsigned int prevCharVal = s[i - 1] - 'a';
        unsigned int nextCharVal = s[i + L - 1] - 'a';

        currentHash1 = (currentHash1 - ((unsigned long long)prevCharVal * h1) % mod1 + mod1) % mod1;
        currentHash1 = ((unsigned long long)currentHash1 * base1) % mod1;
        currentHash1 = (currentHash1 + nextCharVal) % mod1;
        
        currentHash2 = (currentHash2 - ((unsigned long long)prevCharVal * h2) % mod2 + mod2) % mod2;
        currentHash2 = ((unsigned long long)currentHash2 * base2) % mod2;
        currentHash2 = (currentHash2 + nextCharVal) % mod2;

        bucket_idx = currentHash1 % table_size;
        Node* current = table[bucket_idx];
        while (current) {
            if (current->hash1 == currentHash1 && current->hash2 == currentHash2) {
                found_start = i;
                goto cleanup;
            }
            current = current->next;
        }

        newNode = (Node*)malloc(sizeof(Node));
        if (!newNode) {
            found_start = -1;
            goto cleanup;
        }
        newNode->hash1 = currentHash1;
        newNode->hash2 = currentHash2;
        newNode->next = table[bucket_idx];
        table[bucket_idx] = newNode;
    }

cleanup:
    free_table(table, table_size);
    return found_start;
}

char* longestDupSubstring(char* s) {
    int n = strlen(s);
    int low = 1, high = n;
    int bestLen = 0;
    int start = -1;

    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (mid == 0) {
            low = mid + 1;
            continue;
        }
        int foundStart = search(s, mid, n);
        
        if (foundStart != -1) {
            bestLen = mid;
            start = foundStart;
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    char* result;
    if (start != -1) {
        result = (char*)malloc(sizeof(char) * (bestLen + 1));
        if (result) {
            strncpy(result, s + start, bestLen);
            result[bestLen] = '\0';
        }
    } else {
        result = (char*)malloc(sizeof(char));
        if(result) result[0] = '\0';
    }
    return result;
}

int main() {
    char* testCases[] = {
        "banana",
        "abcd",
        "ababa",
        "mississippi",
        "aaaaaaaaaa"
    };
    char* expectedResults[] = {
        "ana",
        "",
        "aba",
        "issi",
        "aaaaaaaaa"
    };
    int num_tests = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < num_tests; ++i) {
        char* s = testCases[i];
        char* result = longestDupSubstring(s);
        printf("Test Case %d:\n", i + 1);
        printf("Input: s = \"%s\"\n", s);
        printf("Output: \"%s\"\n", result);
        printf("Expected: \"%s\"\n", expectedResults[i]);
        printf("\n");
        free(result);
    }

    return 0;
}