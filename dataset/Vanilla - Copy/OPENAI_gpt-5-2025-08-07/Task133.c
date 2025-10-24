#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>

#define USERNAME_MAX 32
#define PASSWORD_MAX 64

typedef struct {
    char username[USERNAME_MAX];
    char password[PASSWORD_MAX];
} User;

static bool has_upper(const char* s){ for(; *s; ++s) if (isupper((unsigned char)*s)) return true; return false; }
static bool has_lower(const char* s){ for(; *s; ++s) if (islower((unsigned char)*s)) return true; return false; }
static bool has_digit(const char* s){ for(; *s; ++s) if (isdigit((unsigned char)*s)) return true; return false; }
static bool has_special(const char* s){ for(; *s; ++s) if (!isalnum((unsigned char)*s)) return true; return false; }

static void to_lower_copy(const char* src, char* dst, size_t dstsz){
    size_t i=0;
    if (dstsz == 0) return;
    for (; src[i] && i < dstsz-1; ++i) dst[i] = (char)tolower((unsigned char)src[i]);
    dst[i] = '\0';
}

static bool contains_case_insensitive(const char* hay, const char* needle){
    size_t hlen = strlen(hay), nlen = strlen(needle);
    if (nlen == 0) return true;
    char* h = (char*)malloc(hlen + 1);
    char* n = (char*)malloc(nlen + 1);
    if (!h || !n){ if(h) free(h); if(n) free(n); return false; }
    to_lower_copy(hay, h, hlen + 1);
    to_lower_copy(needle, n, nlen + 1);
    bool found = strstr(h, n) != NULL;
    free(h); free(n);
    return found;
}

char* reset_password(User users[], int user_count, const char* username, const char* old_pass, const char* new_pass, const char* confirm_pass){
    char* out = (char*)malloc(128);
    if (!out) return NULL;
    if (!username || !old_pass || !new_pass || !confirm_pass){
        snprintf(out, 128, "ERROR: Invalid input.");
        return out;
    }
    int idx = -1;
    for (int i=0;i<user_count;i++){
        if (strcmp(users[i].username, username) == 0){
            idx = i; break;
        }
    }
    if (idx == -1){
        snprintf(out, 128, "ERROR: User not found.");
        return out;
    }
    if (strcmp(users[idx].password, old_pass) != 0){
        snprintf(out, 128, "ERROR: Old password is incorrect.");
        return out;
    }
    if (strcmp(new_pass, confirm_pass) != 0){
        snprintf(out, 128, "ERROR: New password and confirmation do not match.");
        return out;
    }
    if (strcmp(new_pass, old_pass) == 0){
        snprintf(out, 128, "ERROR: New password must be different from old password.");
        return out;
    }
    if (strlen(new_pass) < 8){
        snprintf(out, 128, "ERROR: Password must be at least 8 characters long.");
        return out;
    }
    if (!has_upper(new_pass) || !has_lower(new_pass) || !has_digit(new_pass) || !has_special(new_pass)){
        snprintf(out, 128, "ERROR: Password must include uppercase, lowercase, digit, and special character.");
        return out;
    }
    if (contains_case_insensitive(new_pass, username)){
        snprintf(out, 128, "ERROR: Password must not contain the username.");
        return out;
    }
    if (strlen(new_pass) >= PASSWORD_MAX){
        snprintf(out, 128, "ERROR: Password too long for storage.");
        return out;
    }
    strncpy(users[idx].password, new_pass, PASSWORD_MAX-1);
    users[idx].password[PASSWORD_MAX-1] = '\0';
    snprintf(out, 128, "SUCCESS: Password updated.");
    return out;
}

int main(){
    User users[3] = {
        {"alice", "OldPass1!"},
        {"bob",   "Bob#2020"},
        {"carol", "Car0l@123"}
    };

    char* msg;

    // Test 1: Successful reset for alice
    msg = reset_password(users, 3, "alice", "OldPass1!", "NewStrong1$", "NewStrong1$");
    printf("%s\n", msg); free(msg);

    // Test 2: Wrong old password for bob
    msg = reset_password(users, 3, "bob", "wrong", "BobNew#2024", "BobNew#2024");
    printf("%s\n", msg); free(msg);

    // Test 3: Confirm mismatch for carol
    msg = reset_password(users, 3, "carol", "Car0l@123", "Another1!", "Mismatch1!");
    printf("%s\n", msg); free(msg);

    // Test 4: Unknown user dave
    msg = reset_password(users, 3, "dave", "anything", "SomePass1!", "SomePass1!");
    printf("%s\n", msg); free(msg);

    // Test 5: Weak password for alice after successful change (note old is updated)
    msg = reset_password(users, 3, "alice", "NewStrong1$", "short", "short");
    printf("%s\n", msg); free(msg);

    return 0;
}