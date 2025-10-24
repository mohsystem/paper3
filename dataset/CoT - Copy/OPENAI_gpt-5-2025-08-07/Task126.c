#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

// Step 1: Purpose: map user input to safe routes
// Step 2: Security: whitelist and strict validation
// Step 3: Implement sanitize and constant route selection
// Step 4: Review: return only literals; no dynamic URL creation
// Step 5: Output: safe predefined paths or /error

static void sanitize(const char* in, char* out, size_t out_size) {
    if (!in || out_size == 0) {
        if (out_size > 0) out[0] = '\0';
        return;
    }

    // Trim leading whitespace
    size_t len = strlen(in);
    size_t start = 0;
    while (start < len && isspace((unsigned char)in[start])) start++;

    // Trim trailing whitespace
    if (start == len) {
        out[0] = '\0';
        return;
    }
    size_t end = len - 1;
    while (end > start && isspace((unsigned char)in[end])) end--;

    // Validate allowed characters and copy lowered
    bool valid = true;
    size_t j = 0;
    for (size_t i = start; i <= end; i++) {
        unsigned char c = (unsigned char)in[i];
        if (!(isalnum(c) || c == '_' || c == '-')) {
            valid = false;
            break;
        }
        char lc = (char)tolower(c);
        if (j + 1 < out_size) {
            out[j++] = lc;
        } else {
            // Truncation would change meaning; mark invalid
            valid = false;
            break;
        }
    }
    if (valid) {
        out[j] = '\0';
    } else {
        out[0] = '\0';
    }
}

const char* get_redirect_target(const char* user_input) {
    char key[128];
    sanitize(user_input, key, sizeof(key));
    if (key[0] == '\0') return "/error";

    if (strcmp(key, "home") == 0) return "/home";
    if (strcmp(key, "profile") == 0) return "/user/profile";
    if (strcmp(key, "settings") == 0) return "/settings";
    if (strcmp(key, "help") == 0) return "/help";
    if (strcmp(key, "logout") == 0) return "/logout";
    return "/error";
}

int main(void) {
    const char* tests[5] = {"home", "HoMe", " profile ", "../admin", "unknown"};
    for (int i = 0; i < 5; i++) {
        printf("Input: \"%s\" -> Redirect: %s\n", tests[i], get_redirect_target(tests[i]));
    }
    return 0;
}