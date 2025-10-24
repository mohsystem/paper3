#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    long long h1;
    long long h2;
} HashPair;

int comparePairs(const void* a, const void* b) {
    HashPair* p1 = (HashPair*)a;
    HashPair* p2 = (HashPair*)b;
    if (p1->h1 < p2->h1) return -1;
    if (p1->h1 > p2->h1) return 1;
    if (p1->h2 < p2->h2) return -1;
    if (p1->h2 > p2->h2) return 1;
    return 0;
}

HashPair get_hashes(int l, int r, long long* h1, long long* h2, long long* p1_powers, long long* p2_powers) {
    long long M1 = 1000000007;
    long long M2 = 1000000009;
    int len = r - l + 1;
    long long hash_a = (h1[r + 1] - (h1[l] * p1_powers[len]) % M1 + M1) % M1;
    long long hash_b = (h2[r + 1] - (h2[l] * p2_powers[len]) % M2 + M2) % M2;
    HashPair p = {hash_a, hash_b};
    return p;
}

int distinctEchoSubstrings(const char* text) {
    int n = strlen(text);
    if (n == 0) return 0;

    long long P1 = 31, M1 = 1000000007;
    long long P2 = 37, M2 = 1000000009;

    long long* p1_powers = (long long*)malloc((n + 1) * sizeof(long long));
    long long* p2_powers = (long long*)malloc((n + 1) * sizeof(long long));
    long long* h1 = (long long*)malloc((n + 1) * sizeof(long long));
    long long* h2 = (long long*)malloc((n + 1) * sizeof(long long));
    
    if (!p1_powers || !p2_powers || !h1 || !h2) { exit(1); }

    p1_powers[0] = 1;
    p2_powers[0] = 1;
    for (int i = 1; i <= n; ++i) {
        p1_powers[i] = (p1_powers[i - 1] * P1) % M1;
        p2_powers[i] = (p2_powers[i - 1] * P2) % M2;
    }
    
    h1[0] = 0;
    h2[0] = 0;
    for (int i = 0; i < n; ++i) {
        h1[i + 1] = (h1[i] * P1 + (text[i] - 'a' + 1)) % M1;
        h2[i + 1] = (h2[i] * P2 + (text[i] - 'a' + 1)) % M2;
    }

    int capacity = 10;
    int count = 0;
    HashPair* found = (HashPair*)malloc(capacity * sizeof(HashPair));
    if (!found) { exit(1); }

    for (int len = 2; len <= n; len += 2) {
        int half = len / 2;
        for (int i = 0; i <= n - len; ++i) {
            HashPair h_first = get_hashes(i, i + half - 1, h1, h2, p1_powers, p2_powers);
            HashPair h_second = get_hashes(i + half, i + len - 1, h1, h2, p1_powers, p2_powers);

            if (h_first.h1 == h_second.h1 && h_first.h2 == h_second.h2) {
                if (count == capacity) {
                    capacity *= 2;
                    HashPair* temp = (HashPair*)realloc(found, capacity * sizeof(HashPair));
                    if (!temp) { exit(1); }
                    found = temp;
                }
                found[count++] = get_hashes(i, i + len - 1, h1, h2, p1_powers, p2_powers);
            }
        }
    }
    
    int unique_count = 0;
    if (count > 0) {
        qsort(found, count, sizeof(HashPair), comparePairs);
        unique_count = 1;
        for (int i = 1; i < count; ++i) {
            if (comparePairs(&found[i-1], &found[i]) != 0) {
                unique_count++;
            }
        }
    }
    
    free(p1_powers);
    free(p2_powers);
    free(h1);
    free(h2);
    free(found);

    return unique_count;
}

int main() {
    // Test Case 1
    const char* text1 = "abcabcabc";
    printf("Test Case 1: text = \"%s\", Expected: 3, Got: %d\n", text1, distinctEchoSubstrings(text1));

    // Test Case 2
    const char* text2 = "leetcodeleetcode";
    printf("Test Case 2: text = \"%s\", Expected: 2, Got: %d\n", text2, distinctEchoSubstrings(text2));

    // Test Case 3
    const char* text3 = "aaaaa";
    printf("Test Case 3: text = \"%s\", Expected: 2, Got: %d\n", text3, distinctEchoSubstrings(text3));

    // Test Case 4
    const char* text4 = "ababa";
    printf("Test Case 4: text = \"%s\", Expected: 2, Got: %d\n", text4, distinctEchoSubstrings(text4));

    // Test Case 5
    const char* text5 = "a";
    printf("Test Case 5: text = \"%s\", Expected: 0, Got: %d\n", text5, distinctEchoSubstrings(text5));

    return 0;
}