#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char* read_first_line(const char* filePath, size_t* out_len) {
    FILE* f = fopen(filePath, "r");
    if (!f) return NULL;

    size_t cap = 64;
    size_t len = 0;
    char* buf = (char*)malloc(cap);
    if (!buf) { fclose(f); return NULL; }

    int ch;
    while ((ch = fgetc(f)) != EOF) {
        if (ch == '\n') break;
        if (len + 1 >= cap) {
            cap *= 2;
            char* nb = (char*)realloc(buf, cap);
            if (!nb) { free(buf); fclose(f); return NULL; }
            buf = nb;
        }
        buf[len++] = (char)ch;
    }
    buf[len] = '\0';
    fclose(f);
    if (out_len) *out_len = len;
    return buf;
}

int compare_password(const char* filePath, const char* enteredPassword) {
    size_t stored_len = 0;
    char* stored = read_first_line(filePath, &stored_len);
    if (!stored) return 0;

    size_t entered_len = strlen(enteredPassword);
    if (stored_len != entered_len) {
        free(stored);
        return 0;
    }
    for (size_t i = 0; i < stored_len; ++i) {
        if (stored[i] != enteredPassword[i]) {
            free(stored);
            return 0;
        }
    }
    free(stored);
    return 1;
}

static void write_file(const char* path, const char* content) {
    FILE* f = fopen(path, "wb");
    if (!f) return;
    if (content && *content) {
        fwrite(content, 1, strlen(content), f);
    }
    fclose(f);
}

int main(void) {
    // Prepare test files
    const char* f1 = "test_pw_1.txt"; write_file(f1, "hunter2");
    const char* f2 = "test_pw_2.txt"; write_file(f2, "hunter2");
    const char* f3 = "test_pw_3.txt"; write_file(f3, "");
    const char* f4 = "test_pw_4.txt"; write_file(f4, "pass");
    const char* f5 = "test_pw_5.txt"; write_file(f5, "password");

    // 5 test cases
    printf("%s\n", compare_password(f1, "hunter2") ? "true" : "false");   // true
    printf("%s\n", compare_password(f2, "hunter3") ? "true" : "false");   // false
    printf("%s\n", compare_password(f3, "") ? "true" : "false");          // true
    printf("%s\n", compare_password(f4, "password") ? "true" : "false");  // false
    printf("%s\n", compare_password(f5, "pass") ? "true" : "false");      // false

    return 0;
}