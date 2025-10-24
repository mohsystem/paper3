#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

char* copyString(const char* s) {
    if (s == NULL) return NULL;
    size_t len = strlen(s);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    strcpy(out, s);
    return out;
}

char* concatenate(const char* a, const char* b) {
    if (!a && !b) return NULL;
    if (!a) return copyString(b);
    if (!b) return copyString(a);
    size_t la = strlen(a), lb = strlen(b);
    char* out = (char*)malloc(la + lb + 1);
    if (!out) return NULL;
    memcpy(out, a, la);
    memcpy(out + la, b, lb);
    out[la + lb] = '\0';
    return out;
}

char* reverseString(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    for (size_t i = 0; i < len; ++i) {
        out[i] = s[len - 1 - i];
    }
    out[len] = '\0';
    return out;
}

char* toUpperString(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    for (size_t i = 0; i < len; ++i) {
        out[i] = (char)toupper((unsigned char)s[i]);
    }
    out[len] = '\0';
    return out;
}

char* removeWhitespace(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    size_t j = 0;
    for (size_t i = 0; i < len; ++i) {
        if (!isspace((unsigned char)s[i])) {
            out[j++] = s[i];
        }
    }
    out[j] = '\0';
    return out;
}

int main() {
    const char* t1 = "Hello";
    const char* t2a = "Hello";
    const char* t2b = " World";
    const char* t3 = "OpenAI";
    const char* t4 = "Mixed Case 123";
    const char* t5 = "  a b\tc\nd  ";

    char* r1 = copyString(t1);
    char* r2 = concatenate(t2a, t2b);
    char* r3 = reverseString(t3);
    char* r4 = toUpperString(t4);
    char* r5 = removeWhitespace(t5);

    printf("Test 1 - copyString: %s\n", r1 ? r1 : "(null)");
    printf("Test 2 - concatenate: %s\n", r2 ? r2 : "(null)");
    printf("Test 3 - reverseString: %s\n", r3 ? r3 : "(null)");
    printf("Test 4 - toUpperString: %s\n", r4 ? r4 : "(null)");
    printf("Test 5 - removeWhitespace: %s\n", r5 ? r5 : "(null)");

    free(r1);
    free(r2);
    free(r3);
    free(r4);
    free(r5);

    return 0;
}