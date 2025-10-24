
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Simple hash table implementation for strings
#define HASH_SIZE 10007

typedef struct Node {
    char* str;
    struct Node* next;
} Node;

typedef struct {
    Node* buckets[HASH_SIZE];
} HashSet;

unsigned long hash(const char* str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;
    return hash % HASH_SIZE;
}

int contains(HashSet* set, const char* str) {
    unsigned long idx = hash(str);
    Node* current = set->buckets[idx];
    while (current) {
        if (strcmp(current->str, str) == 0)
            return 1;
        current = current->next;
    }
    return 0;
}

void insert(HashSet* set, const char* str) {
    if (contains(set, str))
        return;
    
    unsigned long idx = hash(str);
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->str = strdup(str);
    newNode->next = set->buckets[idx];
    set->buckets[idx] = newNode;
}

int countElements(HashSet* set) {
    int count = 0;
    for (int i = 0; i < HASH_SIZE; i++) {
        Node* current = set->buckets[i];
        while (current) {
            count++;
            current = current->next;
        }
    }
    return count;
}

void freeHashSet(HashSet* set) {
    for (int i = 0; i < HASH_SIZE; i++) {
        Node* current = set->buckets[i];
        while (current) {
            Node* temp = current;
            current = current->next;
            free(temp->str);
            free(temp);
        }
    }
}

int distinctEchoSubstrings(const char* text) {
    HashSet set = {0};
    int n = strlen(text);
    
    for (int len = 2; len <= n; len += 2) {
        int halfLen = len / 2;
        
        for (int i = 0; i <= n - len; i++) {
            int match = 1;
            for (int j = 0; j < halfLen; j++) {
                if (text[i + j] != text[i + halfLen + j]) {
                    match = 0;
                    break;
                }
            }
            
            if (match) {
                char* substring = (char*)malloc(len + 1);
                strncpy(substring, text + i, len);
                substring[len] = '\\0';
                insert(&set, substring);
                free(substring);
            }
        }
    }
    
    int result = countElements(&set);
    freeHashSet(&set);
    return result;
}

int main() {
    // Test case 1
    printf("Test 1: %d\\n", distinctEchoSubstrings("abcabcabc")); // Expected: 3
    
    // Test case 2
    printf("Test 2: %d\\n", distinctEchoSubstrings("leetcodeleetcode")); // Expected: 2
    
    // Test case 3
    printf("Test 3: %d\\n", distinctEchoSubstrings("aa")); // Expected: 1
    
    // Test case 4
    printf("Test 4: %d\\n", distinctEchoSubstrings("aaaa")); // Expected: 2
    
    // Test case 5
    printf("Test 5: %d\\n", distinctEchoSubstrings("abcdefg")); // Expected: 0
    
    return 0;
}
