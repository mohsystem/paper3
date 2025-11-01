
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <openssl/sha.h>

#define MAX_USERNAME 64
#define MAX_PASSWORD 128
#define MAX_SESSION 128
#define HASH_SIZE 65

typedef struct {
    char username[MAX_USERNAME];
    char passwordHash[HASH_SIZE];
} Moderator;

typedef struct {
    char session[MAX_SESSION];
    char username[MAX_USERNAME];
} Session;

Moderator moderators[10];
Session sessions[10];
int mod_count = 0;
int session_count = 0;

void hash_password(const char* password, char* output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password, strlen(password), hash);
    
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[64] = '\\0';
}

void sanitize_input(char* input) {
    int j = 0;
    for(int i = 0; input[i] != '\\0'; i++) {
        if(isalnum(input[i])) {
            input[j++] = input[i];
        }
    }
    input[j] = '\\0';
}

void initialize_data() {
    // Add moderator admin
    strcpy(moderators[mod_count].username, "admin");
    hash_password("admin123", moderators[mod_count].passwordHash);
    mod_count++;
    
    // Add moderator mod1
    strcpy(moderators[mod_count].username, "mod1");
    hash_password("modpass456", moderators[mod_count].passwordHash);
    mod_count++;
    
    // Add sessions
    strcpy(sessions[session_count].session, "session123");
    strcpy(sessions[session_count].username, "admin");
    session_count++;
    
    strcpy(sessions[session_count].session, "session456");
    strcpy(sessions[session_count].username, "mod1");
    session_count++;
}

int auth_mod(char* username, char* password, char* session) {
    // Sanitize inputs
    sanitize_input(username);
    sanitize_input(session);
    
    if(strlen(username) == 0 || strlen(password) == 0 || strlen(session) == 0) {
        return 0;
    }
    
    // Verify session
    int session_valid = 0;
    for(int i = 0; i < session_count; i++) {
        if(strcmp(sessions[i].session, session) == 0 && 
           strcmp(sessions[i].username, username) == 0) {
            session_valid = 1;
            break;
        }
    }
    
    if(!session_valid) {
        return 0;
    }
    
    // Verify credentials
    char input_hash[HASH_SIZE];
    hash_password(password, input_hash);
    
    for(int i = 0; i < mod_count; i++) {
        if(strcmp(moderators[i].username, username) == 0 && 
           strcmp(moderators[i].passwordHash, input_hash) == 0) {
            return 1;
        }
    }
    
    return 0;
}

const char* check_mod(char* username, char* password, char* session) {
    int is_moderator = auth_mod(username, password, session);
    return is_moderator ? "Moderator access granted" : "Access denied";
}

int main() {
    initialize_data();
    
    char username[MAX_USERNAME], password[MAX_PASSWORD], session[MAX_SESSION];
    
    // Test Case 1
    printf("Test Case 1: Valid moderator admin\\n");
    strcpy(username, "admin");
    strcpy(password, "admin123");
    strcpy(session, "session123");
    printf("%s\\n\\n", check_mod(username, password, session));
    
    // Test Case 2
    printf("Test Case 2: Valid moderator mod1\\n");
    strcpy(username, "mod1");
    strcpy(password, "modpass456");
    strcpy(session, "session456");
    printf("%s\\n\\n", check_mod(username, password, session));
    
    // Test Case 3
    printf("Test Case 3: Invalid password\\n");
    strcpy(username, "admin");
    strcpy(password, "wrongpass");
    strcpy(session, "session123");
    printf("%s\\n\\n", check_mod(username, password, session));
    
    // Test Case 4
    printf("Test Case 4: Invalid session\\n");
    strcpy(username, "admin");
    strcpy(password, "admin123");
    strcpy(session, "invalidsession");
    printf("%s\\n\\n", check_mod(username, password, session));
    
    // Test Case 5
    printf("Test Case 5: Non-existent user\\n");
    strcpy(username, "hacker");
    strcpy(password, "password");
    strcpy(session, "session999");
    printf("%s\\n\\n", check_mod(username, password, session));
    
    return 0;
}
