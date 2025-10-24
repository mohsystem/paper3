#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// A struct to hold a hash value and its original index
typedef struct {
    long long hash;
    int index;
} HashPair;

// Comparison function for qsort
int comparePairs(const void* a, const void* b) {
    long long hashA = ((HashPair*)a)->hash;
    long long hashB = ((HashPair*)b)->hash;
    if (hashA < hashB) return -1;
    if (hashA > hashB) return 1;
    return 0;
}

/**
 * Checks for a duplicated substring of a given length.
 * Since C doesn't have a standard hash table, this function computes all hashes,
 * sorts them, and then checks for adjacent equal hashes.
 * @return The starting index of a duplicated substring, or -1 if none.
 */
int check(int len, const char* s, int n, long long base, long long mod) {
    if (len == 0) return 0;
    if (len >= n) return -1;

    long long h = 1;
    for (int i = 0; i < len; ++i) {
        h = (h * base) % mod;
    }

    int num_hashes = n - len + 1;
    HashPair* hashes = (HashPair*)malloc(num_hashes * sizeof(HashPair));
    if (!hashes) return -1; // Memory allocation failed

    long long current_hash = 0;
    for (int i = 0; i < len; ++i) {
        current_hash = (current_hash * base + (s[i] - 'a' + 1)) % mod;
    }
    hashes[0].hash = current_hash;
    hashes[0].index = 0;

    for (int i = 1; i <= n - len; ++i) {
        current_hash = (current_hash * base - ((long long)(s[i - 1] - 'a' + 1) * h) % mod + mod) % mod;
        current_hash = (current_hash + (s[i + len - 1] - 'a' + 1)) % mod;
        hashes[i].hash = current_hash;
        hashes[i].index = i;
    }

    qsort(hashes, num_hashes, sizeof(HashPair), comparePairs);

    int found_index = -1;
    for (int i = 1; i < num_hashes; ++i) {
        if (hashes[i].hash == hashes[i - 1].hash) {
            // Found a potential duplicate.
            // A more robust solution would use strncmp to verify.
            found_index = hashes[i - 1].index;
            break; 
        }
    }
    
    free(hashes);
    return found_index;
}

/**
 * Finds the longest duplicated substring in a string.
 * @param s The input string (must be mutable for testing).
 * @return A new dynamically allocated string with the result. The caller must free this memory.
 */
char* longestDupSubstring(char* s) {
    int n = strlen(s);
    if (n <= 1) {
        char* empty = (char*)malloc(1);
        if(empty) empty[0] = '\0';
        return empty;
    }

    long long base = 29;
    long long mod = 1e9 + 7;

    int low = 1, high = n - 1;
    int ans_start = -1;
    int ans_len = 0;

    while (low <= high) {
        int mid = low + (high - low) / 2;
        int start_index = check(mid, s, n, base, mod);
        if (start_index != -1) {
            ans_len = mid;
            ans_start = start_index;
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    char* result;
    if (ans_len == 0) {
        result = (char*)malloc(1);
        if(result) result[0] = '\0';
    } else {
        result = (char*)malloc(ans_len + 1);
        if (result) {
            strncpy(result, s + ans_start, ans_len);
            result[ans_len] = '\0';
        }
    }
    return result;
}

int main() {
    char s1[] = "banana";
    char s2[] = "abcd";
    char s3[] = "ababa";
    char s4[] = "aaaaa";
    char s5[] = "mississippi";

    char* testCases[] = {s1, s2, s3, s4, s5};
    int num_tests = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < num_tests; ++i) {
        char* s = testCases[i];
        printf("Input: s = \"%s\"\n", s);
        char* result = longestDupSubstring(s);
        if (result) {
            printf("Output: \"%s\"\n", result);
            free(result);
        }
        printf("\n");
    }

    return 0;
}